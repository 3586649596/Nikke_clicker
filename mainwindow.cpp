// ============================================================
// 文件名：mainwindow.cpp
// 功能：主窗口 - 实现文件
// 说明：实现主窗口的界面和逻辑
// ============================================================

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <QMessageBox>
#include <QDebug>

// ============================================================
// 构造函数
// ============================================================
/**
 * @brief MainWindow 构造函数
 *
 * 初始化流程：
 * 1. 创建 UI 控件
 * 2. 创建核心对象（MouseClicker, KeyboardHook）
 * 3. 连接信号和槽
 * 4. 初始化键盘钩子
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_clicker(nullptr)
    , m_keyboardHook(nullptr)
    , m_overlay(nullptr)
    , m_settings(nullptr)
    , m_toggleHotkey(VK_F8)          // 默认使用 F8 作为开关键
    , m_isCapturingHotkey(false)
    , m_updateChecker(nullptr)
    , m_checkUpdateBtn(nullptr)
    , m_updateProgressBar(nullptr)
    , m_updateStatusLabel(nullptr)
{
    ui->setupUi(this);  // 初始化 Qt Designer 生成的 UI

    // 设置窗口标题和大小（包含版本号）
    setWindowTitle(QString("Nikke 鼠标宏 v%1").arg(APP_VERSION));
    setMinimumSize(420, 680);  // 设置最小尺寸，确保内容完整显示
    resize(450, 720);          // 设置初始大小

    // 获取设置管理器单例
    m_settings = SettingsManager::instance();

    // 创建界面
    setupUi();

    // 创建鼠标点击器
    m_clicker = new MouseClicker(this);

    // 获取键盘钩子单例
    m_keyboardHook = KeyboardHook::instance();

    // 创建悬浮窗口
    m_overlay = new OverlayWidget();
    m_overlay->show();

    // 创建更新检查器
    m_updateChecker = new UpdateChecker(this);

    // 连接信号和槽
    connectSignals();

    // 加载设置
    loadSettings();

    // 初始化键盘钩子
    initKeyboardHook();

    qDebug() << "MainWindow: 初始化完成";
}

// ============================================================
// 析构函数
// ============================================================
MainWindow::~MainWindow()
{
    // 保存设置
    saveSettings();

    // 停止点击器
    if (m_clicker && m_clicker->isClicking()) {
        m_clicker->stopClicking();
        m_clicker->wait(1000);
    }

    // 停止键盘钩子
    if (m_keyboardHook && m_keyboardHook->isHookInstalled()) {
        m_keyboardHook->stopHook();
        m_keyboardHook->wait(1000);
    }

    // 关闭悬浮窗口
    if (m_overlay) {
        m_overlay->close();
        delete m_overlay;
        m_overlay = nullptr;
    }

    delete ui;
    qDebug() << "MainWindow: 已销毁";
}

// ============================================================
// 创建 UI
// ============================================================
/**
 * @brief 创建用户界面
 *
 * 使用代码方式创建 UI，而不是使用 .ui 文件。
 * 这样可以更清楚地了解 Qt 布局系统的工作方式。
 *
 * 学习要点：
 * 1. QVBoxLayout/QHBoxLayout - 垂直/水平布局
 * 2. QGroupBox - 分组框，用于组织相关控件
 * 3. addWidget() - 向布局中添加控件
 * 4. addLayout() - 向布局中添加子布局
 */
void MainWindow::setupUi()
{
    // 创建中央窗口部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局（垂直布局）
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);         // 控件间距（优化后减小）
    mainLayout->setContentsMargins(15, 15, 15, 15);  // 边距（优化后减小）

    // ========================================
    // 快捷键设置组
    // ========================================
    QGroupBox *hotkeyGroup = new QGroupBox("快捷键设置", this);
    QHBoxLayout *hotkeyLayout = new QHBoxLayout(hotkeyGroup);

    QLabel *hotkeyLabel = new QLabel("开启/关闭:", this);
    m_hotkeyEdit = new QLineEdit(this);
    m_hotkeyEdit->setReadOnly(true);    // 设为只读
    m_hotkeyEdit->setText("F8");        // 默认显示 F8
    m_hotkeyEdit->setFixedWidth(80);
    m_hotkeyEdit->setAlignment(Qt::AlignCenter);

    m_changeHotkeyBtn = new QPushButton("修改", this);
    m_changeHotkeyBtn->setFixedWidth(60);

    hotkeyLayout->addWidget(hotkeyLabel);
    hotkeyLayout->addWidget(m_hotkeyEdit);
    hotkeyLayout->addWidget(m_changeHotkeyBtn);
    hotkeyLayout->addStretch();  // 添加弹性空间，把控件推到左边

    mainLayout->addWidget(hotkeyGroup);

    // ========================================
    // 射击参数组
    // ========================================
    QGroupBox *paramGroup = new QGroupBox("射击参数", this);
    paramGroup->setMinimumSize(350, 150);  // 设置最小宽度和高度
    paramGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);  // 水平扩展，垂直固定
    QVBoxLayout *paramLayout = new QVBoxLayout(paramGroup);

    // --- 点击间隔 ---
    QHBoxLayout *intervalLayout = new QHBoxLayout();
    QLabel *intervalLabel = new QLabel("点击间隔:", this);
    intervalLabel->setMinimumWidth(75);
    intervalLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_clickIntervalSlider = new QSlider(Qt::Horizontal, this);
    m_clickIntervalSlider->setRange(10, 500);   // 范围 10-500 毫秒
    m_clickIntervalSlider->setValue(20);        // 默认 20 毫秒
    m_clickIntervalSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_clickIntervalSpinBox = new QSpinBox(this);
    m_clickIntervalSpinBox->setRange(10, 500);
    m_clickIntervalSpinBox->setValue(20);
    m_clickIntervalSpinBox->setSuffix(" ms");   // 添加单位后缀
    m_clickIntervalSpinBox->setFixedWidth(150);  // 固定宽度确保完整显示
    m_clickIntervalSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    intervalLayout->addWidget(intervalLabel);
    intervalLayout->addWidget(m_clickIntervalSlider);
    intervalLayout->addWidget(m_clickIntervalSpinBox);
    intervalLayout->setStretch(0, 0);  // 标签：不伸展
    intervalLayout->setStretch(1, 1);  // 滑块：伸展
    intervalLayout->setStretch(2, 0);  // SpinBox：不伸展
    paramLayout->addLayout(intervalLayout);

    // --- 按下时长 ---
    QHBoxLayout *pressDownLayout = new QHBoxLayout();
    QLabel *pressDownLabel = new QLabel("按下时长:", this);
    pressDownLabel->setMinimumWidth(75);
    pressDownLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_pressDownSlider = new QSlider(Qt::Horizontal, this);
    m_pressDownSlider->setRange(10, 1000);      // 范围 10-1000 毫秒
    m_pressDownSlider->setValue(200);           // 默认 200 毫秒
    m_pressDownSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_pressDownSpinBox = new QSpinBox(this);
    m_pressDownSpinBox->setRange(10, 1000);
    m_pressDownSpinBox->setValue(200);
    m_pressDownSpinBox->setSuffix(" ms");
    m_pressDownSpinBox->setFixedWidth(150);  // 固定宽度确保完整显示
    m_pressDownSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    pressDownLayout->addWidget(pressDownLabel);
    pressDownLayout->addWidget(m_pressDownSlider);
    pressDownLayout->addWidget(m_pressDownSpinBox);
    pressDownLayout->setStretch(0, 0);  // 标签：不伸展
    pressDownLayout->setStretch(1, 1);  // 滑块：伸展
    pressDownLayout->setStretch(2, 0);  // SpinBox：不伸展
    paramLayout->addLayout(pressDownLayout);

    // --- 随机延迟 ---
    QHBoxLayout *randomLayout = new QHBoxLayout();
    QLabel *randomLabel = new QLabel("随机延迟:", this);
    randomLabel->setMinimumWidth(75);
    randomLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_randomDelaySlider = new QSlider(Qt::Horizontal, this);
    m_randomDelaySlider->setRange(0, 50);       // 范围 0-50 毫秒
    m_randomDelaySlider->setValue(5);           // 默认 ±5 毫秒
    m_randomDelaySlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_randomDelaySpinBox = new QSpinBox(this);
    m_randomDelaySpinBox->setRange(0, 50);
    m_randomDelaySpinBox->setValue(5);
    m_randomDelaySpinBox->setPrefix("±");       // 添加前缀
    m_randomDelaySpinBox->setSuffix(" ms");
    m_randomDelaySpinBox->setFixedWidth(150);  // 固定宽度确保完整显示
    m_randomDelaySpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    randomLayout->addWidget(randomLabel);
    randomLayout->addWidget(m_randomDelaySlider);
    randomLayout->addWidget(m_randomDelaySpinBox);
    randomLayout->setStretch(0, 0);  // 标签：不伸展
    randomLayout->setStretch(1, 1);  // 滑块：伸展
    randomLayout->setStretch(2, 0);  // SpinBox：不伸展
    paramLayout->addLayout(randomLayout);

    mainLayout->addWidget(paramGroup);

    // ========================================
    // 点击模式组
    // ========================================
    QGroupBox *modeGroup = new QGroupBox("点击模式", this);
    modeGroup->setMinimumSize(350, 100);  // 设置最小宽度和高度
    modeGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);  // 水平扩展，垂直固定
    QHBoxLayout *modeLayout = new QHBoxLayout(modeGroup);

    m_sendInputRadio = new QRadioButton("SendInput (推荐)", this);
    m_postMessageRadio = new QRadioButton("PostMessage", this);
    m_sendInputRadio->setChecked(true);  // 默认选中 SendInput

    modeLayout->addWidget(m_sendInputRadio);
    modeLayout->addWidget(m_postMessageRadio);
    modeLayout->addStretch();

    mainLayout->addWidget(modeGroup);

    // ========================================
    // 状态显示组
    // ========================================
    QGroupBox *statusGroup = new QGroupBox("状态", this);
    statusGroup->setMinimumSize(350, 100);  // 设置最小宽度和高度
    statusGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);  // 水平扩展，垂直固定
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);

    m_statusLabel = new QLabel("当前状态: 已停止", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    statusLayout->addWidget(m_statusLabel);

    m_showOverlayCheckBox = new QCheckBox("显示悬浮窗", this);
    m_showOverlayCheckBox->setChecked(true);
    statusLayout->addWidget(m_showOverlayCheckBox);

    mainLayout->addWidget(statusGroup);

    // ========================================
    // 控制按钮
    // ========================================
    QHBoxLayout *btnLayout = new QHBoxLayout();

    m_startStopBtn = new QPushButton("启动", this);
    m_startStopBtn->setFixedHeight(40);
    m_startStopBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-size: 14px; }"
        "QPushButton:hover { background-color: #45a049; }"
    );

    btnLayout->addWidget(m_startStopBtn);

    mainLayout->addLayout(btnLayout);

    // ========================================
    // 更新组
    // ========================================
    QGroupBox *updateGroup = new QGroupBox("软件更新", this);
    updateGroup->setMinimumSize(350, 100);  // 设置最小宽度和高度
    updateGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);  // 水平扩展，垂直固定
    QVBoxLayout *updateLayout = new QVBoxLayout(updateGroup);

    // 更新状态标签
    m_updateStatusLabel = new QLabel("点击按钮检查更新", this);
    m_updateStatusLabel->setAlignment(Qt::AlignCenter);
    updateLayout->addWidget(m_updateStatusLabel);

    // 进度条（默认隐藏）
    m_updateProgressBar = new QProgressBar(this);
    m_updateProgressBar->setRange(0, 100);
    m_updateProgressBar->setValue(0);
    m_updateProgressBar->setVisible(false);
    updateLayout->addWidget(m_updateProgressBar);

    // 检查更新按钮
    m_checkUpdateBtn = new QPushButton("检查更新", this);
    m_checkUpdateBtn->setFixedHeight(30);
    updateLayout->addWidget(m_checkUpdateBtn);

    mainLayout->addWidget(updateGroup);

    // 添加弹性空间
    mainLayout->addStretch();
}

// ============================================================
// 连接信号和槽
// ============================================================
/**
 * @brief 连接所有信号和槽
 *
 * Qt 信号槽机制说明：
 * connect(发送者, &发送者类::信号, 接收者, &接收者类::槽)
 *
 * 新语法的优点：
 * 1. 编译时检查，避免拼写错误
 * 2. 支持 lambda 表达式
 * 3. 更好的类型安全
 */
void MainWindow::connectSignals()
{
    // ========================================
    // 滑块和数字框同步
    // ========================================
    // 点击间隔：滑块和数字框双向绑定
    connect(m_clickIntervalSlider, &QSlider::valueChanged,
            m_clickIntervalSpinBox, &QSpinBox::setValue);
    connect(m_clickIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            m_clickIntervalSlider, &QSlider::setValue);
    connect(m_clickIntervalSlider, &QSlider::valueChanged,
            this, &MainWindow::onClickIntervalChanged);

    // 按下时长
    connect(m_pressDownSlider, &QSlider::valueChanged,
            m_pressDownSpinBox, &QSpinBox::setValue);
    connect(m_pressDownSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            m_pressDownSlider, &QSlider::setValue);
    connect(m_pressDownSlider, &QSlider::valueChanged,
            this, &MainWindow::onPressDownDurationChanged);

    // 随机延迟
    connect(m_randomDelaySlider, &QSlider::valueChanged,
            m_randomDelaySpinBox, &QSpinBox::setValue);
    connect(m_randomDelaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            m_randomDelaySlider, &QSlider::setValue);
    connect(m_randomDelaySlider, &QSlider::valueChanged,
            this, &MainWindow::onRandomDelayChanged);

    // ========================================
    // 点击模式
    // ========================================
    connect(m_sendInputRadio, &QRadioButton::toggled,
            this, &MainWindow::onClickModeChanged);

    // ========================================
    // 按钮
    // ========================================
    connect(m_startStopBtn, &QPushButton::clicked,
            this, &MainWindow::onStartStopClicked);
    connect(m_changeHotkeyBtn, &QPushButton::clicked,
            this, &MainWindow::onChangeHotkeyClicked);

    // ========================================
    // 悬浮窗显示控制
    // ========================================
    connect(m_showOverlayCheckBox, &QCheckBox::toggled,
            this, &MainWindow::onShowOverlayChanged);

    // ========================================
    // 鼠标点击器信号
    // ========================================
    connect(m_clicker, &MouseClicker::statusChanged,
            this, &MainWindow::onClickerStatusChanged);

    // ========================================
    // 键盘钩子信号
    // ========================================
    connect(m_keyboardHook, &KeyboardHook::hotkeyPressed,
            this, &MainWindow::onHotkeyPressed);
    connect(m_keyboardHook, &KeyboardHook::hookInstalled,
            this, &MainWindow::onHookInstalled);
    connect(m_keyboardHook, &KeyboardHook::hookFailed,
            this, &MainWindow::onHookFailed);

    // ========================================
    // 更新检查器信号
    // ========================================
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

// ============================================================
// 初始化键盘钩子
// ============================================================
void MainWindow::initKeyboardHook()
{
    // 注册默认快捷键 F8
    m_keyboardHook->registerHotkey(m_toggleHotkey, "toggle");

    // 启动键盘钩子线程
    m_keyboardHook->start();

    qDebug() << "MainWindow: 键盘钩子初始化中...";
}

// ============================================================
// 槽函数实现
// ============================================================

void MainWindow::onStartStopClicked()
{
    m_clicker->toggle();
}

void MainWindow::onClickIntervalChanged(int value)
{
    m_clicker->setClickInterval(value);
    qDebug() << "MainWindow: 点击间隔改为" << value << "ms";
}

void MainWindow::onPressDownDurationChanged(int value)
{
    m_clicker->setPressDownDuration(value);
    qDebug() << "MainWindow: 按下时长改为" << value << "ms";
}

void MainWindow::onRandomDelayChanged(int value)
{
    m_clicker->setRandomDelayRange(value);
    qDebug() << "MainWindow: 随机延迟改为 ±" << value << "ms";
}

void MainWindow::onClickModeChanged()
{
    if (m_sendInputRadio->isChecked()) {
        m_clicker->setClickMode(MouseClicker::SendInputMode);
    } else {
        m_clicker->setClickMode(MouseClicker::PostMessageMode);
    }
}

void MainWindow::onChangeHotkeyClicked()
{
    if (!m_isCapturingHotkey) {
        // 开始捕获新快捷键
        m_isCapturingHotkey = true;
        m_changeHotkeyBtn->setText("取消");
        m_hotkeyEdit->setText("按下新按键...");
        m_hotkeyEdit->setStyleSheet("background-color: #FFFFCC;");  // 黄色背景提示

        // 启用键盘钩子的捕获模式，捕获所有按键
        m_keyboardHook->setCaptureMode(true);
    } else {
        // 取消捕获
        m_isCapturingHotkey = false;
        m_changeHotkeyBtn->setText("修改");
        m_hotkeyEdit->setText(KeyboardHook::keyCodeToString(m_toggleHotkey));
        m_hotkeyEdit->setStyleSheet("");

        // 关闭捕获模式
        m_keyboardHook->setCaptureMode(false);
    }
}

void MainWindow::onHotkeyPressed(int vkCode, const QString& name)
{
    qDebug() << "MainWindow: 收到快捷键信号 -" << name << "键码:" << Qt::hex << vkCode;

    // 如果正在捕获新快捷键
    if (m_isCapturingHotkey) {
        // 关闭捕获模式
        m_keyboardHook->setCaptureMode(false);

        // 更新快捷键
        m_keyboardHook->unregisterHotkey(m_toggleHotkey);  // 注销旧快捷键
        m_toggleHotkey = vkCode;
        m_keyboardHook->registerHotkey(vkCode, "toggle");  // 注册新快捷键

        // 更新 UI
        m_hotkeyEdit->setText(KeyboardHook::keyCodeToString(vkCode));
        m_hotkeyEdit->setStyleSheet("");
        m_changeHotkeyBtn->setText("修改");
        m_isCapturingHotkey = false;

        qDebug() << "MainWindow: 快捷键已更新为" << KeyboardHook::keyCodeToString(vkCode);
        return;
    }

    // 正常快捷键处理：切换点击器状态
    if (name == "toggle") {
        m_clicker->toggle();
    }
}

void MainWindow::onClickerStatusChanged(bool running)
{
    updateStatusDisplay();

    // 更新悬浮窗口状态
    if (m_overlay) {
        m_overlay->setRunning(running);
    }

    if (running) {
        m_startStopBtn->setText("停止");
        m_startStopBtn->setStyleSheet(
            "QPushButton { background-color: #f44336; color: white; font-size: 14px; }"
            "QPushButton:hover { background-color: #da190b; }"
        );
    } else {
        m_startStopBtn->setText("启动");
        m_startStopBtn->setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; font-size: 14px; }"
            "QPushButton:hover { background-color: #45a049; }"
        );
    }
}

void MainWindow::onHookInstalled()
{
    qDebug() << "MainWindow: 键盘钩子已安装";
    m_statusLabel->setText("当前状态: 已停止 (快捷键就绪)");
}

void MainWindow::onHookFailed(const QString& error)
{
    qDebug() << "MainWindow: 键盘钩子安装失败 -" << error;
    QMessageBox::warning(this, "警告",
        "键盘钩子安装失败，快捷键功能不可用。\n"
        "请尝试以管理员身份运行程序。\n\n"
        "错误信息: " + error);
    m_statusLabel->setText("当前状态: 钩子失败");
}

void MainWindow::updateStatusDisplay()
{
    if (m_clicker->isClicking()) {
        m_statusLabel->setText("当前状态: 运行中");
        m_statusLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: green;");
    } else {
        m_statusLabel->setText("当前状态: 已停止");
        m_statusLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: gray;");
    }
}

void MainWindow::onShowOverlayChanged(bool checked)
{
    if (m_overlay) {
        m_overlay->setVisible(checked);
    }
}

// ============================================================
// 加载设置
// ============================================================
void MainWindow::loadSettings()
{
    qDebug() << "MainWindow: 加载设置...";

    // 加载点击参数
    int interval = m_settings->clickInterval();
    int pressDown = m_settings->pressDownDuration();
    int randomDelay = m_settings->randomDelayRange();
    int clickMode = m_settings->clickMode();

    // 更新 UI 控件
    m_clickIntervalSlider->setValue(interval);
    m_pressDownSlider->setValue(pressDown);
    m_randomDelaySlider->setValue(randomDelay);

    if (clickMode == 0) {
        m_sendInputRadio->setChecked(true);
    } else {
        m_postMessageRadio->setChecked(true);
    }

    // 更新点击器参数
    m_clicker->setClickInterval(interval);
    m_clicker->setPressDownDuration(pressDown);
    m_clicker->setRandomDelayRange(randomDelay);
    m_clicker->setClickMode(clickMode == 0 ? MouseClicker::SendInputMode : MouseClicker::PostMessageMode);

    // 加载快捷键
    m_toggleHotkey = m_settings->toggleHotkey();
    m_hotkeyEdit->setText(KeyboardHook::keyCodeToString(m_toggleHotkey));

    // 加载悬浮窗位置和可见性
    if (m_overlay) {
        m_overlay->move(m_settings->overlayPos());
        bool visible = m_settings->overlayVisible();
        m_overlay->setVisible(visible);
        m_showOverlayCheckBox->setChecked(visible);  // 同步复选框状态
    }

    qDebug() << "MainWindow: 设置加载完成";
}

// ============================================================
// 保存设置
// ============================================================
void MainWindow::saveSettings()
{
    qDebug() << "MainWindow: 保存设置...";

    // 保存点击参数
    m_settings->setClickInterval(m_clickIntervalSlider->value());
    m_settings->setPressDownDuration(m_pressDownSlider->value());
    m_settings->setRandomDelayRange(m_randomDelaySlider->value());
    m_settings->setClickMode(m_sendInputRadio->isChecked() ? 0 : 1);

    // 保存快捷键
    m_settings->setToggleHotkey(m_toggleHotkey);

    // 保存窗口位置
    m_settings->setMainWindowPos(pos());

    if (m_overlay) {
        m_settings->setOverlayPos(m_overlay->pos());
        m_settings->setOverlayVisible(m_overlay->isVisible());
    }

    // 立即写入文件
    m_settings->save();

    qDebug() << "MainWindow: 设置保存完成";
}

// ============================================================
// 检查更新（公共方法）
// ============================================================
void MainWindow::checkForUpdates()
{
    if (m_updateChecker) {
        m_updateChecker->checkForUpdates();
    }
}

// ============================================================
// 更新相关槽函数
// ============================================================

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

    // 询问用户是否下载
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "发现新版本",
        QString("发现新版本 v%1\n当前版本 v%2\n\n是否下载更新？")
            .arg(version)
            .arg(APP_VERSION),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        // 开始下载
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

    // 询问用户是否立即更新
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "下载完成",
        "更新包已下载完成。\n\n立即安装更新？\n（程序将自动重启）",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        // 应用更新
        m_updateChecker->applyUpdate(filePath);
    } else {
        // 断开按钮原有连接，改为应用更新
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
