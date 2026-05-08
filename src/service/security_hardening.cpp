#include "service/security_hardening.h"

#include "common/logging.h"
#include "common/security/process_dacl.h"

namespace antivirus::service {

void applyServiceSecurityHardening()
{
    if (!antivirus::common::security::denyTerminateForBuiltinUsers()) {
        antivirus::common::log_warning(L"Service process DACL hardening was not applied");
    }
}

} // namespace antivirus::service
