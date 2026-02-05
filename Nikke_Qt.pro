# ============================================================
# Nikke Qt 鼠标宏 - 项目配置文件
# ============================================================

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# ============================================================
# 源文件
# ============================================================
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mouseclicker.cpp \
    keyboardhook.cpp \
    overlaywidget.cpp \
    settingsmanager.cpp \
    updatechecker.cpp

# ============================================================
# 头文件
# ============================================================
HEADERS += \
    mainwindow.h \
    mouseclicker.h \
    keyboardhook.h \
    overlaywidget.h \
    settingsmanager.h \
    updatechecker.h \
    version.h

# ============================================================
# UI 文件
# ============================================================
FORMS += \
    mainwindow.ui

# ============================================================
# Windows 特定配置
# ============================================================
win32 {
    # 链接 Windows API 库
    LIBS += -luser32 -lkernel32

    # 嵌入 manifest 文件，请求管理员权限
    # 注意：开发调试时注释掉这两行，发布时取消注释
    # CONFIG += embed_manifest_exe
    # QMAKE_MANIFEST = $$PWD/app.manifest

    # 应用程序图标（可选）
    # RC_ICONS = app.ico
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
