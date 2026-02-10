// ============================================================
// 文件名：overlaywidget.cpp
// 功能：悬浮状态窗口 - 实现文件
// 说明：实现游戏内悬浮状态显示窗口
// ============================================================

#include "overlaywidget.h"
#include "theme_manager.h"
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QDebug>

// ============================================================
// 构造函数
// ============================================================
/**
 * @brief OverlayWidget 构造函数
 *
 * 创建一个小型的悬浮窗口，显示当前状态。
 * 窗口特点：无边框、半透明、置顶、可拖动。
 */
OverlayWidget::OverlayWidget(QWidget *parent)
    : QWidget(parent)
    , m_statusLabel(nullptr)
    , m_isRunning(false)
    , m_isDarkTheme(true)
    , m_isDragging(false)
{
    // 初始化窗口属性
    setupWindow();

    // 创建布局和控件
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);  // 设置边距

    m_statusLabel = new QLabel("已停止", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-size: 12px; font-weight: bold;");

    layout->addWidget(m_statusLabel);

    // 设置窗口大小
    setFixedSize(100, 30);

    // 初始位置：屏幕右上角
    move(100, 100);

    qDebug() << "OverlayWidget: 悬浮窗口已创建";
}

// ============================================================
// 析构函数
// ============================================================
OverlayWidget::~OverlayWidget()
{
    qDebug() << "OverlayWidget: 悬浮窗口已销毁";
}

// ============================================================
// 设置窗口属性
// ============================================================
/**
 * @brief 初始化窗口属性
 *
 * 设置窗口为无边框、置顶、透明背景。
 *
 * 学习要点 - 窗口标志位：
 * - Qt::FramelessWindowHint: 无边框窗口
 * - Qt::WindowStaysOnTopHint: 窗口置顶
 * - Qt::Tool: 工具窗口，不在任务栏显示
 * - Qt::X11BypassWindowManagerHint: 绕过窗口管理器（Linux）
 *
 * 学习要点 - 窗口属性：
 * - Qt::WA_TranslucentBackground: 透明背景
 * - Qt::WA_ShowWithoutActivating: 显示时不激活（不抢焦点）
 */
void OverlayWidget::setupWindow()
{
    // 设置窗口标志
    setWindowFlags(
        Qt::FramelessWindowHint |       // 无边框
        Qt::WindowStaysOnTopHint |      // 置顶
        Qt::Tool |                      // 工具窗口
        Qt::X11BypassWindowManagerHint  // 绕过窗口管理器
    );

    // 设置窗口属性
    setAttribute(Qt::WA_TranslucentBackground);     // 透明背景
    setAttribute(Qt::WA_ShowWithoutActivating);     // 显示时不抢焦点

    // 使用 Windows API 进一步确保置顶
    // 这在某些全屏游戏中是必需的
    forceTopmost();
}

// ============================================================
// 设置运行状态
// ============================================================
void OverlayWidget::setRunning(bool running)
{
    m_isRunning = running;
    updateDisplay();
}

void OverlayWidget::setThemeDark(bool darkTheme)
{
    m_isDarkTheme = darkTheme;
    updateDisplay();
}

// ============================================================
// 更新显示
// ============================================================
void OverlayWidget::updateDisplay()
{
    const QColor textColor = ThemeManager::overlayTextColor(m_isRunning, m_isDarkTheme);

    if (m_isRunning) {
        m_statusLabel->setText("运行中");
    } else {
        m_statusLabel->setText("已停止");
    }
    m_statusLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; font-weight: bold;")
            .arg(textColor.name())
    );

    // 触发重绘（更新背景颜色）
    update();
}

// ============================================================
// 强制置顶
// ============================================================
/**
 * @brief 使用 Windows API 强制置顶
 *
 * 在某些全屏游戏中，Qt 的置顶标志可能无效。
 * 使用 SetWindowPos 和 HWND_TOPMOST 可以强制置顶。
 *
 * SetWindowPos 参数说明：
 * - 参数1: 窗口句柄
 * - 参数2: HWND_TOPMOST 表示置顶
 * - 参数3-6: 位置和大小（0 表示不改变）
 * - 参数7: SWP_NOMOVE | SWP_NOSIZE 表示不改变位置和大小
 */
void OverlayWidget::forceTopmost()
{
    // 获取窗口句柄
    // winId() 返回 Qt 窗口对应的本地窗口句柄
    HWND hwnd = reinterpret_cast<HWND>(winId());

    if (hwnd) {
        // 使用 SetWindowPos 设置置顶
        SetWindowPos(
            hwnd,
            HWND_TOPMOST,       // 置顶标志
            0, 0, 0, 0,         // 位置和大小（不改变）
            SWP_NOMOVE |        // 不改变位置
            SWP_NOSIZE |        // 不改变大小
            SWP_NOACTIVATE      // 不激活窗口
        );

        qDebug() << "OverlayWidget: 已强制置顶";
    }
}

// ============================================================
// 鼠标按下事件
// ============================================================
/**
 * @brief 处理鼠标按下事件
 *
 * 记录鼠标按下时的位置，用于计算拖动距离。
 *
 * 学习要点：
 * - event->button() 获取按下的是哪个鼠标按钮
 * - event->globalPosition() 获取鼠标的全局坐标（Qt 6）
 * - pos() 获取窗口当前位置
 */
void OverlayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        // 记录鼠标相对于窗口的位置
        // Qt 6 使用 globalPosition()，Qt 5 使用 globalPos()
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

// ============================================================
// 鼠标移动事件
// ============================================================
/**
 * @brief 处理鼠标移动事件
 *
 * 根据鼠标移动距离移动窗口。
 *
 * 计算方法：
 * 新窗口位置 = 当前鼠标全局位置 - 记录的相对位置
 */
void OverlayWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        // 计算新位置并移动窗口
        QPoint newPos = event->globalPosition().toPoint() - m_dragPosition;
        move(newPos);
        event->accept();
    }
}

// ============================================================
// 绘制事件
// ============================================================
/**
 * @brief 自定义绘制窗口
 *
 * 绘制一个半透明的圆角矩形作为背景。
 *
 * 学习要点：
 * - QPainter: Qt 的绘图类
 * - setRenderHint(): 设置绘图质量（抗锯齿）
 * - setBrush(): 设置填充颜色
 * - setPen(): 设置边框
 * - drawRoundedRect(): 绘制圆角矩形
 */
void OverlayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)  // 避免未使用参数的警告

    QPainter painter(this);

    // 启用抗锯齿，使边缘更平滑
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置背景颜色（半透明黑色或绿色）
    const QColor bgColor = ThemeManager::overlayBackgroundColor(m_isRunning, m_isDarkTheme);

    // 设置画笔和画刷
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);  // 无边框

    // 绘制圆角矩形
    // 参数：矩形区域，x方向圆角半径，y方向圆角半径
    painter.drawRoundedRect(rect(), 5, 5);
}
