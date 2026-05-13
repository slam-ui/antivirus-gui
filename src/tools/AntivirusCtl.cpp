#include "winui/RpcClientWin.h"

#include <windows.h>

#include <iostream>
#include <string_view>
#include <vector>

namespace {

bool hasArgument(const std::vector<std::wstring_view>& args, std::wstring_view expected)
{
    for (const std::wstring_view arg : args) {
        if (arg == expected) {
            return true;
        }
    }

    return false;
}

} // namespace

int wmain(int argc, wchar_t* argv[])
{
    std::vector<std::wstring_view> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    antivirus::winui::RpcClientWin rpcClient;

    if (hasArgument(args, L"--ping")) {
        const bool ok = rpcClient.ping();
        std::wcout << (ok ? L"RPC available\n" : L"RPC unavailable\n");
        return ok ? 0 : 1;
    }

    if (hasArgument(args, L"--request-stop")) {
        if (!rpcClient.ping()) {
            std::wcout << L"RPC unavailable; service may already be stopped\n";
            return 0;
        }

        const bool ok = rpcClient.requestServiceStop();
        std::wcout << (ok ? L"Stop request sent\n" : L"Stop request failed\n");
        return ok ? 0 : 1;
    }

    if (hasArgument(args, L"--status")) {
        std::wcout << L"Service status: " << rpcClient.serviceStatus() << L"\n";
        return 0;
    }

    std::wcout << L"Usage: AntivirusCtl.exe --ping | --status | --request-stop\n";
    return 0;
}
