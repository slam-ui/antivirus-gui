#include "gui/MainWindow.h"

#include "gui/ActivationDialog.h"
#include "gui/AppLifecycle.h"
#include "gui/AuthDialog.h"
#include "gui/RpcClient.h"

#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace antivirus::gui {
namespace {

QString formatScanResult(const ScanResult& result)
{
    QString text;

    if (!result.lastError.isEmpty()) {
        text += QStringLiteral("РћС€РёР±РєР°: %1\n\n").arg(result.lastError);
    }

    if (!result.details.isEmpty()) {
        text += result.details;
    } else {
        text += result.malicious
            ? QStringLiteral("Р РµР·СѓР»СЊС‚Р°С‚: РѕР±РЅР°СЂСѓР¶РµРЅР° СѓРіСЂРѕР·Р°")
            : QStringLiteral("Р РµР·СѓР»СЊС‚Р°С‚: СѓРіСЂРѕР· РЅРµ РѕР±РЅР°СЂСѓР¶РµРЅРѕ");
    }

    return text;
}

} // namespace

MainWindow::MainWindow(AppLifecycle& lifecycle, RpcClient& rpcClient, QWidget* parent)
    : QMainWindow(parent)
    , lifecycle_(lifecycle)
    , rpcClient_(rpcClient)
{
    setWindowTitle(QStringLiteral("РђРЅС‚РёРІРёСЂСѓСЃ GUI"));
    resize(820, 620);

    auto* fileMenu = menuBar()->addMenu(QStringLiteral("Р¤Р°Р№Р»"));
    auto* exitAction = fileMenu->addAction(QStringLiteral("Р’С‹С…РѕРґ"));
    connect(exitAction, &QAction::triggered, this, [this]() {
        lifecycle_.quitApplication();
    });

    auto* accountMenu = menuBar()->addMenu(QStringLiteral("РђРєРєР°СѓРЅС‚"));
    auto* loginAction = accountMenu->addAction(QStringLiteral("Р’РѕР№С‚Рё"));
    auto* logoutAction = accountMenu->addAction(QStringLiteral("Р’С‹Р№С‚Рё РёР· Р°РєРєР°СѓРЅС‚Р°"));
    connect(loginAction, &QAction::triggered, this, [this]() {
        showLoginFlow();
    });
    connect(logoutAction, &QAction::triggered, this, [this]() {
        logout();
    });

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    auto* title = new QLabel(QStringLiteral("РђРЅС‚РёРІРёСЂСѓСЃ GUI"), central);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);

    accountLabel_ = new QLabel(QStringLiteral("РђРєРєР°СѓРЅС‚: РЅРµ РїСЂРѕРІРµСЂРµРЅ"), central);
    accountLabel_->setWordWrap(true);

    licenseLabel_ = new QLabel(QStringLiteral("Р›РёС†РµРЅР·РёСЏ: РЅРµ РїСЂРѕРІРµСЂРµРЅР°"), central);
    licenseLabel_->setWordWrap(true);

    featureLabel_ = new QLabel(QStringLiteral("Р¤СѓРЅРєС†РёРё: Р·Р°Р±Р»РѕРєРёСЂРѕРІР°РЅС‹"), central);
    featureLabel_->setWordWrap(true);

    databaseLabel_ = new QLabel(QStringLiteral("РђРЅС‚РёРІРёСЂСѓСЃРЅС‹Рµ Р±Р°Р·С‹: РЅРµ Р·Р°РіСЂСѓР¶РµРЅС‹"), central);
    databaseLabel_->setWordWrap(true);

    auto* note = new QLabel(QStringLiteral("Р—Р°РєСЂС‹С‚РёРµ РѕРєРЅР° СЃРєСЂС‹РІР°РµС‚ РёРЅС‚РµСЂС„РµР№СЃ, РїСЂРёР»РѕР¶РµРЅРёРµ РїСЂРѕРґРѕР»Р¶Р°РµС‚ СЂР°Р±РѕС‚Р°С‚СЊ РІ С„РѕРЅРµ."), central);
    note->setWordWrap(true);

    auto* scanButtonsLayout = new QHBoxLayout();
    scanFileButton_ = new QPushButton(QStringLiteral("РЎРєР°РЅРёСЂРѕРІР°С‚СЊ С„Р°Р№Р»"), central);
    scanDirectoryButton_ = new QPushButton(QStringLiteral("РЎРєР°РЅРёСЂРѕРІР°С‚СЊ РїР°РїРєСѓ"), central);
    scanFixedDrivesButton_ = new QPushButton(QStringLiteral("Scan all fixed drives"), central);

    scanButtonsLayout->addWidget(scanFileButton_);
    scanButtonsLayout->addWidget(scanDirectoryButton_);
    scanButtonsLayout->addWidget(scanFixedDrivesButton_);

    connect(scanFileButton_, &QPushButton::clicked, this, [this]() {
        scanFile();
    });
    connect(scanDirectoryButton_, &QPushButton::clicked, this, [this]() {
        scanDirectory();
    });    connect(scanFixedDrivesButton_, &QPushButton::clicked, this, [this]() {
        scanFixedDrives();
    });


    scanResultEdit_ = new QTextEdit(central);
    scanResultEdit_->setReadOnly(true);
    scanResultEdit_->setPlaceholderText(QStringLiteral("Р—РґРµСЃСЊ Р±СѓРґСѓС‚ РѕС‚РѕР±СЂР°Р¶Р°С‚СЊСЃСЏ СЂРµР·СѓР»СЊС‚Р°С‚С‹ СЃРєР°РЅРёСЂРѕРІР°РЅРёСЏ."));

    auto* exitButton = new QPushButton(QStringLiteral("Р’С‹С…РѕРґ"), central);
    connect(exitButton, &QPushButton::clicked, this, [this]() {
        lifecycle_.quitApplication();
    });

    layout->addWidget(title);
    layout->addWidget(accountLabel_);
    layout->addWidget(licenseLabel_);
    layout->addWidget(featureLabel_);
    layout->addWidget(databaseLabel_);
    layout->addWidget(note);
    layout->addLayout(scanButtonsLayout);
    layout->addWidget(scanResultEdit_, 1);
    layout->addWidget(exitButton);

    setCentralWidget(central);
    statusBar()->showMessage(QStringLiteral("РЈС‡РµР±РЅС‹Р№ C++20 / Qt 6 РїСЂРѕРµРєС‚"));

    QTimer::singleShot(0, this, [this]() {
        updateAccountState();
    });

    auto* pollingTimer = new QTimer(this);
    pollingTimer->setInterval(7000);
    connect(pollingTimer, &QTimer::timeout, this, [this]() {
        updateAccountState();
    });
    pollingTimer->start();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->ignore();
    hide();
}

void MainWindow::updateAccountState()
{
    const auto auth = rpcClient_.authState();
    const auto license = rpcClient_.licenseState();
    const auto feature = rpcClient_.featureState();

    if (auth.authenticated) {
        accountLabel_->setText(QStringLiteral("РђРєРєР°СѓРЅС‚: %1 (%2)").arg(auth.displayName, auth.login));
    } else {
        accountLabel_->setText(QStringLiteral("РђРєРєР°СѓРЅС‚: С‚СЂРµР±СѓРµС‚СЃСЏ РІС…РѕРґ"));
    }

    if (license.licenseActive) {
        licenseLabel_->setText(QStringLiteral("Р›РёС†РµРЅР·РёСЏ: Р°РєС‚РёРІРЅР° РґРѕ %1").arg(license.licenseExpiresAt));
    } else if (license.featureBlockedReason.contains(QStringLiteral("РёСЃС‚РµРє"), Qt::CaseInsensitive)) {
        licenseLabel_->setText(QStringLiteral("Р›РёС†РµРЅР·РёСЏ: РёСЃС‚РµРєР»Р° %1").arg(license.licenseExpiresAt));
    } else if (!license.featureBlockedReason.isEmpty()) {
        licenseLabel_->setText(QStringLiteral("Р›РёС†РµРЅР·РёСЏ: %1").arg(license.featureBlockedReason));
    } else {
        licenseLabel_->setText(QStringLiteral("Р›РёС†РµРЅР·РёСЏ: С‚СЂРµР±СѓРµС‚СЃСЏ Р°РєС‚РёРІР°С†РёСЏ"));
    }

    featureLabel_->setText(feature.functionalityEnabled
                               ? QStringLiteral("Р¤СѓРЅРєС†РёРё: РґРѕСЃС‚СѓРїРЅС‹")
                               : QStringLiteral("Р¤СѓРЅРєС†РёРё: Р·Р°Р±Р»РѕРєРёСЂРѕРІР°РЅС‹ (%1)").arg(feature.blockedReason));

    scanFileButton_->setEnabled(feature.functionalityEnabled);
    scanDirectoryButton_->setEnabled(feature.functionalityEnabled);
    scanFixedDrivesButton_->setEnabled(feature.functionalityEnabled);

    updateDatabaseState();

    if (!auth.authenticated) {
        showLoginFlow();
        return;
    }

    if (!license.licenseActive && license.activationRequired) {
        showActivationFlow();
    }
}

void MainWindow::updateDatabaseState()
{
    const auto database = rpcClient_.databaseInfo();

    if (database.loaded) {
        databaseLabel_->setText(QStringLiteral("РђРЅС‚РёРІРёСЂСѓСЃРЅС‹Рµ Р±Р°Р·С‹: Р·Р°РіСЂСѓР¶РµРЅС‹, РґР°С‚Р° РІС‹РїСѓСЃРєР°: %1, Р·Р°РїРёСЃРµР№: %2")
                                    .arg(database.releaseDate)
                                    .arg(database.recordCount));
    } else if (!database.lastError.isEmpty()) {
        databaseLabel_->setText(QStringLiteral("РђРЅС‚РёРІРёСЂСѓСЃРЅС‹Рµ Р±Р°Р·С‹: %1").arg(database.lastError));
    } else {
        databaseLabel_->setText(QStringLiteral("РђРЅС‚РёРІРёСЂСѓСЃРЅС‹Рµ Р±Р°Р·С‹: РЅРµ Р·Р°РіСЂСѓР¶РµРЅС‹"));
    }
}

void MainWindow::showLoginFlow()
{
    AuthDialog dialog(rpcClient_, this);
    if (dialog.exec() == QDialog::Accepted) {
        updateAccountState();
    }
}

void MainWindow::showActivationFlow()
{
    ActivationDialog dialog(rpcClient_, this);
    if (dialog.exec() == QDialog::Accepted) {
        updateAccountState();
    }
}

void MainWindow::logout()
{
    rpcClient_.logout();
    updateAccountState();
}

void MainWindow::scanFile()
{
    const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Р’С‹Р±РµСЂРёС‚Рµ С„Р°Р№Р» РґР»СЏ СЃРєР°РЅРёСЂРѕРІР°РЅРёСЏ"));
    if (path.isEmpty()) {
        return;
    }

    scanResultEdit_->setPlainText(QStringLiteral("РЎРєР°РЅРёСЂРѕРІР°РЅРёРµ С„Р°Р№Р»Р°...\n%1").arg(path));
    const ScanResult result = rpcClient_.scanFile(path);
    scanResultEdit_->setPlainText(formatScanResult(result));
    updateDatabaseState();
}

void MainWindow::scanDirectory()
{
    const QString path = QFileDialog::getExistingDirectory(this, QStringLiteral("Р’С‹Р±РµСЂРёС‚Рµ РїР°РїРєСѓ РґР»СЏ СЃРєР°РЅРёСЂРѕРІР°РЅРёСЏ"));
    if (path.isEmpty()) {
        return;
    }

    scanResultEdit_->setPlainText(QStringLiteral("РЎРєР°РЅРёСЂРѕРІР°РЅРёРµ РїР°РїРєРё...\n%1").arg(path));
    const ScanResult result = rpcClient_.scanDirectory(path);
    scanResultEdit_->setPlainText(formatScanResult(result));
    updateDatabaseState();
}

void MainWindow::scanFixedDrives()
{
    scanResultEdit_->setPlainText(QStringLiteral("Scanning all fixed drives..."));
    const ScanResult result = rpcClient_.scanFixedDrives();
    scanResultEdit_->setPlainText(formatScanResult(result));
    updateDatabaseState();
}

} // namespace antivirus::gui