#include <windows.h>
#undef GetCurrentTime

#include <winrt/base.h>
#include <WindowsAppSDK-VersionInfo.h>
#include <MddBootstrap.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.UI.h>

namespace mux = winrt::Microsoft::UI::Xaml;
namespace controls = winrt::Microsoft::UI::Xaml::Controls;
namespace media = winrt::Microsoft::UI::Xaml::Media;

namespace {

controls::TextBlock makeTextBlock(const wchar_t* text, double fontSize = 14.0)
{
    controls::TextBlock block;
    block.Text(text);
    block.FontSize(fontSize);
    block.TextWrapping(mux::TextWrapping::Wrap);
    return block;
}

controls::Button makeButton(const wchar_t* text)
{
    controls::Button button;
    button.Content(winrt::box_value(winrt::hstring{text}));
    button.Margin(mux::Thickness{0, 0, 8, 8});
    return button;
}

struct WinUiApp : winrt::implements<WinUiApp, mux::IApplicationOverrides> {
    void OnLaunched(mux::LaunchActivatedEventArgs const&)
    {
        window_ = mux::Window();
        window_.Title(L"Антивирус GUI — WinUI");

        controls::ScrollViewer scrollViewer;
        scrollViewer.HorizontalScrollBarVisibility(controls::ScrollBarVisibility::Disabled);
        scrollViewer.VerticalScrollBarVisibility(controls::ScrollBarVisibility::Auto);

        controls::StackPanel root;
        root.Padding(mux::Thickness{24});
        root.Spacing(12);
        root.Background(media::SolidColorBrush(winrt::Windows::UI::Color{255, 248, 250, 252}));

        controls::TextBlock title = makeTextBlock(L"Антивирус GUI — WinUI", 26);
        title.Margin(mux::Thickness{0, 0, 0, 8});
        root.Children().Append(title);

        root.Children().Append(makeTextBlock(L"Служба: статус будет получен через RPC"));
        root.Children().Append(makeTextBlock(L"Аккаунт: не проверен"));
        root.Children().Append(makeTextBlock(L"Лицензия: не проверена"));
        root.Children().Append(makeTextBlock(L"Антивирусные базы: не проверены"));
        root.Children().Append(makeTextBlock(L"Мониторинг: выключен"));

        controls::StackPanel actions;
        actions.Margin(mux::Thickness{0, 8, 0, 8});
        actions.Spacing(4);
        actions.Children().Append(makeButton(L"Войти"));
        actions.Children().Append(makeButton(L"Выйти"));
        actions.Children().Append(makeButton(L"Активировать"));
        actions.Children().Append(makeButton(L"Сканировать файл"));
        actions.Children().Append(makeButton(L"Сканировать папку"));
        actions.Children().Append(makeButton(L"Сканировать все несъёмные диски"));
        actions.Children().Append(makeButton(L"Запустить мониторинг"));
        actions.Children().Append(makeButton(L"Остановить мониторинг"));
        actions.Children().Append(makeButton(L"Остановить службу"));
        root.Children().Append(actions);

        controls::TextBox resultBox;
        resultBox.Header(winrt::box_value(winrt::hstring{L"Результат"}));
        resultBox.PlaceholderText(L"Здесь будут отображаться результаты последней операции.");
        resultBox.AcceptsReturn(true);
        resultBox.IsReadOnly(true);
        resultBox.MinHeight(220);
        root.Children().Append(resultBox);

        scrollViewer.Content(root);
        window_.Content(scrollViewer);
        window_.Activate();
    }

private:
    mux::Window window_{nullptr};
};

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    const auto bootstrap = Microsoft::Windows::ApplicationModel::DynamicDependency::Bootstrap::Initialize();

    mux::Application::Start([](auto&&) {
        winrt::make<WinUiApp>();
    });

    return 0;
}
