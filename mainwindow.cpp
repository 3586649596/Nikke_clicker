#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "theme_manager.h"
#include "version.h"

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QIcon>
#include <QMessageBox>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_clicker(nullptr)
    , m_keyboardHook(nullptr)
    , m_overlay(nullptr)
    , m_settings(nullptr)
    , m_toggleHotkey(VK_F8)
    , m_isCapturingHotkey(false)
    , m_hookReady(false)
    , m_statusLabel(nullptr)
    , m_statusBadge(nullptr)
    , m_runtimeSummaryLabel(nullptr)
    , m_hotkeyEdit(nullptr)
    , m_changeHotkeyBtn(nullptr)
    , m_presetButtonGroup(nullptr)
    , m_presetStableBtn(nullptr)
    , m_presetBalancedBtn(nullptr)
    , m_presetAggressiveBtn(nullptr)
    , m_clickIntervalSlider(nullptr)
    , m_clickIntervalSpinBox(nullptr)
    , m_pressDownSlider(nullptr)
    , m_pressDownSpinBox(nullptr)
    , m_randomDelaySlider(nullptr)
    , m_randomDelaySpinBox(nullptr)
    , m_sendInputRadio(nullptr)
    , m_postMessageRadio(nullptr)
    , m_startStopBtn(nullptr)
    , m_showOverlayCheckBox(nullptr)
    , m_updateChecker(nullptr)
    , m_checkUpdateBtn(nullptr)
    , m_updateProgressBar(nullptr)
    , m_updateStatusLabel(nullptr)
    , m_statusFadeEffect(nullptr)
    , m_statusFadeAnimation(nullptr)
    , m_themeApplyInProgress(false)
{
    ui->setupUi(this);

    setWindowTitle(QString("Nikke 鼠标宏 v%1").arg(APP_VERSION));
    setWindowIcon(QIcon(":/icons/logo.svg"));
    setMinimumSize(460, 720);
    resize(520, 800);

    m_settings = SettingsManager::instance();

    setupUi();

    m_clicker = new MouseClicker(this);
    m_keyboardHook = KeyboardHook::instance();

    m_overlay = new OverlayWidget();
    m_overlay->setThemeDark(ThemeManager::isSystemDark());
    m_overlay->show();

    m_updateChecker = new UpdateChecker(this);

    connectSignals();
    loadSettings();
    initKeyboardHook();

    ThemeManager::apply(this);
    updateStatusDisplay();
    refreshRuntimeSummary();
}

MainWindow::~MainWindow()
{
    saveSettings();

    if (m_clicker && m_clicker->isClicking()) {
        m_clicker->stopClicking();
        m_clicker->wait(1000);
    }

    if (m_keyboardHook && m_keyboardHook->isHookInstalled()) {
        m_keyboardHook->stopHook();
        m_keyboardHook->wait(1000);
    }

    if (m_overlay) {
        m_overlay->close();
        delete m_overlay;
        m_overlay = nullptr;
    }

    delete ui;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange && !m_themeApplyInProgress) {
        m_themeApplyInProgress = true;
        ThemeManager::apply(this);
        if (m_overlay) {
            m_overlay->setThemeDark(ThemeManager::isSystemDark());
        }
        updateDynamicStateStyles();
        m_themeApplyInProgress = false;
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralSurface");
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    buildHeroSection(mainLayout);
    buildControlSection(mainLayout);
    buildUpdateSection(mainLayout);

    mainLayout->addStretch();

    ThemeManager::apply(this);
}

void MainWindow::buildHeroSection(QVBoxLayout *mainLayout)
{
    QFrame *heroCard = new QFrame(this);
    heroCard->setObjectName("heroCard");

    QVBoxLayout *heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(16, 16, 16, 16);
    heroLayout->setSpacing(12);

    QHBoxLayout *titleLayout = new QHBoxLayout();

    QLabel *logoLabel = new QLabel(this);
    logoLabel->setPixmap(QIcon(":/icons/logo.svg").pixmap(28, 28));

    QVBoxLayout *titleTextLayout = new QVBoxLayout();
    titleTextLayout->setSpacing(2);

    QLabel *heroTitle = new QLabel("Nikke 鼠标宏", this);
    heroTitle->setObjectName("heroTitle");

    QLabel *heroSubtitle = new QLabel("单屏 HUD 控制台", this);
    heroSubtitle->setObjectName("heroSubtitle");

    titleTextLayout->addWidget(heroTitle);
    titleTextLayout->addWidget(heroSubtitle);

    titleLayout->addWidget(logoLabel);
    titleLayout->addLayout(titleTextLayout);
    titleLayout->addStretch();

    heroLayout->addLayout(titleLayout);

    m_statusBadge = new QLabel("已停止", this);
    m_statusBadge->setObjectName("statusBadge");
    m_statusBadge->setAlignment(Qt::AlignCenter);

    m_statusFadeEffect = new QGraphicsOpacityEffect(this);
    m_statusFadeEffect->setOpacity(1.0);
    m_statusBadge->setGraphicsEffect(m_statusFadeEffect);

    m_statusFadeAnimation = new QPropertyAnimation(m_statusFadeEffect, "opacity", this);
    m_statusFadeAnimation->setDuration(180);

    heroLayout->addWidget(m_statusBadge, 0, Qt::AlignLeft);

    m_statusLabel = new QLabel("待命中（热键初始化中）", this);
    m_statusLabel->setObjectName("sectionHint");
    heroLayout->addWidget(m_statusLabel);

    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(10);

    m_startStopBtn = new QPushButton("启动", this);
    m_startStopBtn->setObjectName("primaryToggleButton");
    m_startStopBtn->setMinimumHeight(48);

    QFrame *hotkeyPanel = new QFrame(this);
    hotkeyPanel->setObjectName("panelCard");
    QHBoxLayout *hotkeyLayout = new QHBoxLayout(hotkeyPanel);
    hotkeyLayout->setContentsMargins(10, 8, 10, 8);
    hotkeyLayout->setSpacing(8);

    QLabel *hotkeyIcon = new QLabel(this);
    hotkeyIcon->setPixmap(QIcon(":/icons/hotkey.svg").pixmap(16, 16));

    QLabel *hotkeyTitle = new QLabel("热键", this);
    hotkeyTitle->setObjectName("bodyText");

    m_hotkeyEdit = new QLineEdit(this);
    m_hotkeyEdit->setObjectName("hotkeyValue");
    m_hotkeyEdit->setReadOnly(true);
    m_hotkeyEdit->setText("F8");
    m_hotkeyEdit->setAlignment(Qt::AlignCenter);
    m_hotkeyEdit->setFixedWidth(80);

    m_changeHotkeyBtn = new QPushButton("修改", this);
    m_changeHotkeyBtn->setObjectName("changeHotkeyButton");

    hotkeyLayout->addWidget(hotkeyIcon);
    hotkeyLayout->addWidget(hotkeyTitle);
    hotkeyLayout->addWidget(m_hotkeyEdit);
    hotkeyLayout->addWidget(m_changeHotkeyBtn);

    controlLayout->addWidget(m_startStopBtn, 1);
    controlLayout->addWidget(hotkeyPanel, 2);

    heroLayout->addLayout(controlLayout);

    mainLayout->addWidget(heroCard);
}

void MainWindow::buildControlSection(QVBoxLayout *mainLayout)
{
    QFrame *panelCard = new QFrame(this);
    panelCard->setObjectName("panelCard");

    QVBoxLayout *panelLayout = new QVBoxLayout(panelCard);
    panelLayout->setContentsMargins(16, 16, 16, 16);
    panelLayout->setSpacing(12);

    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *paramIcon = new QLabel(this);
    paramIcon->setPixmap(QIcon(":/icons/params.svg").pixmap(18, 18));

    QLabel *title = new QLabel("参数与行为", this);
    title->setObjectName("sectionTitle");

    QLabel *hint = new QLabel("调参后实时生效", this);
    hint->setObjectName("sectionHint");

    headerLayout->addWidget(paramIcon);
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(hint);
    panelLayout->addLayout(headerLayout);

    m_presetButtonGroup = new QButtonGroup(this);
    m_presetButtonGroup->setExclusive(true);

    QHBoxLayout *presetLayout = new QHBoxLayout();
    presetLayout->setSpacing(8);

    QLabel *presetLabel = new QLabel("预设", this);
    presetLabel->setObjectName("bodyText");

    m_presetStableBtn = new QPushButton("稳定", this);
    m_presetBalancedBtn = new QPushButton("均衡", this);
    m_presetAggressiveBtn = new QPushButton("激进", this);

    m_presetStableBtn->setObjectName("presetButton");
    m_presetBalancedBtn->setObjectName("presetButton");
    m_presetAggressiveBtn->setObjectName("presetButton");

    m_presetStableBtn->setCheckable(true);
    m_presetBalancedBtn->setCheckable(true);
    m_presetAggressiveBtn->setCheckable(true);

    m_presetButtonGroup->addButton(m_presetStableBtn, static_cast<int>(UiPreset::Stable));
    m_presetButtonGroup->addButton(m_presetBalancedBtn, static_cast<int>(UiPreset::Balanced));
    m_presetButtonGroup->addButton(m_presetAggressiveBtn, static_cast<int>(UiPreset::Aggressive));

    presetLayout->addWidget(presetLabel);
    presetLayout->addWidget(m_presetStableBtn);
    presetLayout->addWidget(m_presetBalancedBtn);
    presetLayout->addWidget(m_presetAggressiveBtn);
    presetLayout->addStretch();

    panelLayout->addLayout(presetLayout);

    auto addSliderRow = [this, panelLayout](const QString& labelText,
                                             int min,
                                             int max,
                                             int value,
                                             QSlider **slider,
                                             QSpinBox **spinBox,
                                             const QString& prefix,
                                             const QString& suffix) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(8);

        QLabel *label = new QLabel(labelText, this);
        label->setObjectName("bodyText");
        label->setMinimumWidth(80);

        *slider = new QSlider(Qt::Horizontal, this);
        (*slider)->setRange(min, max);
        (*slider)->setValue(value);

        *spinBox = new QSpinBox(this);
        (*spinBox)->setRange(min, max);
        (*spinBox)->setValue(value);
        (*spinBox)->setPrefix(prefix);
        (*spinBox)->setSuffix(suffix);
        (*spinBox)->setFixedWidth(120);

        rowLayout->addWidget(label);
        rowLayout->addWidget(*slider, 1);
        rowLayout->addWidget(*spinBox);

        panelLayout->addLayout(rowLayout);
    };

    addSliderRow("点击间隔", 10, 500, 20,
                 &m_clickIntervalSlider, &m_clickIntervalSpinBox,
                 "", " ms");

    addSliderRow("按下时长", 10, 1000, 200,
                 &m_pressDownSlider, &m_pressDownSpinBox,
                 "", " ms");

    addSliderRow("随机延迟", 0, 50, 5,
                 &m_randomDelaySlider, &m_randomDelaySpinBox,
                 "±", " ms");

    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->setSpacing(12);

    QLabel *modeIcon = new QLabel(this);
    modeIcon->setPixmap(QIcon(":/icons/mode.svg").pixmap(18, 18));

    m_sendInputRadio = new QRadioButton("SendInput（推荐）", this);
    m_postMessageRadio = new QRadioButton("PostMessage", this);
    m_sendInputRadio->setChecked(true);

    modeLayout->addWidget(modeIcon);
    modeLayout->addWidget(m_sendInputRadio);
    modeLayout->addWidget(m_postMessageRadio);
    modeLayout->addStretch();

    panelLayout->addLayout(modeLayout);

    m_showOverlayCheckBox = new QCheckBox("显示悬浮窗", this);
    m_showOverlayCheckBox->setChecked(true);
    panelLayout->addWidget(m_showOverlayCheckBox);

    m_runtimeSummaryLabel = new QLabel("", this);
    m_runtimeSummaryLabel->setObjectName("runtimeSummary");
    panelLayout->addWidget(m_runtimeSummaryLabel);

    mainLayout->addWidget(panelCard);
}

void MainWindow::buildUpdateSection(QVBoxLayout *mainLayout)
{
    QFrame *panelCard = new QFrame(this);
    panelCard->setObjectName("panelCard");

    QVBoxLayout *panelLayout = new QVBoxLayout(panelCard);
    panelLayout->setContentsMargins(16, 16, 16, 16);
    panelLayout->setSpacing(10);

    QHBoxLayout *titleLayout = new QHBoxLayout();

    QLabel *updateIcon = new QLabel(this);
    updateIcon->setPixmap(QIcon(":/icons/update.svg").pixmap(18, 18));

    QLabel *title = new QLabel("软件更新", this);
    title->setObjectName("sectionTitle");

    titleLayout->addWidget(updateIcon);
    titleLayout->addWidget(title);
    titleLayout->addStretch();

    panelLayout->addLayout(titleLayout);

    m_updateStatusLabel = new QLabel("点击按钮检查更新", this);
    m_updateStatusLabel->setObjectName("sectionHint");
    panelLayout->addWidget(m_updateStatusLabel);

    m_updateProgressBar = new QProgressBar(this);
    m_updateProgressBar->setRange(0, 100);
    m_updateProgressBar->setValue(0);
    m_updateProgressBar->setVisible(false);
    panelLayout->addWidget(m_updateProgressBar);

    m_checkUpdateBtn = new QPushButton("检查更新", this);
    m_checkUpdateBtn->setObjectName("secondaryButton");
    m_checkUpdateBtn->setMinimumHeight(34);
    panelLayout->addWidget(m_checkUpdateBtn, 0, Qt::AlignLeft);

    mainLayout->addWidget(panelCard);
}

void MainWindow::connectSignals()
{
    connect(m_clickIntervalSlider, &QSlider::valueChanged,
            m_clickIntervalSpinBox, &QSpinBox::setValue);
    connect(m_clickIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            m_clickIntervalSlider, &QSlider::setValue);
    connect(m_clickIntervalSlider, &QSlider::valueChanged,
            this, &MainWindow::onClickIntervalChanged);

    connect(m_pressDownSlider, &QSlider::valueChanged,
            m_pressDownSpinBox, &QSpinBox::setValue);
    connect(m_pressDownSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            m_pressDownSlider, &QSlider::setValue);
    connect(m_pressDownSlider, &QSlider::valueChanged,
            this, &MainWindow::onPressDownDurationChanged);

    connect(m_randomDelaySlider, &QSlider::valueChanged,
            m_randomDelaySpinBox, &QSpinBox::setValue);
    connect(m_randomDelaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            m_randomDelaySlider, &QSlider::setValue);
    connect(m_randomDelaySlider, &QSlider::valueChanged,
            this, &MainWindow::onRandomDelayChanged);

    connect(m_sendInputRadio, &QRadioButton::toggled,
            this, &MainWindow::onClickModeChanged);

    connect(m_startStopBtn, &QPushButton::clicked,
            this, &MainWindow::onStartStopClicked);
    connect(m_changeHotkeyBtn, &QPushButton::clicked,
            this, &MainWindow::onChangeHotkeyClicked);

    connect(m_presetButtonGroup, &QButtonGroup::idClicked, this, [this](int id) {
        applyPreset(static_cast<UiPreset>(id), true);
    });

    connect(m_showOverlayCheckBox, &QCheckBox::toggled,
            this, &MainWindow::onShowOverlayChanged);

    connect(m_clicker, &MouseClicker::statusChanged,
            this, &MainWindow::onClickerStatusChanged);

    connect(m_keyboardHook, &KeyboardHook::hotkeyPressed,
            this, &MainWindow::onHotkeyPressed);
    connect(m_keyboardHook, &KeyboardHook::hookInstalled,
            this, &MainWindow::onHookInstalled);
    connect(m_keyboardHook, &KeyboardHook::hookFailed,
            this, &MainWindow::onHookFailed);

    connect(m_checkUpdateBtn, &QPushButton::clicked,
            this, &MainWindow::onCheckUpdateClicked);
    connect(m_updateChecker, &UpdateChecker::updateAvailable,
            this, &MainWindow::onUpdateAvailable);
    connect(m_updateChecker, &UpdateChecker::noUpdateAvailable,
            this, &MainWindow::onNoUpdateAvailable);
    connect(m_updateChecker, &UpdateChecker::checkFailed,
            this, &MainWindow::onCheckUpdateFailed);
    connect(m_updateChecker, &UpdateChecker::downloadProgress,
            this, &MainWindow::onDownloadProgress);
    connect(m_updateChecker, &UpdateChecker::downloadFinished,
            this, &MainWindow::onDownloadFinished);
    connect(m_updateChecker, &UpdateChecker::downloadFailed,
            this, &MainWindow::onDownloadFailed);
}

void MainWindow::initKeyboardHook()
{
    m_keyboardHook->registerHotkey(m_toggleHotkey, "toggle");
    m_keyboardHook->start();
}

void MainWindow::onStartStopClicked()
{
    m_clicker->toggle();
}

void MainWindow::onClickIntervalChanged(int value)
{
    m_clicker->setClickInterval(value);
    syncPresetSelectionWithValues();
    refreshRuntimeSummary();
}

void MainWindow::onPressDownDurationChanged(int value)
{
    m_clicker->setPressDownDuration(value);
    syncPresetSelectionWithValues();
    refreshRuntimeSummary();
}

void MainWindow::onRandomDelayChanged(int value)
{
    m_clicker->setRandomDelayRange(value);
    syncPresetSelectionWithValues();
    refreshRuntimeSummary();
}

void MainWindow::onClickModeChanged()
{
    if (m_sendInputRadio->isChecked()) {
        m_clicker->setClickMode(MouseClicker::SendInputMode);
    } else {
        m_clicker->setClickMode(MouseClicker::PostMessageMode);
    }

    refreshRuntimeSummary();
}

void MainWindow::onChangeHotkeyClicked()
{
    if (!m_isCapturingHotkey) {
        m_isCapturingHotkey = true;
        m_changeHotkeyBtn->setText("取消");
        m_hotkeyEdit->setText("按下新按键...");
        m_hotkeyEdit->setProperty("capturing", true);
        repolish(m_hotkeyEdit);
        m_keyboardHook->setCaptureMode(true);
    } else {
        m_isCapturingHotkey = false;
        m_changeHotkeyBtn->setText("修改");
        m_hotkeyEdit->setText(KeyboardHook::keyCodeToString(m_toggleHotkey));
        m_hotkeyEdit->setProperty("capturing", false);
        repolish(m_hotkeyEdit);
        m_keyboardHook->setCaptureMode(false);
    }

    refreshRuntimeSummary();
}

void MainWindow::onHotkeyPressed(int vkCode, const QString& name)
{
    if (m_isCapturingHotkey) {
        m_keyboardHook->setCaptureMode(false);

        m_keyboardHook->unregisterHotkey(m_toggleHotkey);
        m_toggleHotkey = vkCode;
        m_keyboardHook->registerHotkey(vkCode, "toggle");

        m_hotkeyEdit->setText(KeyboardHook::keyCodeToString(vkCode));
        m_hotkeyEdit->setProperty("capturing", false);
        repolish(m_hotkeyEdit);

        m_changeHotkeyBtn->setText("修改");
        m_isCapturingHotkey = false;
        refreshRuntimeSummary();
        return;
    }

    if (name == "toggle") {
        m_clicker->toggle();
    }
}

void MainWindow::onClickerStatusChanged(bool running)
{
    if (m_overlay) {
        m_overlay->setRunning(running);
    }

    updateStatusDisplay();
    playStatusAnimation();
}

void MainWindow::onHookInstalled()
{
    m_hookReady = true;
    updateStatusDisplay();
}

void MainWindow::onHookFailed(const QString& error)
{
    m_hookReady = false;
    QMessageBox::warning(this, "警告",
        "键盘钩子安装失败，快捷键功能不可用。\n"
        "请尝试以管理员身份运行程序。\n\n"
        "错误信息: " + error);

    updateStatusDisplay();
}

void MainWindow::updateStatusDisplay()
{
    const bool running = m_clicker && m_clicker->isClicking();

    if (running) {
        m_statusBadge->setText("运行中");
        m_statusLabel->setText("自动点击已启动，按热键可立即停止");
    } else if (m_hookReady) {
        m_statusBadge->setText("已停止");
        m_statusLabel->setText("待命中（热键已就绪）");
    } else {
        m_statusBadge->setText("已停止");
        m_statusLabel->setText("待命中（热键初始化中）");
    }

    updateDynamicStateStyles();
}

void MainWindow::updateDynamicStateStyles()
{
    const bool running = m_clicker && m_clicker->isClicking();

    if (m_startStopBtn) {
        m_startStopBtn->setText(running ? "停止" : "启动");
        m_startStopBtn->setProperty("running", running);
        repolish(m_startStopBtn);
    }

    if (m_statusBadge) {
        m_statusBadge->setProperty("running", running);
        repolish(m_statusBadge);
    }
}

void MainWindow::playStatusAnimation()
{
    if (!m_statusFadeAnimation || !m_statusFadeEffect) {
        return;
    }

    m_statusFadeAnimation->stop();
    m_statusFadeAnimation->setStartValue(0.3);
    m_statusFadeAnimation->setEndValue(1.0);
    m_statusFadeAnimation->start();
}

void MainWindow::onShowOverlayChanged(bool checked)
{
    if (m_overlay) {
        m_overlay->setThemeDark(ThemeManager::isSystemDark());
        m_overlay->setVisible(checked);
    }
}

void MainWindow::loadSettings()
{
    const int interval = m_settings->clickInterval();
    const int pressDown = m_settings->pressDownDuration();
    const int randomDelay = m_settings->randomDelayRange();
    const int clickMode = m_settings->clickMode();

    m_clickIntervalSlider->setValue(interval);
    m_pressDownSlider->setValue(pressDown);
    m_randomDelaySlider->setValue(randomDelay);

    if (clickMode == 0) {
        m_sendInputRadio->setChecked(true);
    } else {
        m_postMessageRadio->setChecked(true);
    }

    m_clicker->setClickInterval(interval);
    m_clicker->setPressDownDuration(pressDown);
    m_clicker->setRandomDelayRange(randomDelay);
    m_clicker->setClickMode(clickMode == 0 ? MouseClicker::SendInputMode : MouseClicker::PostMessageMode);

    m_toggleHotkey = m_settings->toggleHotkey();
    m_hotkeyEdit->setText(KeyboardHook::keyCodeToString(m_toggleHotkey));
    m_hotkeyEdit->setProperty("capturing", false);

    move(m_settings->mainWindowPos());

    if (m_overlay) {
        m_overlay->move(m_settings->overlayPos());
        const bool visible = m_settings->overlayVisible();
        m_overlay->setVisible(visible);
        m_showOverlayCheckBox->setChecked(visible);
    }

    const int lastPreset = m_settings->lastPreset();
    QPushButton *savedPresetBtn = qobject_cast<QPushButton*>(m_presetButtonGroup->button(lastPreset));
    if (savedPresetBtn) {
        savedPresetBtn->setChecked(true);
    } else {
        syncPresetSelectionWithValues();
    }

    refreshRuntimeSummary();
    updateStatusDisplay();
}

void MainWindow::saveSettings()
{
    m_settings->setClickInterval(m_clickIntervalSlider->value());
    m_settings->setPressDownDuration(m_pressDownSlider->value());
    m_settings->setRandomDelayRange(m_randomDelaySlider->value());
    m_settings->setClickMode(m_sendInputRadio->isChecked() ? 0 : 1);

    m_settings->setToggleHotkey(m_toggleHotkey);
    m_settings->setMainWindowPos(pos());

    QAbstractButton *checkedPreset = m_presetButtonGroup->checkedButton();
    if (checkedPreset) {
        m_settings->setLastPreset(m_presetButtonGroup->id(checkedPreset));
    } else {
        m_settings->setLastPreset(-1);
    }

    if (m_overlay) {
        m_settings->setOverlayPos(m_overlay->pos());
        m_settings->setOverlayVisible(m_overlay->isVisible());
    }

    m_settings->save();
}

void MainWindow::checkForUpdates()
{
    if (m_updateChecker) {
        m_updateChecker->checkForUpdates();
    }
}

void MainWindow::onCheckUpdateClicked()
{
    m_checkUpdateBtn->setEnabled(false);
    m_checkUpdateBtn->setText("检查中...");
    m_updateStatusLabel->setText("正在检查更新...");
    m_updateProgressBar->setVisible(false);

    m_updateChecker->checkForUpdates();
}

void MainWindow::onUpdateAvailable(const QString& version, const QString& url, const QString& notes)
{
    Q_UNUSED(notes)

    m_checkUpdateBtn->setEnabled(true);
    m_checkUpdateBtn->setText("下载更新");
    m_updateStatusLabel->setText(QString("发现新版本: v%1").arg(version));

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "发现新版本",
        QString("发现新版本 v%1\n当前版本 v%2\n\n是否下载更新？")
            .arg(version)
            .arg(APP_VERSION),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_checkUpdateBtn->setEnabled(false);
        m_checkUpdateBtn->setText("下载中...");
        m_updateProgressBar->setVisible(true);
        m_updateProgressBar->setValue(0);
        m_updateChecker->downloadUpdate(url);
    }
}

void MainWindow::onNoUpdateAvailable()
{
    m_checkUpdateBtn->setEnabled(true);
    m_checkUpdateBtn->setText("检查更新");
    m_updateStatusLabel->setText("当前已是最新版本");

    QMessageBox::information(this, "检查更新",
        QString("当前已是最新版本 v%1").arg(APP_VERSION));
}

void MainWindow::onCheckUpdateFailed(const QString& error)
{
    m_checkUpdateBtn->setEnabled(true);
    m_checkUpdateBtn->setText("检查更新");
    m_updateStatusLabel->setText("检查更新失败");

    QMessageBox::warning(this, "检查更新失败",
        QString("无法检查更新：\n%1").arg(error));
}

void MainWindow::onDownloadProgress(int percent)
{
    m_updateProgressBar->setValue(percent);
    m_updateStatusLabel->setText(QString("下载中... %1%").arg(percent));
}

void MainWindow::onDownloadFinished(const QString& filePath)
{
    m_pendingUpdatePath = filePath;

    m_checkUpdateBtn->setEnabled(true);
    m_checkUpdateBtn->setText("立即更新");
    m_updateStatusLabel->setText("下载完成，点击更新");
    m_updateProgressBar->setVisible(false);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "下载完成",
        "更新包已下载完成。\n\n立即安装更新？\n（程序将自动重启）",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_updateChecker->applyUpdate(filePath);
    } else {
        disconnect(m_checkUpdateBtn, &QPushButton::clicked,
                   this, &MainWindow::onCheckUpdateClicked);
        connect(m_checkUpdateBtn, &QPushButton::clicked, this, [this]() {
            if (!m_pendingUpdatePath.isEmpty()) {
                m_updateChecker->applyUpdate(m_pendingUpdatePath);
            }
        });
    }
}

void MainWindow::onDownloadFailed(const QString& error)
{
    m_checkUpdateBtn->setEnabled(true);
    m_checkUpdateBtn->setText("检查更新");
    m_updateStatusLabel->setText("下载失败");
    m_updateProgressBar->setVisible(false);

    QMessageBox::warning(this, "下载失败",
        QString("更新包下载失败：\n%1").arg(error));
}

void MainWindow::refreshRuntimeSummary()
{
    const int interval = m_clickIntervalSlider->value();
    const int pressDown = m_pressDownSlider->value();
    const int randomDelay = m_randomDelaySlider->value();

    const double cycleMs = qMax(1, interval + pressDown);
    const double cps = 1000.0 / cycleMs;

    const QString mode = m_sendInputRadio->isChecked() ? "SendInput" : "PostMessage";

    QString presetName = "自定义";
    QAbstractButton *checkedPreset = m_presetButtonGroup->checkedButton();
    if (checkedPreset) {
        presetName = presetToLabel(static_cast<UiPreset>(m_presetButtonGroup->id(checkedPreset)));
    }

    const QString status = (m_clicker && m_clicker->isClicking()) ? "运行中" : "待命";
    const QString hotkey = KeyboardHook::keyCodeToString(m_toggleHotkey);

    m_runtimeSummaryLabel->setText(
        QString("%1  ·  预估 %2 次/秒  ·  模式 %3  ·  随机 ±%4ms  ·  预设 %5  ·  热键 %6")
            .arg(status)
            .arg(QString::number(cps, 'f', 1))
            .arg(mode)
            .arg(randomDelay)
            .arg(presetName)
            .arg(hotkey)
    );
}

void MainWindow::applyPreset(UiPreset preset, bool persistPreset)
{
    QAbstractButton *presetBtn = m_presetButtonGroup->button(static_cast<int>(preset));
    if (presetBtn) {
        presetBtn->setChecked(true);
    }

    switch (preset) {
    case UiPreset::Custom:
        break;
    case UiPreset::Stable:
        m_clickIntervalSlider->setValue(150);
        m_pressDownSlider->setValue(250);
        m_randomDelaySlider->setValue(8);
        break;
    case UiPreset::Balanced:
        m_clickIntervalSlider->setValue(130);
        m_pressDownSlider->setValue(230);
        m_randomDelaySlider->setValue(5);
        break;
    case UiPreset::Aggressive:
        m_clickIntervalSlider->setValue(100);
        m_pressDownSlider->setValue(200);
        m_randomDelaySlider->setValue(2);
        break;
    }

    if (persistPreset && m_settings) {
        m_settings->setLastPreset(static_cast<int>(preset));
    }

    refreshRuntimeSummary();
}

void MainWindow::syncPresetSelectionWithValues()
{
    const UiPreset preset = currentPresetFromValues();
    QAbstractButton *button = m_presetButtonGroup->button(static_cast<int>(preset));

    if (button) {
        button->setChecked(true);
        if (m_settings) {
            m_settings->setLastPreset(m_presetButtonGroup->id(button));
        }
    } else {
        m_presetButtonGroup->setExclusive(false);
        m_presetStableBtn->setChecked(false);
        m_presetBalancedBtn->setChecked(false);
        m_presetAggressiveBtn->setChecked(false);
        m_presetButtonGroup->setExclusive(true);
        if (m_settings) {
            m_settings->setLastPreset(-1);
        }
    }
}

MainWindow::UiPreset MainWindow::currentPresetFromValues() const
{
    const int interval = m_clickIntervalSlider->value();
    const int pressDown = m_pressDownSlider->value();
    const int randomDelay = m_randomDelaySlider->value();

    if (interval == 150 && pressDown == 250 && randomDelay == 8) {
        return UiPreset::Stable;
    }

    if (interval == 130 && pressDown == 230 && randomDelay == 5) {
        return UiPreset::Balanced;
    }

    if (interval == 100 && pressDown == 200 && randomDelay == 2) {
        return UiPreset::Aggressive;
    }

    return UiPreset::Custom;
}

QString MainWindow::presetToLabel(UiPreset preset) const
{
    switch (preset) {
    case UiPreset::Custom:
        return "自定义";
    case UiPreset::Stable:
        return "稳定";
    case UiPreset::Balanced:
        return "均衡";
    case UiPreset::Aggressive:
        return "激进";
    }

    return "自定义";
}

void MainWindow::repolish(QWidget *widget)
{
    if (!widget) {
        return;
    }

    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}
