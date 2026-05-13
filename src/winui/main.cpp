#include "common/secure_stop_confirmation.h"
#include "winui/RpcClientWin.h"

#include <windows.h>
#undef GetCurrentTime

#include <commdlg.h>
#include <shlobj.h>

#include <winrt/base.h>
#include <WindowsAppSDK-VersionInfo.h>
#include <MddBootstrap.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.h>

#include <atomic>
#include <cstdio>
#include <exception>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

namespace mux = winrt::Microsoft::UI::Xaml;
namespace controls = winrt::Microsoft::UI::Xaml::Controls;
namespace dispatching = winrt::Microsoft::UI::Dispatching;
namespace markup = winrt::Microsoft::UI::Xaml::Markup;
namespace media = winrt::Microsoft::UI::Xaml::Media;
namespace xamltype = winrt::Microsoft::UI::Xaml::XamlTypeInfo;

namespace {

winrt::Windows::UI::Color color(unsigned char red, unsigned char green, unsigned char blue)
{
    return winrt::Windows::UI::Color{255, red, green, blue};
}

media::SolidColorBrush brush(unsigned char red, unsigned char green, unsigned char blue)
{
    return media::SolidColorBrush(color(red, green, blue));
}

std::wstring formatHresult(winrt::hresult code)
{
    wchar_t buffer[16]{};
    swprintf_s(buffer, L"0x%08X", static_cast<unsigned int>(code.value));
    return buffer;
}

std::wstring widenAscii(std::string_view text)
{
    std::wstring result;
    result.reserve(text.size());
    for (const char ch : text) {
        result.push_back(static_cast<unsigned char>(ch));
    }
    return result;
}

void showStartupError(const std::wstring& message)
{
    MessageBoxW(nullptr, message.c_str(), L"Antivirus GUI", MB_OK | MB_ICONERROR);
}

controls::TextBlock makeTextBlock(const wchar_t* text, double fontSize = 14.0)
{
    controls::TextBlock block;
    block.Text(text);
    block.FontSize(fontSize);
    block.TextWrapping(mux::TextWrapping::Wrap);
    return block;
}

controls::TextBlock makeStatusText(const wchar_t* text)
{
    controls::TextBlock block = makeTextBlock(text, 14);
    block.Foreground(brush(51, 65, 85));
    return block;
}

controls::Button makeButton(const wchar_t* text)
{
    controls::Button button;
    button.Content(winrt::box_value(winrt::hstring{text}));
    button.MinHeight(34);
    button.MinWidth(168);
    button.Margin(mux::Thickness{0, 0, 8, 8});
    return button;
}

controls::StackPanel makeCard(const wchar_t* title)
{
    controls::StackPanel content;
    content.Spacing(8);

    controls::TextBlock header = makeTextBlock(title, 16);
    header.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
    header.Foreground(brush(15, 23, 42));
    content.Children().Append(header);

    return content;
}

controls::Border wrapCard(controls::StackPanel const& content)
{
    controls::Border border;
    border.Background(brush(255, 255, 255));
    border.BorderBrush(brush(226, 232, 240));
    border.BorderThickness(mux::Thickness{1});
    border.CornerRadius(mux::CornerRadius{8});
    border.Padding(mux::Thickness{16});
    border.Child(content);
    return border;
}

std::wstring valueOrDash(const std::wstring& value)
{
    return value.empty() ? L"-" : value;
}

std::wstring serviceStatusText(long status, bool rpcAvailable)
{
    if (!rpcAvailable) {
        return L"Служба: RPC недоступен";
    }

    switch (status) {
    case SERVICE_STOPPED:
        return L"Служба: остановлена";
    case SERVICE_START_PENDING:
        return L"Служба: запускается";
    case SERVICE_STOP_PENDING:
        return L"Служба: останавливается";
    case SERVICE_RUNNING:
        return L"Служба работает";
    case SERVICE_PAUSED:
        return L"Служба: приостановлена";
    default:
        return L"Служба: состояние " + std::to_wstring(status);
    }
}

std::wstring authText(const antivirus::winui::AuthState& auth)
{
    if (!auth.lastError.empty()) {
        return L"Аккаунт: ошибка: " + auth.lastError;
    }

    if (auth.authenticated) {
        return L"Аккаунт: выполнен вход, " + valueOrDash(auth.displayName) + L" (" + valueOrDash(auth.login) + L")";
    }

    return L"Аккаунт: требуется вход";
}

std::wstring licenseText(const antivirus::winui::LicenseState& license)
{
    if (!license.lastError.empty()) {
        return L"Лицензия: ошибка: " + license.lastError;
    }

    if (license.licenseActive) {
        return L"Лицензия активна до " + valueOrDash(license.licenseExpiresAt);
    }

    if (!license.featureBlockedReason.empty()) {
        return L"Лицензия: " + license.featureBlockedReason;
    }

    return L"Лицензия: требуется активация";
}

std::wstring featureText(const antivirus::winui::FeatureState& feature)
{
    if (feature.functionalityEnabled) {
        return L"Функции сканирования и мониторинга доступны";
    }

    return L"Функции заблокированы: " + valueOrDash(feature.blockedReason);
}

std::wstring databaseText(const antivirus::winui::DatabaseInfo& database)
{
    if (database.loaded) {
        return L"Базы загружены: " + valueOrDash(database.releaseDate)
            + L", записей: " + std::to_wstring(database.recordCount);
    }

    if (!database.lastError.empty()) {
        return L"Базы: " + database.lastError;
    }

    return L"Базы не загружены";
}

std::wstring monitorText(const antivirus::winui::DirectoryMonitorStatus& status)
{
    if (status.running) {
        return L"Мониторинг включён: " + valueOrDash(status.path);
    }

    if (!status.lastError.empty()) {
        return L"Мониторинг: " + status.lastError;
    }

    return L"Мониторинг выключен";
}

std::wstring formatScanResult(const antivirus::winui::ScanResult& result)
{
    std::wstringstream stream;

    if (!result.lastError.empty()) {
        stream << L"Ошибка: " << result.lastError << L"\n\n";
    }

    if (!result.details.empty()) {
        stream << result.details;
        return stream.str();
    }

    stream << L"Путь: " << valueOrDash(result.scannedPath) << L"\n";
    stream << L"Просканировано файлов: " << result.scannedFiles << L"\n";
    stream << L"Обнаружено угроз: " << result.maliciousFiles << L"\n";
    stream << (result.malicious ? L"Результат: обнаружена угроза" : L"Результат: угроз не обнаружено");

    if (result.malicious) {
        stream << L"\nУгроза: " << valueOrDash(result.threatName);
        stream << L"\nТип объекта: " << valueOrDash(result.objectType);
        stream << L"\nСмещение: " << result.detectionOffset;
    }

    return stream.str();
}

std::optional<std::wstring> pickFile()
{
    wchar_t buffer[MAX_PATH]{};

    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = static_cast<DWORD>(std::size(buffer));
    dialog.lpstrTitle = L"Выберите файл для сканирования";
    dialog.lpstrFilter = L"Все файлы\0*.*\0";
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!GetOpenFileNameW(&dialog)) {
        return std::nullopt;
    }

    return std::wstring(buffer);
}

std::optional<std::wstring> pickFolder(const wchar_t* title)
{
    BROWSEINFOW dialog{};
    dialog.lpszTitle = title;
    dialog.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

    PIDLIST_ABSOLUTE item = SHBrowseForFolderW(&dialog);
    if (item == nullptr) {
        return std::nullopt;
    }

    wchar_t path[MAX_PATH]{};
    const bool ok = SHGetPathFromIDListW(item, path) != FALSE;
    CoTaskMemFree(item);

    if (!ok) {
        return std::nullopt;
    }

    return std::wstring(path);
}

struct WinUiApp : mux::ApplicationT<WinUiApp, markup::IXamlMetadataProvider> {
    void OnLaunched(mux::LaunchActivatedEventArgs const&)
    {
        buildUi();
        refreshState();

        pollingTimer_ = mux::DispatcherTimer();
        pollingTimer_.Interval(std::chrono::seconds(5));
        pollingTimer_.Tick([this](auto&&, auto&&) {
            refreshState();
        });
        pollingTimer_.Start();
    }

    markup::IXamlType GetXamlType(winrt::Windows::UI::Xaml::Interop::TypeName const& type)
    {
        return xamlProvider_.GetXamlType(type);
    }

    markup::IXamlType GetXamlType(winrt::hstring const& fullName)
    {
        return xamlProvider_.GetXamlType(fullName);
    }

    winrt::com_array<markup::XmlnsDefinition> GetXmlnsDefinitions()
    {
        return xamlProvider_.GetXmlnsDefinitions();
    }

private:
    void buildUi()
    {
        window_ = mux::Window();
        window_.Title(L"Антивирус GUI - WinUI");

        controls::ScrollViewer scrollViewer;
        scrollViewer.HorizontalScrollBarVisibility(controls::ScrollBarVisibility::Disabled);
        scrollViewer.VerticalScrollBarVisibility(controls::ScrollBarVisibility::Auto);
        scrollViewer.Background(brush(248, 250, 252));

        controls::StackPanel root;
        root.Padding(mux::Thickness{24});
        root.Spacing(14);
        root.Background(brush(248, 250, 252));

        controls::TextBlock title = makeTextBlock(L"Антивирус GUI", 28);
        title.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
        title.Foreground(brush(15, 23, 42));
        root.Children().Append(title);

        controls::TextBlock subtitle = makeTextBlock(L"Учебный C++20 / WinUI интерфейс для службы, RPC, лицензии и сканирования.", 14);
        subtitle.Foreground(brush(71, 85, 105));
        root.Children().Append(subtitle);

        controls::StackPanel serviceCard = makeCard(L"Статус службы");
        serviceLabel_ = makeStatusText(L"Служба: проверка...");
        serviceCard.Children().Append(serviceLabel_);
        root.Children().Append(wrapCard(serviceCard));

        controls::StackPanel accountCard = makeCard(L"Аккаунт и лицензия");
        accountLabel_ = makeStatusText(L"Аккаунт: не проверен");
        licenseLabel_ = makeStatusText(L"Лицензия: не проверена");
        featureLabel_ = makeStatusText(L"Функции: не проверены");
        accountCard.Children().Append(accountLabel_);
        accountCard.Children().Append(licenseLabel_);
        accountCard.Children().Append(featureLabel_);

        loginBox_ = controls::TextBox();
        loginBox_.Header(winrt::box_value(winrt::hstring{L"Логин"}));
        loginBox_.PlaceholderText(L"demo");
        loginBox_.MaxWidth(360);
        loginBox_.HorizontalAlignment(mux::HorizontalAlignment::Left);

        passwordBox_ = controls::PasswordBox();
        passwordBox_.Header(winrt::box_value(winrt::hstring{L"Пароль"}));
        passwordBox_.PlaceholderText(L"demo");
        passwordBox_.MaxWidth(360);
        passwordBox_.HorizontalAlignment(mux::HorizontalAlignment::Left);

        activationBox_ = controls::TextBox();
        activationBox_.Header(winrt::box_value(winrt::hstring{L"Код активации"}));
        activationBox_.PlaceholderText(L"DEMO-1234");
        activationBox_.MaxWidth(360);
        activationBox_.HorizontalAlignment(mux::HorizontalAlignment::Left);

        controls::StackPanel accountButtons;
        accountButtons.Orientation(controls::Orientation::Horizontal);
        accountButtons.Children().Append(loginButton_ = makeButton(L"Войти"));
        accountButtons.Children().Append(logoutButton_ = makeButton(L"Выйти"));
        accountButtons.Children().Append(activateButton_ = makeButton(L"Активировать лицензию"));

        loginButton_.Click([this](auto&&, auto&&) {
            login();
        });
        logoutButton_.Click([this](auto&&, auto&&) {
            logout();
        });
        activateButton_.Click([this](auto&&, auto&&) {
            activateLicense();
        });

        accountCard.Children().Append(loginBox_);
        accountCard.Children().Append(passwordBox_);
        accountCard.Children().Append(activationBox_);
        accountCard.Children().Append(accountButtons);
        root.Children().Append(wrapCard(accountCard));

        controls::StackPanel databaseCard = makeCard(L"Антивирусные базы");
        databaseLabel_ = makeStatusText(L"Базы: не проверены");
        databaseCard.Children().Append(databaseLabel_);
        root.Children().Append(wrapCard(databaseCard));

        controls::StackPanel scanCard = makeCard(L"Сканирование");
        controls::TextBlock scanHint = makeStatusText(L"Сканирование выполняется через RPC в службе. Если лицензия не активна, кнопки недоступны.");
        scanCard.Children().Append(scanHint);

        controls::StackPanel scanButtons;
        scanButtons.Orientation(controls::Orientation::Horizontal);
        scanButtons.Children().Append(scanFileButton_ = makeButton(L"Сканировать файл"));
        scanButtons.Children().Append(scanDirectoryButton_ = makeButton(L"Сканировать папку"));
        scanButtons.Children().Append(scanFixedDrivesButton_ = makeButton(L"Сканировать все несъёмные диски"));
        scanCard.Children().Append(scanButtons);

        scanFileButton_.Click([this](auto&&, auto&&) {
            scanFile();
        });
        scanDirectoryButton_.Click([this](auto&&, auto&&) {
            scanDirectory();
        });
        scanFixedDrivesButton_.Click([this](auto&&, auto&&) {
            scanFixedDrives();
        });
        root.Children().Append(wrapCard(scanCard));

        controls::StackPanel monitorCard = makeCard(L"Мониторинг");
        monitorLabel_ = makeStatusText(L"Мониторинг: не проверен");
        monitorCard.Children().Append(monitorLabel_);

        controls::StackPanel monitorButtons;
        monitorButtons.Orientation(controls::Orientation::Horizontal);
        monitorButtons.Children().Append(startMonitorButton_ = makeButton(L"Запустить мониторинг папки"));
        monitorButtons.Children().Append(stopMonitorButton_ = makeButton(L"Остановить мониторинг"));
        monitorCard.Children().Append(monitorButtons);

        startMonitorButton_.Click([this](auto&&, auto&&) {
            startMonitoring();
        });
        stopMonitorButton_.Click([this](auto&&, auto&&) {
            stopMonitoring();
        });
        root.Children().Append(wrapCard(monitorCard));

        controls::StackPanel serviceActionsCard = makeCard(L"Управление службой");
        controls::TextBlock stopHint = makeStatusText(L"Остановка службы требует подтверждения в защищённом окне Windows.");
        serviceActionsCard.Children().Append(stopHint);
        serviceActionsCard.Children().Append(stopServiceButton_ = makeButton(L"Остановить службу"));
        stopServiceButton_.Click([this](auto&&, auto&&) {
            stopService();
        });
        root.Children().Append(wrapCard(serviceActionsCard));

        controls::StackPanel resultCard = makeCard(L"Результат и журнал последней операции");
        busyLabel_ = makeStatusText(L"Последняя операция: ожидание действия");
        resultBox_ = controls::TextBox();
        resultBox_.PlaceholderText(L"Здесь будут отображаться результаты сканирования, мониторинга, входа и активации.");
        resultBox_.AcceptsReturn(true);
        resultBox_.IsReadOnly(true);
        resultBox_.TextWrapping(mux::TextWrapping::Wrap);
        resultBox_.MinHeight(240);
        resultCard.Children().Append(busyLabel_);
        resultCard.Children().Append(resultBox_);
        root.Children().Append(wrapCard(resultCard));

        scrollViewer.Content(root);
        window_.Content(scrollViewer);
        window_.Activate();
    }

    void refreshState()
    {
        if (busy_.load()) {
            return;
        }

        const bool rpcAvailable = rpcClient_.ping();
        serviceLabel_.Text(serviceStatusText(rpcClient_.serviceStatus(), rpcAvailable));

        const auto auth = rpcClient_.authState();
        const auto license = rpcClient_.licenseState();
        const auto feature = rpcClient_.featureState();
        const auto database = rpcClient_.databaseInfo();
        const auto monitor = rpcClient_.directoryMonitorStatus();

        accountLabel_.Text(authText(auth));
        licenseLabel_.Text(licenseText(license));
        featureLabel_.Text(featureText(feature));
        databaseLabel_.Text(databaseText(database));
        monitorLabel_.Text(monitorText(monitor));

        setActionButtonsEnabled(feature.functionalityEnabled, monitor.running, rpcAvailable);
    }

    void setActionButtonsEnabled(bool featureEnabled, bool monitorRunning, bool rpcAvailable)
    {
        const bool canUseFeature = featureEnabled && !busy_.load();
        const bool canCallRpc = rpcAvailable && !busy_.load();

        loginButton_.IsEnabled(canCallRpc);
        logoutButton_.IsEnabled(canCallRpc);
        activateButton_.IsEnabled(canCallRpc);
        scanFileButton_.IsEnabled(canUseFeature);
        scanDirectoryButton_.IsEnabled(canUseFeature);
        scanFixedDrivesButton_.IsEnabled(canUseFeature);
        startMonitorButton_.IsEnabled(canUseFeature);
        stopMonitorButton_.IsEnabled(canUseFeature && monitorRunning);
        stopServiceButton_.IsEnabled(canCallRpc);
    }

    void setAllButtonsEnabled(bool enabled)
    {
        loginButton_.IsEnabled(enabled);
        logoutButton_.IsEnabled(enabled);
        activateButton_.IsEnabled(enabled);
        scanFileButton_.IsEnabled(enabled);
        scanDirectoryButton_.IsEnabled(enabled);
        scanFixedDrivesButton_.IsEnabled(enabled);
        startMonitorButton_.IsEnabled(enabled);
        stopMonitorButton_.IsEnabled(enabled);
        stopServiceButton_.IsEnabled(enabled);
    }

    void runBackground(const std::wstring& progressText, std::function<std::wstring()> operation)
    {
        if (busy_.exchange(true)) {
            resultBox_.Text(L"Операция уже выполняется. Дождитесь завершения.");
            return;
        }

        setAllButtonsEnabled(false);
        busyLabel_.Text(L"Последняя операция: выполняется");
        resultBox_.Text(progressText);

        dispatching::DispatcherQueue dispatcher = window_.DispatcherQueue();
        std::thread([this, dispatcher, operation = std::move(operation)]() mutable {
            std::wstring resultText;

            try {
                resultText = operation();
            } catch (...) {
                resultText = L"Ошибка: операция завершилась с исключением.";
            }

            dispatcher.TryEnqueue([this, resultText = std::move(resultText)]() {
                resultBox_.Text(resultText);
                busyLabel_.Text(L"Последняя операция выполнена");
                busy_.store(false);
                refreshState();
            });
        }).detach();
    }

    void login()
    {
        std::wstring login = loginBox_.Text().c_str();
        std::wstring password = passwordBox_.Password().c_str();

        runBackground(L"Выполняется вход...", [this, login = std::move(login), password = std::move(password)]() {
            const auto state = rpcClient_.login(login, password);
            if (!state.lastError.empty()) {
                return L"Вход не выполнен: " + state.lastError;
            }

            return state.authenticated
                ? L"Вход выполнен: " + valueOrDash(state.displayName)
                : std::wstring(L"Вход не выполнен. Проверьте логин и пароль.");
        });
    }

    void logout()
    {
        runBackground(L"Выполняется выход...", [this]() {
            const auto state = rpcClient_.logout();
            if (!state.lastError.empty()) {
                return L"Выход не выполнен: " + state.lastError;
            }

            return std::wstring(L"Выход выполнен. Функции сканирования и мониторинга заблокированы до нового входа и активации.");
        });
    }

    void activateLicense()
    {
        std::wstring activationCode = activationBox_.Text().c_str();

        runBackground(L"Выполняется активация лицензии...", [this, activationCode = std::move(activationCode)]() {
            const auto state = rpcClient_.activateProduct(activationCode);
            if (!state.lastError.empty()) {
                return L"Активация не выполнена: " + state.lastError;
            }

            return state.licenseActive
                ? L"Лицензия активна до " + valueOrDash(state.licenseExpiresAt)
                : L"Лицензия не активирована: " + valueOrDash(state.featureBlockedReason);
        });
    }

    void scanFile()
    {
        const std::optional<std::wstring> path = pickFile();
        if (!path.has_value()) {
            return;
        }

        runBackground(L"Сканирование файла...\n" + *path, [this, path = *path]() {
            return formatScanResult(rpcClient_.scanFile(path));
        });
    }

    void scanDirectory()
    {
        const std::optional<std::wstring> path = pickFolder(L"Выберите папку для сканирования");
        if (!path.has_value()) {
            return;
        }

        runBackground(L"Сканирование папки...\n" + *path, [this, path = *path]() {
            return formatScanResult(rpcClient_.scanDirectory(path));
        });
    }

    void scanFixedDrives()
    {
        runBackground(L"Сканирование всех несъёмных дисков...", [this]() {
            return formatScanResult(rpcClient_.scanFixedDrives());
        });
    }

    void startMonitoring()
    {
        const std::optional<std::wstring> path = pickFolder(L"Выберите папку для мониторинга");
        if (!path.has_value()) {
            return;
        }

        runBackground(L"Запуск мониторинга папки...\n" + *path, [this, path = *path]() {
            const auto status = rpcClient_.startDirectoryMonitor(path);
            if (!status.lastError.empty()) {
                return L"Мониторинг не запущен: " + status.lastError;
            }

            return L"Мониторинг запущен: " + valueOrDash(status.path);
        });
    }

    void stopMonitoring()
    {
        runBackground(L"Остановка мониторинга...", [this]() {
            const auto status = rpcClient_.stopDirectoryMonitor();
            if (!status.lastError.empty()) {
                return L"Мониторинг остановлен с предупреждением: " + status.lastError;
            }

            return std::wstring(L"Мониторинг остановлен.");
        });
    }

    void stopService()
    {
        if (!antivirus::common::confirmServiceStopOnSecureDesktop()) {
            resultBox_.Text(L"Остановка службы отменена пользователем.");
            busyLabel_.Text(L"Последняя операция отменена");
            return;
        }

        runBackground(L"Отправляется запрос на остановку службы...", [this]() {
            return rpcClient_.requestServiceStop()
                ? L"Запрос на остановку службы отправлен."
                : L"Не удалось отправить запрос на остановку службы через RPC.";
        });
    }

    antivirus::winui::RpcClientWin rpcClient_;
    std::atomic_bool busy_{false};

    mux::Window window_{nullptr};
    mux::DispatcherTimer pollingTimer_{nullptr};

    controls::TextBlock serviceLabel_{nullptr};
    controls::TextBlock accountLabel_{nullptr};
    controls::TextBlock licenseLabel_{nullptr};
    controls::TextBlock featureLabel_{nullptr};
    controls::TextBlock databaseLabel_{nullptr};
    controls::TextBlock monitorLabel_{nullptr};
    controls::TextBlock busyLabel_{nullptr};

    controls::TextBox loginBox_{nullptr};
    controls::PasswordBox passwordBox_{nullptr};
    controls::TextBox activationBox_{nullptr};
    controls::TextBox resultBox_{nullptr};

    controls::Button loginButton_{nullptr};
    controls::Button logoutButton_{nullptr};
    controls::Button activateButton_{nullptr};
    controls::Button scanFileButton_{nullptr};
    controls::Button scanDirectoryButton_{nullptr};
    controls::Button scanFixedDrivesButton_{nullptr};
    controls::Button startMonitorButton_{nullptr};
    controls::Button stopMonitorButton_{nullptr};
    controls::Button stopServiceButton_{nullptr};

    xamltype::XamlControlsXamlMetaDataProvider xamlProvider_;
};

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    try {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        const auto bootstrap = Microsoft::Windows::ApplicationModel::DynamicDependency::Bootstrap::Initialize();

        mux::Application::Start([](auto&&) {
            static winrt::com_ptr<WinUiApp> app;
            app = winrt::make_self<WinUiApp>();
        });
    } catch (const winrt::hresult_error& error) {
        showStartupError(
            L"Не удалось запустить WinUI-интерфейс.\nHRESULT: " + formatHresult(error.code()) + L"\n" +
            std::wstring(error.message().c_str()));
        return 1;
    } catch (const std::exception& error) {
        showStartupError(L"Не удалось запустить WinUI-интерфейс.\n" + widenAscii(error.what()));
        return 1;
    } catch (...) {
        showStartupError(L"Не удалось запустить WinUI-интерфейс: неизвестная ошибка.");
        return 1;
    }

    return 0;
}
