// ============================================================
// 文件名：keyboardhook.cpp
// 功能：全局键盘钩子 - 实现文件
// 说明：实现 Windows 底层键盘钩子，监听全局快捷键
// ============================================================

#include "keyboardhook.h"
#include <QDebug>
#include <QCoreApplication>

// ============================================================
// 静态成员初始化
// ============================================================
// 单例模式的实例指针，初始为 nullptr
KeyboardHook* KeyboardHook::s_instance = nullptr;

// ============================================================
// 单例模式实现
// ============================================================
/**
 * @brief 获取单例实例
 *
 * 单例模式的关键：
 * 1. 构造函数私有，外部无法直接创建对象
 * 2. 通过静态方法获取唯一实例
 * 3. 第一次调用时创建实例，之后返回同一实例
 *
 * 注意：这个简单实现不是线程安全的，但在 Qt 程序中，
 * 通常在主线程初始化时调用，所以问题不大。
 */
KeyboardHook* KeyboardHook::instance()
{
    if (s_instance == nullptr) {
        s_instance = new KeyboardHook();
    }
    return s_instance;
}

// ============================================================
// 构造函数和析构函数
// ============================================================
KeyboardHook::KeyboardHook(QObject *parent)
    : QThread(parent)
    , m_hook(nullptr)       // 钩子句柄初始为空
    , m_threadId(0)         // 线程ID初始为0
    , m_isRunning(false)    // 初始未运行
    , m_captureMode(false)  // 初始不在捕获模式
{
    qDebug() << "KeyboardHook: 单例实例已创建";
}

KeyboardHook::~KeyboardHook()
{
    // 确保钩子已停止
    if (isRunning()) {
        stopHook();
        wait(1000);  // 等待线程结束
    }
    qDebug() << "KeyboardHook: 单例实例已销毁";
}

// ============================================================
// 快捷键注册
// ============================================================
void KeyboardHook::registerHotkey(int vkCode, const QString& name)
{
    m_hotkeys[vkCode] = name;
    qDebug() << "KeyboardHook: 注册快捷键" << name << "- 键码:" << Qt::hex << vkCode;
}

void KeyboardHook::unregisterHotkey(int vkCode)
{
    if (m_hotkeys.contains(vkCode)) {
        QString name = m_hotkeys[vkCode];
        m_hotkeys.remove(vkCode);
        qDebug() << "KeyboardHook: 注销快捷键" << name;
    }
}

void KeyboardHook::clearHotkeys()
{
    m_hotkeys.clear();
    qDebug() << "KeyboardHook: 清除所有快捷键";
}

bool KeyboardHook::isHookInstalled() const
{
    return m_hook != nullptr && m_isRunning;
}

void KeyboardHook::setCaptureMode(bool capture)
{
    m_captureMode = capture;
    qDebug() << "KeyboardHook: 捕获模式" << (capture ? "开启" : "关闭");
}

bool KeyboardHook::isCaptureMode() const
{
    return m_captureMode;
}

// ============================================================
// 键码转换工具函数
// ============================================================
QString KeyboardHook::keyCodeToString(int vkCode)
{
    // 常用功能键
    switch (vkCode) {
        case VK_F1:  return "F1";
        case VK_F2:  return "F2";
        case VK_F3:  return "F3";
        case VK_F4:  return "F4";
        case VK_F5:  return "F5";
        case VK_F6:  return "F6";
        case VK_F7:  return "F7";
        case VK_F8:  return "F8";
        case VK_F9:  return "F9";
        case VK_F10: return "F10";
        case VK_F11: return "F11";
        case VK_F12: return "F12";
        case VK_ESCAPE: return "Esc";
        case VK_TAB: return "Tab";
        case VK_CAPITAL: return "CapsLock";
        case VK_SPACE: return "Space";
        case VK_RETURN: return "Enter";
        case VK_BACK: return "Backspace";
        case VK_DELETE: return "Delete";
        case VK_INSERT: return "Insert";
        case VK_HOME: return "Home";
        case VK_END: return "End";
        case VK_PRIOR: return "PageUp";
        case VK_NEXT: return "PageDown";
        case VK_LEFT: return "Left";
        case VK_RIGHT: return "Right";
        case VK_UP: return "Up";
        case VK_DOWN: return "Down";
        case VK_NUMPAD0: return "Num0";
        case VK_NUMPAD1: return "Num1";
        case VK_NUMPAD2: return "Num2";
        case VK_NUMPAD3: return "Num3";
        case VK_NUMPAD4: return "Num4";
        case VK_NUMPAD5: return "Num5";
        case VK_NUMPAD6: return "Num6";
        case VK_NUMPAD7: return "Num7";
        case VK_NUMPAD8: return "Num8";
        case VK_NUMPAD9: return "Num9";
        default:
            // 字母键 A-Z (0x41-0x5A)
            if (vkCode >= 0x41 && vkCode <= 0x5A) {
                return QString(QChar(vkCode));
            }
            // 数字键 0-9 (0x30-0x39)
            if (vkCode >= 0x30 && vkCode <= 0x39) {
                return QString::number(vkCode - 0x30);
            }
            return QString("0x%1").arg(vkCode, 2, 16, QChar('0')).toUpper();
    }
}

int KeyboardHook::stringToKeyCode(const QString& keyName)
{
    QString name = keyName.toUpper().trimmed();

    // 功能键
    if (name == "F1") return VK_F1;
    if (name == "F2") return VK_F2;
    if (name == "F3") return VK_F3;
    if (name == "F4") return VK_F4;
    if (name == "F5") return VK_F5;
    if (name == "F6") return VK_F6;
    if (name == "F7") return VK_F7;
    if (name == "F8") return VK_F8;
    if (name == "F9") return VK_F9;
    if (name == "F10") return VK_F10;
    if (name == "F11") return VK_F11;
    if (name == "F12") return VK_F12;
    if (name == "ESC" || name == "ESCAPE") return VK_ESCAPE;
    if (name == "TAB") return VK_TAB;
    if (name == "CAPSLOCK") return VK_CAPITAL;
    if (name == "SPACE") return VK_SPACE;
    if (name == "ENTER" || name == "RETURN") return VK_RETURN;

    // 单个字母
    if (name.length() == 1) {
        QChar ch = name[0];
        if (ch >= 'A' && ch <= 'Z') {
            return ch.unicode();
        }
        if (ch >= '0' && ch <= '9') {
            return 0x30 + (ch.unicode() - '0');
        }
    }

    return 0;  // 未找到
}

// ============================================================
// 停止钩子
// ============================================================
void KeyboardHook::stopHook()
{
    if (m_threadId != 0 && m_isRunning) {
        qDebug() << "KeyboardHook: 发送退出消息到线程" << m_threadId;
        // 向钩子线程发送 WM_QUIT 消息，让消息循环退出
        PostThreadMessage(m_threadId, WM_QUIT, 0, 0);
        m_isRunning = false;
    }
}

// ============================================================
// 线程执行函数
// ============================================================
/**
 * @brief 钩子线程的主函数
 *
 * 这个函数做三件事：
 * 1. 安装键盘钩子
 * 2. 运行消息循环（钩子需要消息循环才能工作）
 * 3. 卸载键盘钩子
 *
 * 学习要点：
 * - 底层钩子必须在有消息循环的线程中运行
 * - GetMessage 会阻塞，直到收到消息
 * - 收到 WM_QUIT 消息时，GetMessage 返回 0，循环结束
 */
void KeyboardHook::run()
{
    qDebug() << "KeyboardHook: 线程开始，线程ID:" << QThread::currentThreadId();

    // 保存线程ID，用于之后发送退出消息
    m_threadId = GetCurrentThreadId();

    // ========================================
    // 步骤1：安装底层键盘钩子
    // ========================================
    /**
     * SetWindowsHookEx 函数说明：
     * - 参数1 (WH_KEYBOARD_LL): 钩子类型，WH_KEYBOARD_LL 是底层键盘钩子
     * - 参数2 (LowLevelKeyboardProc): 回调函数指针
     * - 参数3 (GetModuleHandle(nullptr)): 模块句柄，nullptr 表示当前模块
     * - 参数4 (0): 线程ID，0 表示监听所有线程（全局钩子）
     *
     * 返回值是钩子句柄，失败返回 nullptr
     */
    m_hook = SetWindowsHookEx(
        WH_KEYBOARD_LL,             // 底层键盘钩子
        LowLevelKeyboardProc,       // 回调函数
        GetModuleHandle(nullptr),   // 当前模块
        0                           // 全局钩子
    );

    // 检查钩子是否安装成功
    if (m_hook == nullptr) {
        DWORD error = GetLastError();
        QString errorMsg = QString("安装键盘钩子失败，错误码: %1").arg(error);
        qDebug() << "KeyboardHook:" << errorMsg;
        emit hookFailed(errorMsg);
        return;
    }

    qDebug() << "KeyboardHook: 键盘钩子安装成功";
    m_isRunning = true;
    emit hookInstalled();

    // ========================================
    // 步骤2：运行消息循环
    // ========================================
    /**
     * 消息循环说明：
     * - GetMessage: 从消息队列获取消息，阻塞直到有消息
     * - TranslateMessage: 转换键盘消息（虚拟键码到字符）
     * - DispatchMessage: 分发消息到窗口过程
     *
     * 当收到 WM_QUIT 消息时，GetMessage 返回 0，循环结束
     */
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // ========================================
    // 步骤3：卸载键盘钩子
    // ========================================
    if (m_hook != nullptr) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
        qDebug() << "KeyboardHook: 键盘钩子已卸载";
    }

    m_isRunning = false;
    m_threadId = 0;
    qDebug() << "KeyboardHook: 线程结束";
}

// ============================================================
// 钩子回调函数
// ============================================================
/**
 * @brief 底层键盘钩子回调函数
 *
 * 每当有键盘事件发生时，Windows 会调用这个函数。
 *
 * @param nCode 钩子代码
 *   - nCode < 0: 必须调用 CallNextHookEx 并返回其结果
 *   - nCode == HC_ACTION (0): 正常处理
 *
 * @param wParam 消息类型
 *   - WM_KEYDOWN (0x0100): 按键按下
 *   - WM_KEYUP (0x0101): 按键抬起
 *   - WM_SYSKEYDOWN (0x0104): 系统按键按下（如 Alt+x）
 *   - WM_SYSKEYUP (0x0105): 系统按键抬起
 *
 * @param lParam 指向 KBDLLHOOKSTRUCT 结构体的指针
 *   - vkCode: 虚拟键码
 *   - scanCode: 扫描码
 *   - flags: 标志位
 *   - time: 时间戳
 *
 * @return
 *   - 返回非零值: 阻止消息传递给下一个钩子和目标窗口
 *   - 返回 CallNextHookEx 的结果: 允许消息继续传递
 */
LRESULT CALLBACK KeyboardHook::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // 如果 nCode < 0，必须调用 CallNextHookEx
    if (nCode < 0) {
        return CallNextHookEx(s_instance->m_hook, nCode, wParam, lParam);
    }

    // 获取按键信息
    KBDLLHOOKSTRUCT* pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    int vkCode = pKeyboard->vkCode;

    // 只处理按键按下事件（避免重复触发）
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        // 捕获模式：捕获所有按键（用于设置新快捷键）
        if (s_instance != nullptr && s_instance->m_captureMode) {
            qDebug() << "KeyboardHook: 捕获模式 - 捕获到按键:" << Qt::hex << vkCode;
            emit s_instance->hotkeyPressed(vkCode, "capture");
            return 1;  // 阻止按键传递
        }

        // 正常模式：只检查注册的快捷键
        if (s_instance != nullptr && s_instance->m_hotkeys.contains(vkCode)) {
            QString name = s_instance->m_hotkeys[vkCode];
            qDebug() << "KeyboardHook: 检测到快捷键" << name
                     << "- 键码:" << Qt::hex << vkCode;

            // 发出信号（Qt 会自动处理跨线程传递）
            emit s_instance->hotkeyPressed(vkCode, name);

            // 返回 1 阻止按键传递到其他程序（如游戏）
            // 这样游戏就不会响应这个按键
            return 1;
        }
    }

    // 不是我们关心的按键，传递给下一个钩子
    return CallNextHookEx(s_instance->m_hook, nCode, wParam, lParam);
}
