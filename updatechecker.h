// ============================================================
// 文件名：updatechecker.h
// 功能：自动更新检查器 - 头文件
// 说明：检查 GitHub Releases 获取最新版本，下载更新包
// ============================================================

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QString>

/**
 * @brief UpdateChecker - 自动更新检查器
 *
 * 功能：
 * 1. 调用 GitHub API 检查最新版本
 * 2. 对比版本号判断是否需要更新
 * 3. 后台下载更新包
 * 4. 触发更新脚本完成安装
 *
 * 使用示例：
 * @code
 * UpdateChecker *checker = new UpdateChecker(this);
 * connect(checker, &UpdateChecker::updateAvailable, this, &MainWindow::onUpdateAvailable);
 * checker->checkForUpdates();
 * @endcode
 */
class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent = nullptr);
    ~UpdateChecker();

    /**
     * @brief 检查更新
     * 调用 GitHub API 获取最新 release 信息
     */
    void checkForUpdates();

    /**
     * @brief 下载更新包
     * @param url 更新包下载地址
     */
    void downloadUpdate(const QString& url);

    /**
     * @brief 应用更新
     * 启动更新脚本并退出程序
     * @param zipPath 下载的 zip 文件路径
     */
    void applyUpdate(const QString& zipPath);

    /**
     * @brief 获取最新版本号
     */
    QString latestVersion() const { return m_latestVersion; }

    /**
     * @brief 获取更新说明
     */
    QString releaseNotes() const { return m_releaseNotes; }

    /**
     * @brief 比较版本号
     * @return >0 表示 v1 > v2, <0 表示 v1 < v2, 0 表示相等
     */
    static int compareVersions(const QString& v1, const QString& v2);

signals:
    /**
     * @brief 发现新版本
     * @param version 新版本号
     * @param downloadUrl 下载地址
     * @param releaseNotes 更新说明
     */
    void updateAvailable(const QString& version, const QString& downloadUrl, const QString& releaseNotes);

    /**
     * @brief 当前已是最新版本
     */
    void noUpdateAvailable();

    /**
     * @brief 检查更新失败
     * @param error 错误信息
     */
    void checkFailed(const QString& error);

    /**
     * @brief 下载进度
     * @param percent 进度百分比 (0-100)
     */
    void downloadProgress(int percent);

    /**
     * @brief 下载完成
     * @param filePath 下载文件的本地路径
     */
    void downloadFinished(const QString& filePath);

    /**
     * @brief 下载失败
     * @param error 错误信息
     */
    void downloadFailed(const QString& error);

private slots:
    void onCheckFinished(QNetworkReply* reply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_downloadReply;
    QFile *m_downloadFile;

    QString m_latestVersion;
    QString m_downloadUrl;
    QString m_releaseNotes;
    QString m_downloadPath;
};

#endif // UPDATECHECKER_H
