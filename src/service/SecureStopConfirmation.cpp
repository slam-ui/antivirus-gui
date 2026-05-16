#include "service/SecureStopConfirmation.h"

#include "common/logging.h"

#include <windows.h>
#include <objbase.h>
#include <wincred.h>

namespace antivirus::service {

bool confirmServiceStopOnSecureDesktop()
{
    CREDUI_INFOW info{};
    info.cbSize = sizeof(info);
    info.pszCaptionText = L"Antivirus GUI Service";
    info.pszMessageText = L"Подтвердите учетные данные Windows для остановки службы.";

    ULONG authPackage = 0;
    void* authBuffer = nullptr;
    ULONG authBufferSize = 0;
    BOOL save = FALSE;

    const DWORD promptResult = CredUIPromptForWindowsCredentialsW(&info,
                                                                 0,
                                                                 &authPackage,
                                                                 nullptr,
                                                                 0,
                                                                 &authBuffer,
                                                                 &authBufferSize,
                                                                 &save,
                                                                 CREDUIWIN_GENERIC | CREDUIWIN_SECURE_PROMPT);
    if (promptResult != NO_ERROR) {
        antivirus::common::log_warning(L"Secure credential prompt was cancelled or unavailable");
        return false;
    }

    wchar_t userName[CREDUI_MAX_USERNAME_LENGTH + 1]{};
    wchar_t domain[CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1]{};
    wchar_t password[CREDUI_MAX_PASSWORD_LENGTH + 1]{};
    DWORD userNameSize = _countof(userName);
    DWORD domainSize = _countof(domain);
    DWORD passwordSize = _countof(password);

    const BOOL unpacked = CredUnPackAuthenticationBufferW(0,
                                                         authBuffer,
                                                         authBufferSize,
                                                         userName,
                                                         &userNameSize,
                                                         domain,
                                                         &domainSize,
                                                         password,
                                                         &passwordSize);
    if (authBuffer != nullptr) {
        SecureZeroMemory(authBuffer, authBufferSize);
        CoTaskMemFree(authBuffer);
    }

    if (!unpacked) {
        SecureZeroMemory(password, sizeof(password));
        antivirus::common::log_warning(L"Unable to unpack secure credentials");
        return false;
    }

    HANDLE validatedToken = nullptr;
    const BOOL valid = LogonUserW(userName,
                                 domain[0] != L'\0' ? domain : nullptr,
                                 password,
                                 LOGON32_LOGON_INTERACTIVE,
                                 LOGON32_PROVIDER_DEFAULT,
                                 &validatedToken);
    SecureZeroMemory(password, sizeof(password));

    if (validatedToken != nullptr) {
        CloseHandle(validatedToken);
    }

    if (!valid) {
        antivirus::common::log_warning(L"Secure credential validation failed");
        return false;
    }

    return true;
}

} // namespace antivirus::service
