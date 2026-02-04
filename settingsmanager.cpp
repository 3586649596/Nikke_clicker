// ============================================================
// 文件名：settingsmanager.cpp
// 功能：设置管理器 - 实现文件
// 说明：实现配置的保存和加载
// ============================================================

#include "settingsmanager.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

// ============================================================
// 静态成员初始化
// ============================================================
SettingsManager* SettingsManager::s_instance = nullptr;

// ============================================================
// 配置键名常量
// ============================================================
// 使用常量定义键名，避免拼写错误
namespace Keys {
    // 点击参数
    const QString ClickInterval = "ClickParams/interval";
    const QString PressDownDuration = "ClickParams/pressDownDuration";
    const QString RandomDelayRange = "ClickParams/randomDelay";
    const QString ClickMode = "ClickParams/mode";

    // 快捷键
    const QString ToggleHotkey = "Hotkeys/toggle";

    // 窗口
    const QString MainWindowX = "Window/mainX";
    const QString MainWindowY = "Window/mainY";
    const QString OverlayX = "Window/overlayX";
    const QString OverlayY = "Window/overlayY";
    const QString OverlayVisible = "Window/overlayVisible";
}

// ============================================================
// 默认值常量
// ============================================================
namespace Defaults {
    const int ClickInterval = 20;
    const int PressDownDuration = 200;
    const int RandomDelayRange = 5;
    const int ClickMode = 0;  // SendInput
    const int ToggleHotkey = 0x77;  // VK_F8
    const int WindowX = 100;
    const int WindowY = 100;
    const bool OverlayVisible = true;
}

// ============================================================
// 单例模式实现
// ============================================================
SettingsManager* SettingsManager::instance()
{
    if (s_instance == nullptr) {
        s_instance = new SettingsManager();
    }
    return s_instance;
}

// ============================================================
// 构造函数
// ============================================================
/**
 * @brief SettingsManager 构造函数
 *
 * 创建 QSettings 对象，使用 INI 文件格式存储配置。
 *
 * 学习要点 - QSettings 构造方式：
 * 1. QSettings(组织名, 应用名) - 使用默认位置
 * 2. QSettings(文件路径, 格式) - 使用指定文件
 *
 * 我们使用第二种方式，将配置存储在：
 * Windows: %APPDATA%/NikkeQt/config.ini
 * Linux: ~/.config/NikkeQt/config.ini
 */
SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
{
    // 获取配置文件路径
    // QStandardPaths::AppConfigLocation 返回应用配置目录
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    // 确保目录存在
    QDir dir(configPath);
    if (!dir.exists()) {
        dir.mkpath(configPath);  // 创建目录（包括父目录）
    }

    // 配置文件完整路径
    QString configFile = configPath + "/config.ini";

    qDebug() << "SettingsManager: 配置文件路径:" << configFile;

    // 创建 QSettings 对象
    // QSettings::IniFormat: 使用 INI 文件格式
    // 注意：Qt 6 默认使用 UTF-8 编码，不需要手动设置 setIniCodec
    m_settings = new QSettings(configFile, QSettings::IniFormat, this);

    qDebug() << "SettingsManager: 配置管理器已初始化";
}

// ============================================================
// 析构函数
// ============================================================
SettingsManager::~SettingsManager()
{
    // QSettings 会自动保存，但我们显式调用确保数据写入
    if (m_settings) {
        m_settings->sync();
    }
    qDebug() << "SettingsManager: 配置管理器已销毁";
}

// ============================================================
// 点击参数设置
// ============================================================

void SettingsManager::setClickInterval(int ms)
{
    m_settings->setValue(Keys::ClickInterval, ms);
}

int SettingsManager::clickInterval() const
{
    // value() 方法：获取值，第二个参数是默认值
    return m_settings->value(Keys::ClickInterval, Defaults::ClickInterval).toInt();
}

void SettingsManager::setPressDownDuration(int ms)
{
    m_settings->setValue(Keys::PressDownDuration, ms);
}

int SettingsManager::pressDownDuration() const
{
    return m_settings->value(Keys::PressDownDuration, Defaults::PressDownDuration).toInt();
}

void SettingsManager::setRandomDelayRange(int ms)
{
    m_settings->setValue(Keys::RandomDelayRange, ms);
}

int SettingsManager::randomDelayRange() const
{
    return m_settings->value(Keys::RandomDelayRange, Defaults::RandomDelayRange).toInt();
}

// ============================================================
// 快捷键设置
// ============================================================

void SettingsManager::setToggleHotkey(int vkCode)
{
    m_settings->setValue(Keys::ToggleHotkey, vkCode);
}

int SettingsManager::toggleHotkey() const
{
    return m_settings->value(Keys::ToggleHotkey, Defaults::ToggleHotkey).toInt();
}

// ============================================================
// 点击模式设置
// ============================================================

void SettingsManager::setClickMode(int mode)
{
    m_settings->setValue(Keys::ClickMode, mode);
}

int SettingsManager::clickMode() const
{
    return m_settings->value(Keys::ClickMode, Defaults::ClickMode).toInt();
}

// ============================================================
// 窗口位置设置
// ============================================================

void SettingsManager::setMainWindowPos(const QPoint& pos)
{
    m_settings->setValue(Keys::MainWindowX, pos.x());
    m_settings->setValue(Keys::MainWindowY, pos.y());
}

QPoint SettingsManager::mainWindowPos() const
{
    int x = m_settings->value(Keys::MainWindowX, Defaults::WindowX).toInt();
    int y = m_settings->value(Keys::MainWindowY, Defaults::WindowY).toInt();
    return QPoint(x, y);
}

void SettingsManager::setOverlayPos(const QPoint& pos)
{
    m_settings->setValue(Keys::OverlayX, pos.x());
    m_settings->setValue(Keys::OverlayY, pos.y());
}

QPoint SettingsManager::overlayPos() const
{
    int x = m_settings->value(Keys::OverlayX, Defaults::WindowX).toInt();
    int y = m_settings->value(Keys::OverlayY, Defaults::WindowY).toInt();
    return QPoint(x, y);
}

void SettingsManager::setOverlayVisible(bool visible)
{
    m_settings->setValue(Keys::OverlayVisible, visible);
}

bool SettingsManager::overlayVisible() const
{
    return m_settings->value(Keys::OverlayVisible, Defaults::OverlayVisible).toBool();
}

// ============================================================
// 通用方法
// ============================================================

void SettingsManager::save()
{
    // sync() 方法：立即将更改写入存储
    // 通常 QSettings 会自动同步，但手动调用可以确保立即写入
    m_settings->sync();
    qDebug() << "SettingsManager: 配置已保存";
}

void SettingsManager::reload()
{
    // sync() 也会重新读取外部更改
    m_settings->sync();
    qDebug() << "SettingsManager: 配置已重新加载";
}

void SettingsManager::resetToDefaults()
{
    // 清除所有设置
    m_settings->clear();

    // 设置默认值
    setClickInterval(Defaults::ClickInterval);
    setPressDownDuration(Defaults::PressDownDuration);
    setRandomDelayRange(Defaults::RandomDelayRange);
    setClickMode(Defaults::ClickMode);
    setToggleHotkey(Defaults::ToggleHotkey);
    setMainWindowPos(QPoint(Defaults::WindowX, Defaults::WindowY));
    setOverlayPos(QPoint(Defaults::WindowX, Defaults::WindowY));
    setOverlayVisible(Defaults::OverlayVisible);

    save();
    qDebug() << "SettingsManager: 已恢复默认设置";
}
