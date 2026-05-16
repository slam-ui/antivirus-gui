#include "common/security/process_dacl.h"

#include "common/logging.h"

#include <windows.h>
#include <aclapi.h>
#include <array>

namespace antivirus::common::security {
namespace {

class SidHandle final {
public:
    explicit SidHandle(PSID sid = nullptr)
        : sid_(sid)
    {
    }

    ~SidHandle()
    {
        if (sid_ != nullptr) {
            FreeSid(sid_);
        }
    }

    SidHandle(const SidHandle&) = delete;
    SidHandle& operator=(const SidHandle&) = delete;

    [[nodiscard]] PSID get() const
    {
        return sid_;
    }

private:
    PSID sid_ = nullptr;
};

SidHandle makeBuiltinSid(DWORD aliasRid)
{
    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
    PSID sid = nullptr;
    if (!AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID, aliasRid, 0, 0, 0, 0, 0, 0, &sid)) {
        return SidHandle();
    }

    return SidHandle(sid);
}

SidHandle makeLocalSystemSid()
{
    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
    PSID sid = nullptr;
    if (!AllocateAndInitializeSid(&authority, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &sid)) {
        return SidHandle();
    }

    return SidHandle(sid);
}

} // namespace

bool denyTerminateForBuiltinUsers()
{
    auto users = makeBuiltinSid(DOMAIN_ALIAS_RID_USERS);
    auto admins = makeBuiltinSid(DOMAIN_ALIAS_RID_ADMINS);
    auto system = makeLocalSystemSid();

    if (users.get() == nullptr || admins.get() == nullptr || system.get() == nullptr) {
        log_warning(L"Unable to allocate SIDs for process DACL hardening");
        return false;
    }

    std::array<EXPLICIT_ACCESSW, 3> entries{};
    entries[0].grfAccessPermissions = PROCESS_TERMINATE;
    entries[0].grfAccessMode = DENY_ACCESS;
    entries[0].grfInheritance = NO_INHERITANCE;
    entries[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    entries[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    entries[0].Trustee.ptstrName = static_cast<LPWSTR>(users.get());

    entries[1].grfAccessPermissions = PROCESS_ALL_ACCESS;
    entries[1].grfAccessMode = GRANT_ACCESS;
    entries[1].grfInheritance = NO_INHERITANCE;
    entries[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    entries[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    entries[1].Trustee.ptstrName = static_cast<LPWSTR>(admins.get());

    entries[2].grfAccessPermissions = PROCESS_ALL_ACCESS;
    entries[2].grfAccessMode = GRANT_ACCESS;
    entries[2].grfInheritance = NO_INHERITANCE;
    entries[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    entries[2].Trustee.TrusteeType = TRUSTEE_IS_USER;
    entries[2].Trustee.ptstrName = static_cast<LPWSTR>(system.get());

    PACL acl = nullptr;
    const DWORD aclResult = SetEntriesInAclW(static_cast<ULONG>(entries.size()), entries.data(), nullptr, &acl);
    if (aclResult != ERROR_SUCCESS || acl == nullptr) {
        log_warning(L"SetEntriesInAclW failed for process DACL hardening");
        return false;
    }

    const DWORD setResult = SetSecurityInfo(GetCurrentProcess(), SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, acl, nullptr);
    LocalFree(acl);

    if (setResult != ERROR_SUCCESS) {
        log_warning(L"SetSecurityInfo failed for process DACL hardening");
        return false;
    }

    return true;
}

} // namespace antivirus::common::security
