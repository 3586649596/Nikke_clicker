@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

:: ============================================================
:: Nikke Qt 自动更新脚本
:: 参数1: ZIP文件路径
:: 参数2: 程序安装目录
:: 参数3: 可执行文件路径
:: ============================================================

echo ============================================
echo    Nikke Qt 自动更新程序
echo ============================================
echo.

set "ZIP_PATH=%~1"
set "APP_DIR=%~2"
set "EXE_PATH=%~3"

echo ZIP 文件: %ZIP_PATH%
echo 安装目录: %APP_DIR%
echo 可执行文件: %EXE_PATH%
echo.

:: 等待主程序退出
echo [1/4] 等待主程序退出...
timeout /t 3 /nobreak >nul

:: 检查主程序是否还在运行
:check_process
tasklist /FI "IMAGENAME eq Nikke_Qt.exe" 2>NUL | find /I /N "Nikke_Qt.exe">NUL
if "%ERRORLEVEL%"=="0" (
    echo 主程序仍在运行，等待中...
    timeout /t 1 /nobreak >nul
    goto check_process
)

echo 主程序已退出
echo.

:: 解压更新包
echo [2/4] 解压更新包...
powershell -Command "Expand-Archive -Path '%ZIP_PATH%' -DestinationPath '%APP_DIR%' -Force"

if %ERRORLEVEL% NEQ 0 (
    echo 错误: 解压失败!
    pause
    exit /b 1
)

echo 解压完成
echo.

:: 清理临时文件
echo [3/4] 清理临时文件...
del "%ZIP_PATH%" 2>nul
echo 清理完成
echo.

:: 重启程序
echo [4/4] 重启程序...
echo.
echo 更新完成! 正在启动 Nikke Qt...
timeout /t 1 /nobreak >nul

start "" "%EXE_PATH%"

exit
