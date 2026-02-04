// ============================================================
// 文件名：keyboardhook.h
// 功能：全局键盘钩子 - 头文件
// 说明：使用 Windows 底层键盘钩子监听全局快捷键，即使在游戏中也能响应
// ============================================================

#ifndef KEYBOARDHOOK_H
#define KEYBOARDHOOK_H

// ============================================================
// Qt 头文件
// ============================================================
#include <QObject>      // Qt 基础对象类
#include <QThread>      // 线程类
#include <QMap>         // 映射容器，类似于 std::map

// ============================================================
// Windows API 头文件
// ============================================================
#include <windows.h>    // Windows API

/**
 * @brief KeyboardHook - 全局键盘钩子类
 *
 * 这个类使用 Windows 的底层键盘钩子（Low-Level Keyboard Hook）来监听
 * 全局按键事件。即使其他程序（如游戏）处于前台，也能捕获到按键。
 *
 * 学习要点：
 * 1. Windows 钩子机制 - SetWindowsHookEx/UnhookWindowsHookEx
 * 2. 回调函数 - 钩子触发时调用的函数
 * 3. 消息循环 - 钩子需要在有消息循环的线程中运行
 * 4. 单例模式 - 确保全局只有一个钩子实例
 *
 * 为什么需要单例？
 * - Windows 钩子的回调函数必须是静态函数或全局函数
 * - 静态函数无法访问非静态成员变量
 * - 使用单例模式，可以在静态回调函数中访问实例的成员
 *
 * 使用示例：
 * @code
 * KeyboardHook *hook = KeyboardHook::instance();
 * hook->registerHotkey(VK_F8, "toggle");  // 注册 F8 键
 * connect(hook, &KeyboardHook::hotkeyPressed, this, &MyClass::onHotkey);
 * hook->start();  // 启动钩子
 * @endcode
 */
class KeyboardHook : public QThread
{
    Q_OBJECT

public:
    // ========================================================
    // 单例模式
    // ========================================================
    /**
     * @brief 获取单例实例
     * @return KeyboardHook 的唯一实例指针
     *
     * 单例模式确保整个程序中只有一个 KeyboardHook 实例。
     * 第一次调用时创建实例，之后返回同一个实例。
     */
    static KeyboardHook* instance();

    // ========================================================
    // 禁止拷贝和赋值（单例模式必须）
    // ========================================================
    KeyboardHook(const KeyboardHook&) = delete;             // 禁止拷贝构造
    KeyboardHook& operator=(const KeyboardHook&) = delete;  // 禁止赋值

    // ========================================================
    // 公共方法
    // ========================================================
    /**
     * @brief 注册快捷键
     * @param vkCode Windows 虚拟键码（如 VK_F8 = 0x77）
     * @param name 快捷键名称，用于识别
     *
     * 常用虚拟键码：
     * - VK_F1 ~ VK_F12: 0x70 ~ 0x7B
     * - VK_ESCAPE: 0x1B
     * - VK_SPACE: 0x20
     */
    void registerHotkey(int vkCode, const QString& name);

    /**
     * @brief 注销快捷键
     * @param vkCode 虚拟键码
     */
    void unregisterHotkey(int vkCode);

    /**
     * @brief 注销所有快捷键
     */
    void clearHotkeys();

    /**
     * @brief 检查钩子是否正在运行
     * @return true 表示钩子已安装并在运行
     */
    bool isHookInstalled() const;

    /**
     * @brief 设置捕获模式
     * @param capture true 表示捕获所有按键，false 表示只捕获注册的快捷键
     *
     * 在捕获模式下，任何按键都会触发 hotkeyPressed 信号，
     * 用于让用户设置新的快捷键。
     */
    void setCaptureMode(bool capture);

    /**
     * @brief 检查是否处于捕获模式
     * @return true 表示捕获模式
     */
    bool isCaptureMode() const;

    /**
     * @brief 获取虚拟键码对应的键名
     * @param vkCode 虚拟键码
     * @return 键名字符串（如 "F8"）
     */
    static QString keyCodeToString(int vkCode);

    /**
     * @brief 获取键名对应的虚拟键码
     * @param keyName 键名
     * @return 虚拟键码，找不到返回 0
     */
    static int stringToKeyCode(const QString& keyName);

public slots:
    /**
     * @brief 停止钩子
     * 发送退出消息，让消息循环结束
     */
    void stopHook();

signals:
    // ========================================================
    // 信号
    // ========================================================
    /**
     * @brief 快捷键按下信号
     * @param vkCode 按下的虚拟键码
     * @param name 快捷键名称
     *
     * 当注册的快捷键被按下时发出此信号。
     * 注意：此信号在钩子线程中发出，Qt 会自动处理跨线程传递。
     */
    void hotkeyPressed(int vkCode, const QString& name);

    /**
     * @brief 钩子安装完成信号
     */
    void hookInstalled();

    /**
     * @brief 钩子安装失败信号
     * @param error 错误信息
     */
    void hookFailed(const QString& error);

protected:
    // ========================================================
    // 线程执行函数
    // ========================================================
    /**
     * @brief 线程入口函数
     *
     * 在这个函数中：
     * 1. 安装键盘钩子
     * 2. 运行消息循环
     * 3. 卸载键盘钩子
     */
    void run() override;

private:
    // ========================================================
    // 私有构造函数（单例模式）
    // ========================================================
    explicit KeyboardHook(QObject *parent = nullptr);
    ~KeyboardHook();

    // ========================================================
    // 静态回调函数
    // ========================================================
    /**
     * @brief 底层键盘钩子回调函数
     * @param nCode 钩子代码，小于0时必须调用 CallNextHookEx
     * @param wParam 消息类型（WM_KEYDOWN, WM_KEYUP 等）
     * @param lParam 指向 KBDLLHOOKSTRUCT 结构体的指针
     * @return 返回非零值阻止消息传递，返回 CallNextHookEx 的结果继续传递
     *
     * 这是一个静态函数，因为 Windows API 要求回调函数是普通函数指针。
     * 通过单例模式的 s_instance 访问类的成员。
     */
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    // ========================================================
    // 私有成员变量
    // ========================================================
    static KeyboardHook* s_instance;    // 单例实例指针

    HHOOK m_hook;                       // 钩子句柄
    DWORD m_threadId;                   // 钩子线程ID，用于发送退出消息
    bool m_isRunning;                   // 运行状态

    QMap<int, QString> m_hotkeys;       // 注册的快捷键映射 (键码 -> 名称)
    bool m_captureMode;                 // 捕获模式：true 时捕获所有按键
};

// ============================================================
// 常用虚拟键码定义（方便使用）
// ============================================================
// 注意：这些在 windows.h 中已定义，这里只是注释说明
// VK_F1  = 0x70
// VK_F2  = 0x71
// ...
// VK_F8  = 0x77
// VK_F9  = 0x78
// ...
// VK_F12 = 0x7B

#endif // KEYBOARDHOOK_H
