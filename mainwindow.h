// ============================================================
// 文件名：mainwindow.h
// 功能：主窗口 - 头文件
// 说明：程序的主界面，包含所有设置控件和状态显示
// ============================================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// ============================================================
// Qt 头文件
// ============================================================
#include <QMainWindow>      // Qt 主窗口基类
#include <QSlider>          // 滑块控件
#include <QSpinBox>         // 数字输入框
#include <QLabel>           // 标签控件
#include <QPushButton>      // 按钮控件
#include <QRadioButton>     // 单选按钮
#include <QCheckBox>        // 复选框
#include <QGroupBox>        // 分组框
#include <QLineEdit>        // 文本输入框
#include <QVBoxLayout>      // 垂直布局
#include <QHBoxLayout>      // 水平布局
#include <QProgressBar>     // 进度条

// ============================================================
// 项目头文件
// ============================================================
#include "mouseclicker.h"       // 鼠标点击器
#include "keyboardhook.h"       // 键盘钩子
#include "overlaywidget.h"      // 悬浮窗口
#include "settingsmanager.h"    // 设置管理器
#include "updatechecker.h"      // 更新检查器

// 前置声明（避免循环包含）
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief MainWindow - 主窗口类
 *
 * 程序的主界面，提供以下功能：
 * 1. 快捷键设置 - 设置开启/关闭的快捷键
 * 2. 射击参数 - 调节点击间隔、按下时长、随机延迟
 * 3. 点击模式 - 选择 SendInput 或 PostMessage 模式
 * 4. 状态显示 - 显示当前运行状态
 *
 * 学习要点：
 * 1. QMainWindow 的使用
 * 2. Qt 布局管理（QVBoxLayout, QHBoxLayout）
 * 3. Qt 控件（QSlider, QSpinBox, QPushButton 等）
 * 4. 信号槽连接
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MainWindow();

    /**
     * @brief 检查更新（供外部调用）
     */
    void checkForUpdates();

private slots:
    // ========================================================
    // 槽函数 - 响应用户操作
    // ========================================================

    /**
     * @brief 启动/停止按钮点击
     */
    void onStartStopClicked();

    /**
     * @brief 点击间隔滑块值改变
     * @param value 新的值
     */
    void onClickIntervalChanged(int value);

    /**
     * @brief 按下时长滑块值改变
     * @param value 新的值
     */
    void onPressDownDurationChanged(int value);

    /**
     * @brief 随机延迟滑块值改变
     * @param value 新的值
     */
    void onRandomDelayChanged(int value);

    /**
     * @brief 点击模式改变
     */
    void onClickModeChanged();

    /**
     * @brief 修改快捷键按钮点击
     */
    void onChangeHotkeyClicked();

    /**
     * @brief 快捷键按下
     * @param vkCode 虚拟键码
     * @param name 快捷键名称
     */
    void onHotkeyPressed(int vkCode, const QString& name);

    /**
     * @brief 点击器状态改变
     * @param running 是否正在运行
     */
    void onClickerStatusChanged(bool running);

    /**
     * @brief 键盘钩子安装完成
     */
    void onHookInstalled();

    /**
     * @brief 键盘钩子安装失败
     * @param error 错误信息
     */
    void onHookFailed(const QString& error);

    /**
     * @brief 显示悬浮窗复选框状态改变
     * @param checked 是否选中
     */
    void onShowOverlayChanged(bool checked);

    // ========================================================
    // 更新相关槽函数
    // ========================================================

    /**
     * @brief 检查更新按钮点击
     */
    void onCheckUpdateClicked();

    /**
     * @brief 发现新版本
     */
    void onUpdateAvailable(const QString& version, const QString& url, const QString& notes);

    /**
     * @brief 当前已是最新版本
     */
    void onNoUpdateAvailable();

    /**
     * @brief 检查更新失败
     */
    void onCheckUpdateFailed(const QString& error);

    /**
     * @brief 下载进度更新
     */
    void onDownloadProgress(int percent);

    /**
     * @brief 下载完成
     */
    void onDownloadFinished(const QString& filePath);

    /**
     * @brief 下载失败
     */
    void onDownloadFailed(const QString& error);

private:
    // ========================================================
    // 私有方法
    // ========================================================

    /**
     * @brief 创建 UI 控件
     * 使用代码方式创建界面，便于学习
     */
    void setupUi();

    /**
     * @brief 连接信号和槽
     */
    void connectSignals();

    /**
     * @brief 初始化键盘钩子
     */
    void initKeyboardHook();

    /**
     * @brief 更新状态显示
     */
    void updateStatusDisplay();

    /**
     * @brief 加载设置
     */
    void loadSettings();

    /**
     * @brief 保存设置
     */
    void saveSettings();

    // ========================================================
    // 私有成员变量 - 核心对象
    // ========================================================
    Ui::MainWindow *ui;             // UI 对象指针（由 Qt Designer 生成）

    MouseClicker *m_clicker;        // 鼠标点击器
    KeyboardHook *m_keyboardHook;   // 键盘钩子
    OverlayWidget *m_overlay;       // 悬浮窗口
    SettingsManager *m_settings;    // 设置管理器

    int m_toggleHotkey;             // 开关快捷键的虚拟键码（默认 F8）

    // ========================================================
    // 私有成员变量 - UI 控件
    // ========================================================
    // 快捷键设置
    QLineEdit *m_hotkeyEdit;        // 显示当前快捷键
    QPushButton *m_changeHotkeyBtn; // 修改快捷键按钮
    bool m_isCapturingHotkey;       // 是否正在捕获新快捷键

    // 参数设置
    QSlider *m_clickIntervalSlider;     // 点击间隔滑块
    QSpinBox *m_clickIntervalSpinBox;   // 点击间隔数字框
    QSlider *m_pressDownSlider;         // 按下时长滑块
    QSpinBox *m_pressDownSpinBox;       // 按下时长数字框
    QSlider *m_randomDelaySlider;       // 随机延迟滑块
    QSpinBox *m_randomDelaySpinBox;     // 随机延迟数字框

    // 点击模式
    QRadioButton *m_sendInputRadio;     // SendInput 模式单选
    QRadioButton *m_postMessageRadio;   // PostMessage 模式单选

    // 状态和控制
    QLabel *m_statusLabel;              // 状态显示标签
    QPushButton *m_startStopBtn;        // 启动/停止按钮
    QCheckBox *m_showOverlayCheckBox;   // 显示悬浮窗复选框

    // 更新相关
    UpdateChecker *m_updateChecker;     // 更新检查器
    QPushButton *m_checkUpdateBtn;      // 检查更新按钮
    QProgressBar *m_updateProgressBar;  // 更新下载进度条
    QLabel *m_updateStatusLabel;        // 更新状态标签
    QString m_pendingUpdatePath;        // 待安装的更新包路径
};

#endif // MAINWINDOW_H
