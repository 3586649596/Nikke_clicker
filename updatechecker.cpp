// ============================================================
// 文件名：updatechecker.cpp
// 功能：自动更新检查器 - 实现文件
// ============================================================

#include "updatechecker.h"
#include "version.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QCoreApplication>
#include <QDebug>

// ============================================================
// 构造函数
// ============================================================
UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_downloadReply(nullptr)
    , m_downloadFile(nullptr)
{
    qDebug() << "UpdateChecker: 初始化完成，当前版本:" << APP_VERSION;
}

// ============================================================
// 析构函数
// ============================================================
UpdateChecker::~UpdateChecker()
{
    // 如果正在下载（用户关程序时可能还在下载中）
    if (m_downloadReply) {
        m_downloadReply->abort();          // 中断网络请求
        m_downloadReply->deleteLater();    // 稍后释放（QObject 子类用 deleteLater 更安全，
                                           // 防止事件队列里还有它的信号没处理完）
    }

    // 如果下载文件还开着
    if (m_downloadFile) {
        m_downloadFile->close();           // 关闭文件（释放文件句柄）
        delete m_downloadFile;             // 释放内存（普通对象直接 delete）
    }
    // 注意：m_networkManager 不用手动释放，构造时传了 this 作为父对象，Qt 自动管理
}

// ============================================================
// 检查更新
// ============================================================
void UpdateChecker::checkForUpdates()
{
    qDebug() << "UpdateChecker: 开始检查更新...";
    qDebug() << "UpdateChecker: API URL:" << GITHUB_API_URL;

    QNetworkRequest request;
    request.setUrl(QUrl(GITHUB_API_URL));
    request.setRawHeader("Accept", "application/vnd.github.v3+json");
    request.setRawHeader("User-Agent", "Nikke-Qt-Updater");

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onCheckFinished(reply);
    });
}

// ============================================================
// 检查更新完成回调
// ============================================================
void UpdateChecker::onCheckFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString error = reply->errorString();
        qDebug() << "UpdateChecker: 检查更新失败 -" << error;
        emit checkFailed(error);
        return;
    }

    // 解析 JSON 响应
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        emit checkFailed("解析服务器响应失败");
        return;
    }

    QJsonObject obj = doc.object();

    // 获取版本号（去掉前缀 v）
    m_latestVersion = obj["tag_name"].toString();
    if (m_latestVersion.startsWith('v') || m_latestVersion.startsWith('V')) {
        m_latestVersion = m_latestVersion.mid(1);
    }

    // 获取更新说明
    m_releaseNotes = obj["body"].toString();

    // 获取下载链接
    QJsonArray assets = obj["assets"].toArray();
    if (!assets.isEmpty()) {
        QJsonObject asset = assets[0].toObject();
        m_downloadUrl = asset["browser_download_url"].toString();
    }

    qDebug() << "UpdateChecker: 最新版本:" << m_latestVersion;
    qDebug() << "UpdateChecker: 当前版本:" << APP_VERSION;
    qDebug() << "UpdateChecker: 下载地址:" << m_downloadUrl;

    // 比较版本号
    int cmp = compareVersions(m_latestVersion, APP_VERSION);

    if (cmp > 0) {
        qDebug() << "UpdateChecker: 发现新版本!";
        emit updateAvailable(m_latestVersion, m_downloadUrl, m_releaseNotes);
    } else {
        qDebug() << "UpdateChecker: 当前已是最新版本";
        emit noUpdateAvailable();
    }
}

// ============================================================
// 下载更新包
// ============================================================
void UpdateChecker::downloadUpdate(const QString& url)
{
    qDebug() << "UpdateChecker: 开始下载更新包...";
    qDebug() << "UpdateChecker: URL:" << url;

    // 创建下载目录
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString downloadDir = tempDir + "/NikkeQt_Update";
    QDir().mkpath(downloadDir);

    // 设置下载文件路径
    m_downloadPath = downloadDir + "/Nikke_MouseMacro_v" + m_latestVersion + ".zip";

    // 创建文件
    m_downloadFile = new QFile(m_downloadPath);
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
        emit downloadFailed("无法创建下载文件: " + m_downloadFile->errorString());
        delete m_downloadFile;
        m_downloadFile = nullptr;
        return;
    }

    // 发起下载请求
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", "Nikke-Qt-Updater");

    m_downloadReply = m_networkManager->get(request);

    connect(m_downloadReply, &QNetworkReply::downloadProgress,
            this, &UpdateChecker::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished,
            this, &UpdateChecker::onDownloadFinished);
    connect(m_downloadReply, &QNetworkReply::readyRead, this, [this]() {
        if (m_downloadFile && m_downloadReply) {
            m_downloadFile->write(m_downloadReply->readAll());
        }
    });
}

// ============================================================
// 下载进度回调
// ============================================================
void UpdateChecker::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int percent = static_cast<int>((bytesReceived * 100) / bytesTotal);
        emit downloadProgress(percent);
    }
}

// ============================================================
// 下载完成回调
// ============================================================
void UpdateChecker::onDownloadFinished()
{
    if (!m_downloadReply || !m_downloadFile) {
        return;
    }

    // 写入剩余数据
    m_downloadFile->write(m_downloadReply->readAll());
    m_downloadFile->close();

    if (m_downloadReply->error() != QNetworkReply::NoError) {
        QString error = m_downloadReply->errorString();
        qDebug() << "UpdateChecker: 下载失败 -" << error;
        emit downloadFailed(error);

        // 删除不完整的文件
        QFile::remove(m_downloadPath);
    } else {
        qDebug() << "UpdateChecker: 下载完成 -" << m_downloadPath;
        emit downloadFinished(m_downloadPath);
    }

    // 清理
    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;
    delete m_downloadFile;
    m_downloadFile = nullptr;
}

// ============================================================
// 应用更新
// ============================================================
void UpdateChecker::applyUpdate(const QString& zipPath)
{
    qDebug() << "UpdateChecker: 准备应用更新...";

    // 获取程序所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    QString updaterPath = appDir + "/updater.bat";

    // 检查更新脚本是否存在
    if (!QFile::exists(updaterPath)) {
        qDebug() << "UpdateChecker: 更新脚本不存在，尝试创建...";

        // 动态创建更新脚本
        QFile script(updaterPath);
        if (script.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&script);
            out << "@echo off\r\n";
            out << "chcp 65001 >nul\r\n";
            out << "echo 正在更新 Nikke Qt...\r\n";
            out << "echo 等待程序退出...\r\n";
            out << "timeout /t 2 /nobreak >nul\r\n";
            out << "echo 解压更新包...\r\n";
            out << "powershell -Command \"Expand-Archive -Path '%1' -DestinationPath '%2' -Force\"\r\n";
            out << "echo 更新完成，正在重启...\r\n";
            out << "start \"\" \"%3\"\r\n";
            out << "del \"%1\"\r\n";
            out << "exit\r\n";
            script.close();
        }
    }

    // 获取可执行文件路径
    QString exePath = QCoreApplication::applicationFilePath();

    // 启动更新脚本
    QStringList args;
    args << zipPath << appDir << exePath;

    qDebug() << "UpdateChecker: 启动更新脚本:" << updaterPath;
    qDebug() << "UpdateChecker: 参数:" << args;

    QProcess::startDetached(updaterPath, args);

    // 退出当前程序
    QCoreApplication::quit();
}

// ============================================================
// 比较版本号
// ============================================================
int UpdateChecker::compareVersions(const QString& v1, const QString& v2)
{
    QStringList parts1 = v1.split('.');
    QStringList parts2 = v2.split('.');

    int maxLen = qMax(parts1.size(), parts2.size());

    for (int i = 0; i < maxLen; ++i) {
        int num1 = (i < parts1.size()) ? parts1[i].toInt() : 0;
        int num2 = (i < parts2.size()) ? parts2[i].toInt() : 0;

        if (num1 > num2) return 1;
        if (num1 < num2) return -1;
    }

    return 0;
}
