#include "service/scan/AvUpdateClient.h"

#include <array>
#include <windows.h>

namespace antivirus::service::scan {
namespace {

std::filesystem::path programDataDirectory()
{
    std::array<wchar_t, MAX_PATH> buffer{};
    const DWORD length = GetEnvironmentVariableW(L"ProgramData", buffer.data(), static_cast<DWORD>(buffer.size()));

    if (length > 0 && length < buffer.size()) {
        return std::filesystem::path(buffer.data());
    }

    return std::filesystem::path(L"C:\\ProgramData");
}

} // namespace

AvUpdateClient::AvUpdateClient()
    : serverDirectory_(programDataDirectory() / L"AntivirusGuiMockServer")
{
}

std::filesystem::path AvUpdateClient::serverDirectory() const
{
    return serverDirectory_;
}

std::filesystem::path AvUpdateClient::serverDatabasePath() const
{
    return serverDirectory_ / L"avdb.bin";
}

AvDatabaseLoadResult AvUpdateClient::fetchDatabase() const
{
    AvDatabaseLoadResult result;

    std::error_code errorCode;
    if (!std::filesystem::exists(serverDatabasePath(), errorCode)) {
        result.error = AvDatabaseLoadError::FileNotFound;
        result.mockUpdateServerUnavailable = true;
        result.message = L"Mock update server unavailable";
        return result;
    }

    const AvDatabaseStorage storage(serverDirectory_);
    result = storage.loadDatabaseFile(serverDatabasePath());
    if (result.loaded) {
        result.message = L"Mock update server database loaded";
    } else if (result.message.empty()) {
        result.message = L"Mock update server database is invalid";
    }

    return result;
}

} // namespace antivirus::service::scan
