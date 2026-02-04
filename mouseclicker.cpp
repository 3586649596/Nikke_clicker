// ============================================================
// 文件名：mouseclicker.cpp
// 功能：鼠标自动点击器 - 实现文件
// 说明：实现 MouseClicker 类的所有方法
// ============================================================

#include "mouseclicker.h"
#include <QDebug>       // Qt 调试输出，类似于 cout

// ============================================================
// 构造函数
// ============================================================
/**
 * @brief MouseClicker 构造函数
 * @param parent 父对象指针
 *
 * 初始化所有成员变量为默认值。
 * 学习要点：C++11 的成员初始化列表语法，比在函数体内赋值更高效。
 */
MouseClicker::MouseClicker(QObject *parent)
    : QThread(parent)           // 调用父类构造函数
    , m_isRunning(false)        // 初始状态：未运行
    , m_clickInterval(20)       // 默认点击间隔：20毫秒
    , m_pressDownDuration(200)  // 默认按下时长：200毫秒
    , m_randomDelayRange(5)     // 默认随机延迟：±5毫秒
    , m_clickMode(SendInputMode) // 默认使用 SendInput 模式
{
    // 构造函数体为空，所有初始化都在初始化列表中完成
    qDebug() << "MouseClicker: 对象已创建";
}

// ============================================================
// 析构函数
// ============================================================
/**
 * @brief MouseClicker 析构函数
 *
 * 确保线程安全地停止。如果线程还在运行，等待它结束。
 * 学习要点：析构函数中要处理好资源清理，特别是线程。
 */
MouseClicker::~MouseClicker()
{
    // 如果线程还在运行，先停止它
    if (isRunning()) {
        stopClicking();     // 设置停止标志
        wait(1000);         // 等待线程结束，最多等待1秒
                            // wait() 是 QThread 的方法，阻塞直到线程结束
    }
    qDebug() << "MouseClicker: 对象已销毁";
}

// ============================================================
// 参数设置方法
// ============================================================

void MouseClicker::setClickInterval(int ms)
{
    // QMutexLocker 是 RAII 风格的锁管理
    // 构造时自动加锁，析构时自动解锁，即使发生异常也能正确解锁
    QMutexLocker locker(&m_mutex);
    m_clickInterval = ms;
    qDebug() << "MouseClicker: 点击间隔设置为" << ms << "毫秒";
}

void MouseClicker::setPressDownDuration(int ms)
{
    QMutexLocker locker(&m_mutex);
    m_pressDownDuration = ms;
    qDebug() << "MouseClicker: 按下时长设置为" << ms << "毫秒";
}

void MouseClicker::setRandomDelayRange(int ms)
{
    QMutexLocker locker(&m_mutex);
    m_randomDelayRange = ms;
    qDebug() << "MouseClicker: 随机延迟范围设置为 ±" << ms << "毫秒";
}

void MouseClicker::setClickMode(ClickMode mode)
{
    QMutexLocker locker(&m_mutex);
    m_clickMode = mode;
    qDebug() << "MouseClicker: 点击模式设置为"
             << (mode == SendInputMode ? "SendInput" : "PostMessage");
}

// ============================================================
// 状态查询方法
// ============================================================

bool MouseClicker::isClicking() const
{
    QMutexLocker locker(&m_mutex);  // const 方法中也需要加锁保证线程安全
    return m_isRunning;
}

int MouseClicker::clickInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_clickInterval;
}

int MouseClicker::pressDownDuration() const
{
    QMutexLocker locker(&m_mutex);
    return m_pressDownDuration;
}

int MouseClicker::randomDelayRange() const
{
    QMutexLocker locker(&m_mutex);
    return m_randomDelayRange;
}

MouseClicker::ClickMode MouseClicker::clickMode() const
{
    QMutexLocker locker(&m_mutex);
    return m_clickMode;
}

// ============================================================
// 控制方法
// ============================================================

void MouseClicker::startClicking()
{
    QMutexLocker locker(&m_mutex);

    // 如果已经在运行，不要重复启动
    if (m_isRunning) {
        qDebug() << "MouseClicker: 已经在运行中";
        return;
    }

    m_isRunning = true;
    locker.unlock();  // 在启动线程前解锁，避免死锁

    // 调用 QThread::start() 启动线程
    // 这会在新线程中执行 run() 函数
    start();

    // 发出状态变化信号，通知 UI 更新
    emit statusChanged(true);
    qDebug() << "MouseClicker: 开始点击";
}

void MouseClicker::stopClicking()
{
    QMutexLocker locker(&m_mutex);

    if (!m_isRunning) {
        qDebug() << "MouseClicker: 已经停止";
        return;
    }

    // 设置停止标志，run() 函数中的循环会检测这个标志并退出
    m_isRunning = false;
    locker.unlock();

    // 发出状态变化信号
    emit statusChanged(false);
    qDebug() << "MouseClicker: 停止点击";
}

void MouseClicker::toggle()
{
    if (isClicking()) {
        stopClicking();
    } else {
        startClicking();
    }
}

// ============================================================
// 线程执行函数
// ============================================================
/**
 * @brief 线程主循环
 *
 * 这个函数在独立线程中执行，实现点击循环：
 * 按下 -> 等待(按下时长 ± 随机) -> 抬起 -> 等待(间隔 ± 随机) -> 循环
 *
 * 学习要点：
 * 1. run() 函数执行完毕，线程就结束了
 * 2. 使用 m_isRunning 标志控制循环退出
 * 3. msleep() 是 QThread 的静态方法，让当前线程休眠
 */
void MouseClicker::run()
{
    qDebug() << "MouseClicker: 线程开始执行，线程ID:" << QThread::currentThreadId();

    // 主循环：当 m_isRunning 为 true 时持续执行
    while (true) {
        // 检查是否应该停止
        {
            QMutexLocker locker(&m_mutex);
            if (!m_isRunning) {
                break;  // 退出循环
            }
        }

        // 获取当前参数（需要加锁读取）
        int pressDownDuration, clickInterval;
        ClickMode mode;
        {
            QMutexLocker locker(&m_mutex);
            pressDownDuration = m_pressDownDuration;
            clickInterval = m_clickInterval;
            mode = m_clickMode;
        }

        // ========================================
        // 步骤1：发送鼠标按下事件
        // ========================================
        if (mode == SendInputMode) {
            sendMouseDown_SendInput();
        } else {
            sendMouseDown_PostMessage();
        }

        // ========================================
        // 步骤2：等待按下持续时间（+ 随机延迟）
        // ========================================
        int delay1 = pressDownDuration + generateRandomDelay();
        if (delay1 > 0) {
            msleep(delay1);  // QThread::msleep() 让线程休眠指定毫秒数
        }

        // ========================================
        // 步骤3：发送鼠标抬起事件
        // ========================================
        if (mode == SendInputMode) {
            sendMouseUp_SendInput();
        } else {
            sendMouseUp_PostMessage();
        }

        // 发出点击信号（用于调试或统计）
        emit clicked();

        // ========================================
        // 步骤4：等待点击间隔（+ 随机延迟）
        // ========================================
        int delay2 = clickInterval + generateRandomDelay();
        if (delay2 > 0) {
            msleep(delay2);
        }
    }

    qDebug() << "MouseClicker: 线程结束";
}

// ============================================================
// 私有方法 - SendInput 模式
// ============================================================

/**
 * @brief 使用 SendInput API 发送鼠标按下事件
 *
 * SendInput 是 Windows API，用于模拟键盘和鼠标输入。
 * 它比 mouse_event 更现代，是推荐的方式。
 *
 * 学习要点：
 * - INPUT 结构体用于描述输入事件
 * - MOUSEINPUT 是鼠标输入的具体参数
 * - MOUSEEVENTF_LEFTDOWN 表示左键按下
 */
void MouseClicker::sendMouseDown_SendInput()
{
    // INPUT 结构体：描述一个输入事件
    INPUT input;
    ZeroMemory(&input, sizeof(INPUT));  // 将结构体清零，这是 Windows 编程的好习惯

    input.type = INPUT_MOUSE;  // 指定这是鼠标输入
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;  // 鼠标左键按下

    // SendInput 函数：发送输入事件
    // 参数1：要发送的输入事件数量
    // 参数2：INPUT 结构体数组的指针
    // 参数3：INPUT 结构体的大小
    SendInput(1, &input, sizeof(INPUT));
}

/**
 * @brief 使用 SendInput API 发送鼠标抬起事件
 */
void MouseClicker::sendMouseUp_SendInput()
{
    INPUT input;
    ZeroMemory(&input, sizeof(INPUT));

    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;  // 鼠标左键抬起

    SendInput(1, &input, sizeof(INPUT));
}

// ============================================================
// 私有方法 - PostMessage 模式
// ============================================================

/**
 * @brief 使用 PostMessage API 发送鼠标按下消息
 *
 * PostMessage 直接向窗口发送消息，绕过输入队列。
 * 某些游戏可能需要这种方式。
 *
 * 学习要点：
 * - GetForegroundWindow() 获取当前活动窗口
 * - GetCursorPos() 获取鼠标当前位置
 * - ScreenToClient() 将屏幕坐标转换为窗口坐标
 * - MAKELPARAM() 宏用于组合两个 16 位值为一个 32 位值
 */
void MouseClicker::sendMouseDown_PostMessage()
{
    // 获取当前前台窗口句柄
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return;

    // 获取鼠标当前位置（屏幕坐标）
    POINT pt;
    GetCursorPos(&pt);

    // 将屏幕坐标转换为窗口客户区坐标
    ScreenToClient(hwnd, &pt);

    // 组合坐标为 lParam（低16位是x，高16位是y）
    LPARAM lParam = MAKELPARAM(pt.x, pt.y);

    // 发送鼠标左键按下消息
    // WM_LBUTTONDOWN: 鼠标左键按下消息
    // MK_LBUTTON: 表示左键被按下的标志
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
}

/**
 * @brief 使用 PostMessage API 发送鼠标抬起消息
 */
void MouseClicker::sendMouseUp_PostMessage()
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return;

    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);

    LPARAM lParam = MAKELPARAM(pt.x, pt.y);

    // WM_LBUTTONUP: 鼠标左键抬起消息
    PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
}

// ============================================================
// 私有方法 - 随机延迟生成
// ============================================================

/**
 * @brief 生成随机延迟值
 * @return 在 [-m_randomDelayRange, +m_randomDelayRange] 范围内的随机整数
 *
 * 随机延迟用于模拟人类操作的不规律性，降低被检测的风险。
 *
 * 学习要点：
 * - QRandomGenerator::global() 返回全局随机数生成器
 * - bounded(n) 返回 [0, n) 范围内的随机整数
 */
int MouseClicker::generateRandomDelay()
{
    QMutexLocker locker(&m_mutex);

    if (m_randomDelayRange <= 0) {
        return 0;
    }

    // 生成 [-range, +range] 范围内的随机数
    // 先生成 [0, 2*range] 范围的数，再减去 range
    int range = m_randomDelayRange;
    int randomValue = QRandomGenerator::global()->bounded(2 * range + 1) - range;

    return randomValue;
}
