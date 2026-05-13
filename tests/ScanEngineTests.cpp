#include "service/scan/AvDatabase.h"
#include "service/scan/ScanEngine.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {

using antivirus::service::scan::AvDatabase;
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

    return failures == 0 ? 0 : 1;
}
