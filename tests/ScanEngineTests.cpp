#include "service/scan/AvDatabase.h"
#include "service/scan/AvDatabaseStorage.h"
#include "service/scan/ScanEngine.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

namespace {

using antivirus::service::scan::AvDatabase;
using antivirus::service::scan::AvDatabaseLoadError;
using antivirus::service::scan::AvDatabaseStorage;
using antivirus::service::scan::ObjectType;
using antivirus::service::scan::ScanEngine;
using antivirus::service::scan::ScanResult;

ScanResult scanBytes(const std::string& bytes, ObjectType objectType)
{
    AvDatabase database;
    database.loadDemoDatabase();

    std::istringstream input(bytes, std::ios::binary);
    const ScanEngine engine;
    return engine.scan(input, objectType, database);
}

void expect(bool condition, const char* name, int& failures)
{
    if (!condition) {
        std::cerr << "FAILED: " << name << '\n';
        ++failures;
    }
}

std::filesystem::path uniqueTempRoot()
{
    const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (L"AntivirusGuiTests-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(ticks));
}

std::wstring readProgramData()
{
    std::array<wchar_t, 32768> buffer{};
    const DWORD length = GetEnvironmentVariableW(L"ProgramData", buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        return {};
    }

    return std::wstring(buffer.data(), length);
}

void restoreProgramData(const std::wstring& oldValue)
{
    if (oldValue.empty()) {
        SetEnvironmentVariableW(L"ProgramData", nullptr);
        return;
    }

    SetEnvironmentVariableW(L"ProgramData", oldValue.c_str());
}

bool corruptManifestSignature(const std::filesystem::path& databasePath)
{
    std::fstream file(databasePath, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        return false;
    }

    file.seekg(8, std::ios::beg);
    std::uint32_t releaseDateLength = 0;
    file.read(reinterpret_cast<char*>(&releaseDateLength), sizeof(releaseDateLength));
    if (!file.good()) {
        return false;
    }

    const std::streamoff manifestOffset = 4 + 4 + 4 + static_cast<std::streamoff>(releaseDateLength * sizeof(std::uint16_t)) + 4;
    file.seekg(manifestOffset, std::ios::beg);

    char byte = '\0';
    file.read(&byte, sizeof(byte));
    if (!file.good()) {
        return false;
    }

    byte = static_cast<char>(byte ^ 0xff);
    file.seekp(manifestOffset, std::ios::beg);
    file.write(&byte, sizeof(byte));
    return file.good();
}

bool corruptFirstRecordSignature(const std::filesystem::path& databasePath)
{
    std::fstream file(databasePath, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        return false;
    }

    file.seekg(8, std::ios::beg);
    std::uint32_t releaseDateLength = 0;
    file.read(reinterpret_cast<char*>(&releaseDateLength), sizeof(releaseDateLength));
    if (!file.good()) {
        return false;
    }

    std::streamoff offset = 4 + 4 + 4 + static_cast<std::streamoff>(releaseDateLength * sizeof(std::uint16_t)) + 4 + 32;
    offset += 8;
    offset += 4;
    offset += 32;
    offset += 8;
    offset += 8;
    offset += 4;

    file.seekg(offset, std::ios::beg);
    std::uint32_t threatNameLength = 0;
    file.read(reinterpret_cast<char*>(&threatNameLength), sizeof(threatNameLength));
    if (!file.good()) {
        return false;
    }

    offset += 4 + static_cast<std::streamoff>(threatNameLength * sizeof(std::uint16_t));
    file.seekg(offset, std::ios::beg);

    char byte = '\0';
    file.read(&byte, sizeof(byte));
    if (!file.good()) {
        return false;
    }

    byte = static_cast<char>(byte ^ 0xff);
    file.seekp(offset, std::ios::beg);
    file.write(&byte, sizeof(byte));
    return file.good();
}

} // namespace

int main()
{
    int failures = 0;

    const ScanResult peResult = scanBytes("MZAVGUI-PE-TEST", ObjectType::PeFile);
    expect(peResult.scanned, "PE sample is scanned", failures);
    expect(peResult.malicious, "PE demo signature is detected", failures);
    expect(peResult.detectionOffset == 0, "PE demo signature offset is zero", failures);

    const ScanResult psResult = scanBytes("Write-Host test\nInvoke-AvGuiTest", ObjectType::PowerShellScript);
    expect(psResult.scanned, "PowerShell sample is scanned", failures);
    expect(psResult.malicious, "PowerShell demo signature is detected", failures);

    const ScanResult cleanResult = scanBytes("MZ clean educational file", ObjectType::PeFile);
    expect(cleanResult.scanned, "Clean sample is scanned", failures);
    expect(!cleanResult.malicious, "Clean sample is not detected", failures);

    const std::string outOfRangePe = std::string(513, 'A') + "MZAVGUI-PE-TEST";
    const ScanResult outOfRangeResult = scanBytes(outOfRangePe, ObjectType::PeFile);
    expect(outOfRangeResult.scanned, "Out-of-range PE sample is scanned", failures);
    expect(!outOfRangeResult.malicious, "Out-of-range PE signature is ignored", failures);

    const ScanResult wrongTypeResult = scanBytes("MZAVGUI-PE-TEST", ObjectType::PowerShellScript);
    expect(wrongTypeResult.scanned, "Wrong object type sample is scanned", failures);
    expect(!wrongTypeResult.malicious, "Wrong object type does not match", failures);

    const std::wstring oldProgramData = readProgramData();
    const std::filesystem::path tempRoot = uniqueTempRoot();
    std::filesystem::create_directories(tempRoot);
    SetEnvironmentVariableW(L"ProgramData", tempRoot.wstring().c_str());

    AvDatabaseStorage serverStorage(tempRoot / L"AntivirusGuiMockServer");
    expect(serverStorage.writeDefaultDatabase(), "Mock update server database is written", failures);

    AvDatabaseStorage primaryStorage;
    expect(primaryStorage.writeDefaultDatabase(), "Primary database is written", failures);
    expect(corruptFirstRecordSignature(primaryStorage.databasePath()), "Primary record signature is corrupted", failures);

    const auto repairedRecord = primaryStorage.loadOrRecover();
    expect(repairedRecord.loaded, "Corrupted record database is recovered", failures);
    expect(repairedRecord.repairedFromMockUpdateServer, "Corrupted record recovery uses mock update server", failures);
    expect(repairedRecord.primaryError == AvDatabaseLoadError::InvalidRecordSignature, "Record corruption is tracked separately", failures);
    expect(repairedRecord.skippedRecordCount == 0, "Corrupted record is replaced instead of skipped", failures);
    expect(repairedRecord.records.size() == 2, "Repaired database keeps all demo records", failures);

    expect(primaryStorage.writeDefaultDatabase(), "Primary database is rewritten before manifest test", failures);
    expect(corruptManifestSignature(primaryStorage.databasePath()), "Primary manifest signature is corrupted", failures);

    const auto repaired = primaryStorage.loadOrRecover();
    expect(repaired.loaded, "Corrupted primary database is recovered", failures);
    expect(repaired.repairedFromMockUpdateServer, "Recovery uses mock update server", failures);
    expect(repaired.primaryError == AvDatabaseLoadError::InvalidManifestSignature, "Manifest corruption is tracked separately", failures);
    expect(!repaired.records.empty(), "Repaired database has records", failures);

    restoreProgramData(oldProgramData);
    std::error_code cleanupError;
    std::filesystem::remove_all(tempRoot, cleanupError);

    return failures == 0 ? 0 : 1;
}
