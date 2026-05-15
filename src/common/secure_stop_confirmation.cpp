#include "common/secure_stop_confirmation.h"

#include "common/logging.h"

#include <objbase.h>
#include <windows.h>
#include <wincred.h>

#include <string>

namespace antivirus::common {

bool confirmServiceStopOnSecureDesktop()
{
    CREDUI_INFOW info{};
    info.cbSize = sizeof(info);
    info.hwndParent = nullptr;
    info.pszCaptionText = L"Подтверждение остановки службы";
    info.pszMessageText =
        L"Для остановки AntivirusGuiService подтвердите действие в защищенном окне Windows.";
    info.hbmBanner = nullptr;

    ULONG authPackage = 0;
    LPVOID outAuthBuffer = nullptr;
    ULONG outAuthBufferSize = 0;
    BOOL save = FALSE;

    const DWORD flags = CREDUIWIN_SECURE_PROMPT;

    const DWORD result = CredUIPromptForWindowsCredentialsW(
        &info,
        0,
        &authPackage,
        nullptr,
        0,
        &outAuthBuffer,
        &outAuthBufferSize,
        &save,
        flags
    );

    if (outAuthBuffer != nullptr) {
        SecureZeroMemory(outAuthBuffer, outAuthBufferSize);
        CoTaskMemFree(outAuthBuffer);
    }

    if (result == ERROR_SUCCESS) {
        log_info(L"User confirmed service stop on Secure Desktop");
        return true;
    }

    if (result == ERROR_CANCELLED) {
        log_info(L"User cancelled Secure Desktop service stop confirmation");
        return false;
    }

    log_warning(L"Secure Desktop service stop confirmation failed, error=" + std::to_wstring(result));
    return false;
}

} // namespace antivirus::common
