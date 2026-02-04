# Nikke Qt 鼠标宏

一个用于学习 Qt C++ 的鼠标自动点击工具。

## 功能

- 鼠标左键自动点击（按下-抬起循环）
- 可配置的快捷键（默认 F8 开启/关闭）
- 可调节的点击频率和延迟
- 游戏内悬浮状态窗口
- 最小化到系统托盘

## 编译环境

- Qt 6.x
- MinGW 64-bit 或 MSVC
- Windows 10/11

## 编译步骤

1. 用 Qt Creator 打开 `Nikke_Qt.pro`
2. 选择 Release 或 Debug 构建套件
3. 点击构建（Ctrl+B）
4. 运行（Ctrl+R）

## 使用方法

1. 以管理员身份运行程序
2. 设置快捷键和点击参数
3. 按 F8（或自定义快捷键）开启/关闭
4. 悬浮窗显示当前状态

## 快捷键

| 快捷键 | 功能 |
|-------|------|
| F8 | 开启/关闭自动点击 |
| F9 | 切换点击模式 |

## 注意事项

- 需要以管理员权限运行才能在游戏中使用
- 某些游戏可能会检测自动化工具，请谨慎使用

## 学习要点

这个项目涵盖了以下 Qt/C++ 知识：

- QThread 多线程编程
- Qt 信号槽机制
- Windows API（钩子、SendInput）
- Qt UI 设计
- QSettings 配置管理
- QSystemTrayIcon 系统托盘

## 项目结构

```
Nikke_Qt/
├── main.cpp              # 程序入口
├── mainwindow.h/cpp      # 主窗口
├── mouseclicker.h/cpp    # 鼠标点击核心
├── keyboardhook.h/cpp    # 键盘钩子
├── overlaywidget.h/cpp   # 悬浮窗口
├── settingsmanager.h/cpp # 设置管理
└── hotkeyedit.h/cpp      # 快捷键编辑控件
```
