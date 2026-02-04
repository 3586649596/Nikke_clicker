// ============================================================
// 文件名：settingsmanager.h
// 功能：设置管理器 - 头文件
// 说明：使用 QSettings 保存和加载程序配置
// ============================================================

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

// ============================================================
// Qt 头文件
// ============================================================
#include <QObject>
#include <QSettings>
#include <QString>
#include <QPoint>

/**
 * @brief SettingsManager - 设置管理器类
 *
 * 使用 QSettings 保存和加载程序的配置信息。
 * 采用单例模式，确保全局只有一个配置管理器。
 *
 * 学习要点：
 * 1. QSettings 的使用 - Qt 的配置存储类
 * 2. 单例模式 - 全局唯一实例
 * 3. INI 文件格式 - 跨平台的配置文件格式
 *
 * QSettings 存储位置：
 * - Windows: 注册表或 INI 文件
 * - Linux/macOS: ~/.config/ 目录下
 *
 * 我们使用 INI 文件格式，便于查看和手动编辑。
 * 文件位置：%APPDATA%/NikkeQt/config.ini
 *
 * 使用示例：
 * @code
 * SettingsManager *settings = SettingsManager::instance();
 * settings->setClickInterval(50);
 * int interval = settings->clickInterval();
 * @endcode
 */
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    // ========================================================
    // 单例模式
    // ========================================================
    /**
     * @brief 获取单例实例
     * @return SettingsManager 的唯一实例指针
     */
    static SettingsManager* instance();

    // 禁止拷贝和赋值
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    // ========================================================
    // 点击参数设置
    // ========================================================
    /**
     * @brief 设置点击间隔
     * @param ms 毫秒值
     */
    void setClickInterval(int ms);

    /**
     * @brief 获取点击间隔
     * @return 毫秒值，默认 20
     */
    int clickInterval() const;

    /**
     * @brief 设置按下时长
     * @param ms 毫秒值
     */
    void setPressDownDuration(int ms);

    /**
     * @brief 获取按下时长
     * @return 毫秒值，默认 200
     */
    int pressDownDuration() const;

    /**
     * @brief 设置随机延迟范围
     * @param ms 毫秒值
     */
    void setRandomDelayRange(int ms);

    /**
     * @brief 获取随机延迟范围
     * @return 毫秒值，默认 5
     */
    int randomDelayRange() const;

    // ========================================================
    // 快捷键设置
    // ========================================================
    /**
     * @brief 设置开关快捷键
     * @param vkCode 虚拟键码
     */
    void setToggleHotkey(int vkCode);

    /**
     * @brief 获取开关快捷键
     * @return 虚拟键码，默认 VK_F8 (0x77)
     */
    int toggleHotkey() const;

    // ========================================================
    // 点击模式设置
    // ========================================================
    /**
     * @brief 设置点击模式
     * @param mode 0=SendInput, 1=PostMessage
     */
    void setClickMode(int mode);

    /**
     * @brief 获取点击模式
     * @return 0=SendInput, 1=PostMessage，默认 0
     */
    int clickMode() const;

    // ========================================================
    // 窗口位置设置
    // ========================================================
    /**
     * @brief 设置主窗口位置
     * @param pos 窗口位置
     */
    void setMainWindowPos(const QPoint& pos);

    /**
     * @brief 获取主窗口位置
     * @return 窗口位置，默认 (100, 100)
     */
    QPoint mainWindowPos() const;

    /**
     * @brief 设置悬浮窗位置
     * @param pos 窗口位置
     */
    void setOverlayPos(const QPoint& pos);

    /**
     * @brief 获取悬浮窗位置
     * @return 窗口位置，默认 (100, 100)
     */
    QPoint overlayPos() const;

    /**
     * @brief 设置悬浮窗是否显示
     * @param visible 是否显示
     */
    void setOverlayVisible(bool visible);

    /**
     * @brief 获取悬浮窗是否显示
     * @return 是否显示，默认 true
     */
    bool overlayVisible() const;

    // ========================================================
    // 通用方法
    // ========================================================
    /**
     * @brief 保存所有设置到文件
     * 通常 QSettings 会自动保存，但可以手动调用确保立即写入
     */
    void save();

    /**
     * @brief 重新加载设置
     */
    void reload();

    /**
     * @brief 恢复默认设置
     */
    void resetToDefaults();

private:
    // ========================================================
    // 私有构造函数（单例模式）
    // ========================================================
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager();

    // ========================================================
    // 私有成员变量
    // ========================================================
    static SettingsManager* s_instance;     // 单例实例
    QSettings *m_settings;                  // QSettings 对象
};

#endif // SETTINGSMANAGER_H
