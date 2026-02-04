// ============================================================
// 文件名：mouseclicker.h
// 功能：鼠标自动点击器 - 头文件
// 说明：这个类负责在独立线程中执行鼠标点击操作
// ============================================================

#ifndef MOUSECLICKER_H
#define MOUSECLICKER_H

// ============================================================
// Qt 头文件
// ============================================================
#include <QThread>      // QThread: Qt的线程类，用于多线程编程
#include <QMutex>       // QMutex: 互斥锁，用于线程同步
#include <QRandomGenerator>  // 随机数生成器，用于生成随机延迟

// ============================================================
// Windows API 头文件
// ============================================================
#include <windows.h>    // Windows API 基础头文件，包含 SendInput 等函数

/**
 * @brief MouseClicker - 鼠标自动点击器类
 *
 * 这个类继承自 QThread，在独立的线程中执行鼠标点击操作。
 * 这样做的好处是不会阻塞主界面（UI线程）。
 *
 * 学习要点：
 * 1. QThread 的使用方法 - 通过重写 run() 函数实现线程逻辑
 * 2. 线程安全 - 使用 QMutex 保护共享数据
 * 3. 信号槽跨线程通信 - Qt 自动处理线程间的信号传递
 * 4. Windows API 调用 - SendInput 发送模拟输入
 *
 * 使用示例：
 * @code
 * MouseClicker *clicker = new MouseClicker(this);
 * clicker->setClickInterval(20);     // 设置点击间隔 20ms
 * clicker->setPressDownDuration(200); // 设置按下时长 200ms
 * clicker->start();                   // 开始点击
 * // ...
 * clicker->stop();                    // 停止点击
 * @endcode
 */
class MouseClicker : public QThread
{
    Q_OBJECT  // Qt 元对象宏，启用信号槽机制（必须放在类声明的开头）

public:
    // ========================================================
    // 点击模式枚举
    // ========================================================
    /**
     * @brief ClickMode - 点击模式
     *
     * SendInput: 使用 Windows SendInput API，兼容性最好
     * PostMessage: 直接向窗口发送消息，某些游戏可能需要
     */
    enum ClickMode {
        SendInputMode,    // 使用 SendInput API（推荐）
        PostMessageMode   // 使用 PostMessage API
    };
    Q_ENUM(ClickMode)  // 注册枚举到 Qt 元对象系统，便于调试和序列化

    // ========================================================
    // 构造函数和析构函数
    // ========================================================
    /**
     * @brief 构造函数
     * @param parent 父对象指针（Qt 对象树机制，父对象销毁时自动销毁子对象）
     */
    explicit MouseClicker(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     * 确保线程安全地停止
     */
    ~MouseClicker();

    // ========================================================
    // 公共方法 - 参数设置
    // ========================================================
    /**
     * @brief 设置点击间隔（两次点击之间的等待时间）
     * @param ms 间隔时间，单位毫秒
     */
    void setClickInterval(int ms);

    /**
     * @brief 设置按下持续时间（鼠标按下到抬起的时间）
     * @param ms 持续时间，单位毫秒
     */
    void setPressDownDuration(int ms);

    /**
     * @brief 设置随机延迟范围（用于防检测）
     * @param ms 随机延迟的最大值，实际延迟在 ±ms 范围内
     */
    void setRandomDelayRange(int ms);

    /**
     * @brief 设置点击模式
     * @param mode 点击模式（SendInput 或 PostMessage）
     */
    void setClickMode(ClickMode mode);

    // ========================================================
    // 公共方法 - 状态查询
    // ========================================================
    /**
     * @brief 查询当前是否正在运行
     * @return true 表示正在点击，false 表示已停止
     */
    bool isClicking() const;

    /**
     * @brief 获取当前点击间隔
     * @return 点击间隔（毫秒）
     */
    int clickInterval() const;

    /**
     * @brief 获取当前按下持续时间
     * @return 按下持续时间（毫秒）
     */
    int pressDownDuration() const;

    /**
     * @brief 获取当前随机延迟范围
     * @return 随机延迟范围（毫秒）
     */
    int randomDelayRange() const;

    /**
     * @brief 获取当前点击模式
     * @return 点击模式
     */
    ClickMode clickMode() const;

public slots:
    // ========================================================
    // 公共槽函数 - 控制方法
    // ========================================================
    /**
     * @brief 开始点击
     * 启动线程，开始执行点击循环
     */
    void startClicking();

    /**
     * @brief 停止点击
     * 设置停止标志，线程会在下一个循环结束后退出
     */
    void stopClicking();

    /**
     * @brief 切换状态（开始/停止）
     * 如果正在运行则停止，否则开始
     */
    void toggle();

signals:
    // ========================================================
    // 信号 - 用于通知 UI 状态变化
    // ========================================================
    /**
     * @brief 状态变化信号
     * @param running 当前是否正在运行
     *
     * 当点击器启动或停止时发出此信号，UI 可以连接此信号来更新显示。
     * 学习要点：信号只需声明，不需要实现。Qt 的 moc 工具会自动生成实现。
     */
    void statusChanged(bool running);

    /**
     * @brief 点击执行信号（用于调试）
     * 每次执行点击时发出
     */
    void clicked();

protected:
    // ========================================================
    // 保护方法 - 线程执行函数
    // ========================================================
    /**
     * @brief 线程执行函数
     *
     * 重写 QThread::run()，这是线程的入口点。
     * 当调用 start() 时，Qt 会在新线程中执行此函数。
     *
     * 学习要点：
     * - run() 在新线程中执行，不要在这里直接操作 UI
     * - 通过信号槽与主线程通信
     */
    void run() override;

private:
    // ========================================================
    // 私有方法 - 鼠标操作
    // ========================================================
    /**
     * @brief 使用 SendInput 发送鼠标按下事件
     */
    void sendMouseDown_SendInput();

    /**
     * @brief 使用 SendInput 发送鼠标抬起事件
     */
    void sendMouseUp_SendInput();

    /**
     * @brief 使用 PostMessage 发送鼠标按下事件
     */
    void sendMouseDown_PostMessage();

    /**
     * @brief 使用 PostMessage 发送鼠标抬起事件
     */
    void sendMouseUp_PostMessage();

    /**
     * @brief 生成随机延迟
     * @return 随机延迟值（可能为负数）
     */
    int generateRandomDelay();

    // ========================================================
    // 私有成员变量
    // ========================================================
    bool m_isRunning;           // 运行状态标志
    int m_clickInterval;        // 点击间隔（毫秒）
    int m_pressDownDuration;    // 按下持续时间（毫秒）
    int m_randomDelayRange;     // 随机延迟范围（毫秒）
    ClickMode m_clickMode;      // 点击模式

    mutable QMutex m_mutex;     // 互斥锁，保护共享数据
                                // mutable 关键字允许在 const 方法中修改
};

#endif // MOUSECLICKER_H
