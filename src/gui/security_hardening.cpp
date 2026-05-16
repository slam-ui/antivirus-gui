#include "gui/security_hardening.h"

#include "common/logging.h"
#include "common/security/process_dacl.h"

namespace antivirus::gui {

void applyGuiSecurityHardening()
{
    if (!antivirus::common::security::denyTerminateForBuiltinUsers()) {
        antivirus::common::log_warning(L"GUI process DACL hardening was not applied");
    }
}

} // namespace antivirus::gui
