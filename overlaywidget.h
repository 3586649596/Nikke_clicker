// ============================================================
// 文件名：overlaywidget.h
// 功能：悬浮状态窗口 - 头文件
// 说明：在游戏画面上方显示当前状态的小窗口
// ============================================================

#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

// ============================================================
// Qt 头文件
// ============================================================
#include <QWidget>      // Qt 基础窗口类
#include <QLabel>       // 标签控件
#include <QPoint>       // 点坐标

// ============================================================
// Windows API（用于强制置顶）
// ============================================================
#include <windows.h>

/**
 * @brief OverlayWidget - 悬浮状态窗口类
 *
 * 这是一个始终显示在最上层的小窗口，用于在游戏中显示当前状态。
 *
 * 学习要点：
 * 1. 无边框窗口 - Qt::FramelessWindowHint
 * 2. 窗口置顶 - Qt::WindowStaysOnTopHint
 * 3. 透明背景 - Qt::WA_TranslucentBackground
 * 4. 鼠标拖动 - 重写 mousePressEvent/mouseMoveEvent
 * 5. 工具窗口 - Qt::Tool（不在任务栏显示）
 *
 * 技术难点：
 * - 普通全屏游戏：Qt 的置顶标志有效
 * - 独占全屏游戏：需要使用 Windows API 强制置顶
 *
 * 使用示例：
 * @code
 * OverlayWidget *overlay = new OverlayWidget();
 * overlay->show();
 * overlay->setRunning(true);  // 显示运行状态
 * @endcode
 */
class OverlayWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口（通常为 nullptr，因为它是独立窗口）
     */
    explicit OverlayWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~OverlayWidget();

public slots:
    /**
     * @brief 设置运行状态
     * @param running true 表示运行中，false 表示已停止
     *
     * 根据状态更新显示的文字和颜色
     */
    void setRunning(bool running);

    /**
     * @brief 设置主题明暗
     * @param darkTheme true=深色主题 false=浅色主题
     */
    void setThemeDark(bool darkTheme);

    /**
     * @brief 强制置顶窗口
     *
     * 使用 Windows API 将窗口设为最顶层。
     * 在某些全屏游戏中，Qt 的置顶标志可能无效，需要使用此方法。
     */
    void forceTopmost();

protected:
    // ========================================================
    // 重写的事件处理函数
    // ========================================================

    /**
     * @brief 鼠标按下事件
     * @param event 鼠标事件对象
     *
     * 记录鼠标按下时的位置，用于实现窗口拖动
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件
     * @param event 鼠标事件对象
     *
     * 根据鼠标移动距离移动窗口
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief 绘制事件
     * @param event 绘制事件对象
     *
     * 自定义绘制窗口内容（圆角矩形背景）
     */
    void paintEvent(QPaintEvent *event) override;

private:
    // ========================================================
    // 私有方法
    // ========================================================

    /**
     * @brief 初始化窗口属性
     */
    void setupWindow();

    /**
     * @brief 更新显示内容
     */
    void updateDisplay();

    // ========================================================
    // 私有成员变量
    // ========================================================
    QLabel *m_statusLabel;      // 状态文字标签
    bool m_isRunning;           // 当前运行状态
    bool m_isDarkTheme;         // 当前主题模式

    QPoint m_dragPosition;      // 拖动时记录的鼠标位置
    bool m_isDragging;          // 是否正在拖动
};

#endif // OVERLAYWIDGET_H
