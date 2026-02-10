# Nikke 鼠标宏 / Nikke Mouse Macro

[中文](#nikke-鼠标宏) | [English](#nikke-mouse-macro)

---

## Nikke 鼠标宏

一款轻量级 Windows 鼠标自动点击工具，专为 Nikke 等游戏设计。

### 功能特性

- **自动点击** - 鼠标左键自动按下/抬起循环
- **全局快捷键** - 支持自定义快捷键，游戏内可用
- **参数调节** - 可调节点击频率、按下/抬起延迟
- **悬浮状态窗** - 透明悬浮窗实时显示运行状态
- **系统托盘** - 支持最小化到托盘后台运行
- **自动更新** - 内置版本检测，一键更新到最新版
- **配置保存** - 自动保存用户设置，下次启动恢复

### 下载安装

1. 前往 [Releases](https://github.com/3586649596/Nikke_clicker/releases) 下载最新版本
2. 解压到任意目录
3. 以**管理员身份**运行 `Nikke_Qt.exe`

### 使用说明

1. 启动程序（需管理员权限）
2. 设置点击参数和快捷键
3. 按 **F8**（默认）开启/关闭自动点击
4. 悬浮窗显示当前状态

### 推荐参数

| 参数 | 推荐值 | 说明 |
|-----|-------|------|
| 点击间隔 | 100 ms | 两次点击之间的间隔 |
| 按下时长 | 200 ms | 鼠标按下持续时间 |
| 随机延迟 | ±5 ms | 防止被检测的随机偏移 |

### 快捷键

| 快捷键 | 功能 |
|-------|------|
| F8 | 开启/关闭自动点击 |

*快捷键可在设置中自定义*

### 软件更新

程序内置自动更新功能：

- **自动检测** - 启动后自动检查 GitHub Releases 是否有新版本
- **手动检查** - 点击"检查更新"按钮手动检测
- **一键更新** - 发现新版本后可直接下载并自动安装

更新流程：检测新版本 → 下载压缩包 → 自动解压覆盖 → 重启程序

### 系统要求

- Windows 10/11 64-bit
- 需要管理员权限

### 注意事项

- 必须以**管理员身份**运行才能在游戏中生效
- 使用自动化工具可能违反游戏服务条款，请自行承担风险

### 反馈与支持

如有问题或建议，请提交 [Issue](https://github.com/3586649596/Nikke_clicker/issues)

---

## Nikke Mouse Macro

A lightweight Windows mouse auto-clicker designed for Nikke and similar games.

### Features

- **Auto Click** - Automatic mouse left-click (press/release cycle)
- **Global Hotkey** - Customizable hotkeys that work in-game
- **Adjustable Parameters** - Configure click interval, press duration, and delay
- **Overlay Window** - Transparent floating window shows real-time status
- **System Tray** - Minimize to tray for background operation
- **Auto Update** - Built-in version detection with one-click update
- **Save Settings** - Automatically saves and restores user preferences

### Installation

1. Download the latest version from [Releases](https://github.com/3586649596/Nikke_clicker/releases)
2. Extract to any directory
3. Run `Nikke_Qt.exe` **as Administrator**

### Usage

1. Launch the program (requires admin privileges)
2. Configure click parameters and hotkey
3. Press **F8** (default) to toggle auto-clicking
4. Overlay window shows current status

### Recommended Settings

| Parameter | Recommended | Description |
|-----------|-------------|-------------|
| Click Interval | 100 ms | Time between clicks |
| Press Duration | 200 ms | How long mouse button is held |
| Random Delay | ±5 ms | Random offset to avoid detection |

### Hotkeys

| Hotkey | Function |
|--------|----------|
| F8 | Toggle auto-click on/off |

*Hotkeys can be customized in settings*

### Auto Update

Built-in automatic update feature:

- **Auto Detection** - Checks GitHub Releases for new versions on startup
- **Manual Check** - Click "Check Update" button to check manually
- **One-Click Update** - Download and install updates automatically

Update flow: Detect new version → Download zip → Auto extract → Restart

### System Requirements

- Windows 10/11 64-bit
- Administrator privileges required

### Disclaimer

- Must run **as Administrator** to work in games
- Using automation tools may violate game terms of service, use at your own risk

### Feedback & Support

For issues or suggestions, please submit an [Issue](https://github.com/3586649596/Nikke_clicker/issues)

---

## Changelog

### v1.1.4
- Tuned preset parameter values for Stable / Balanced / Aggressive
- Synced preset recognition logic with updated parameter combinations

### v1.1.3
- Revamped main UI with single-screen HUD style
- Added preset controls and runtime summary
- Fixed startup crash caused by theme change recursion

### v1.1.2
- Improved UI responsive layout
- Fixed controls display issue when resizing window

### v1.1.1
- Update test version

### v1.1.0
- Added auto-update feature
- Added overlay visibility toggle
- Improved UI layout

### v1.0.0
- Initial release
- Basic auto-click functionality
- Global hotkey support
- Overlay status window
