// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QUrl>
#include <QThread>
#include <QScopedPointer>
#include <QAtomicInt>
#include <QHash>
#include <QSet>
#include <QStack>

#include <functional>
#include <sys/stat.h>
#include <dirent.h>

namespace dfmbase {

class FileScannerPrivate;
class ScannerWorker;

/**
 * @brief 文件扫描器
 *
 * 提供异步的文件/目录统计功能，包括：
 * - 文件数量统计
 * - 总大小计算
 * - 进度通知
 *
 * 使用示例：
 * @code
 * auto scanner = new FileScanner(this);
 * connect(scanner, &FileScanner::progressChanged, this, [](const ScanResult& result){
 *     qDebug() << "Current size:" << result.totalSize;
 * });
 * connect(scanner, &FileScanner::finished, this, [](const ScanResult& result, bool success){
 *     if (success) {
 *         qDebug() << "Total files:" << result.fileCount;
 *     }
 * });
 * scanner->start({QUrl::fromLocalFile("/home/user")});
 * @endcode
 */
class FileScanner : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 扫描选项
     */
    enum class ScanOption {
        NoOption = 0x00,   ///< 无特殊选项
        SingleDepth = 0x01,   ///< 只统计顶层，不递归
        IncludeSource = 0x02,   ///< 包含源目录本身（默认不包含）
        CollectFiles = 0x04   ///< 收集所有文件URL列表（默认不收集）
    };
    Q_ENUM(ScanOption)
    Q_DECLARE_FLAGS(ScanOptions, ScanOption)

    /**
     * @brief 扫描结果
     */
    struct ScanResult
    {
        qint64 totalSize { 0 };   ///< 总大小（字节）
        qint64 progressSize { 0 };   ///< 进度大小（用于显示）
        int fileCount { 0 };   ///< 文件数量
        int directoryCount { 0 };   ///< 目录数量
        QList<QUrl> allFiles;   ///< 所有文件的URL列表（仅当 CollectFiles 选项启用时填充）

        bool isValid() const { return fileCount >= 0 && directoryCount >= 0; }
        void clear() { totalSize = progressSize = fileCount = directoryCount = 0; allFiles.clear(); }
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit FileScanner(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     *
     */
    ~FileScanner() override;

    /**
     * @brief 设置扫描选项
     * @param options 选项组合
     *
     * 必须在 start() 之前调用
     */
    void setOptions(ScanOptions options);

    /**
     * @brief 获取当前选项
     */
    ScanOptions options() const;

    /**
     * @brief 获取最新结果
     *
     * 线程安全，可以在任何时候调用
     */
    ScanResult result() const;

    /**
     * @brief 检查是否正在运行
     */
    bool isRunning() const;

    /**
     * @brief 同步扫描文件和目录
     *
     * 在当前线程中同步执行扫描，直到完成。
     * 注意：此方法会阻塞调用线程，不适合在主线程中扫描大量文件。
     *
     * @param urls 要扫描的 URL 列表
     * @param options 扫描选项
     * @return 扫描结果
     *
     * @code
     * // 示例：同步扫描目录
     * auto result = FileScanner::scanSync({QUrl::fromLocalFile("/home/user")});
     * qDebug() << "Total size:" << result.totalSize;
     * qDebug() << "File count:" << result.fileCount;
     * @endcode
     */
    static ScanResult scanSync(const QList<QUrl> &urls,
                               ScanOptions options = ScanOption::NoOption);

    /**
     * @brief 同步扫描文件和目录（支持进度回调）
     *
     * 在当前线程中同步执行扫描，支持通过回调函数中断扫描。
     *
     * @param urls 要扫描的 URL 列表
     * @param options 扫描选项
     * @param progressCallback 进度回调函数，返回 false 可中断扫描
     * @return 扫描结果
     *
     * @code
     * // 示例：可中断的同步扫描
     * QAtomicInt stopFlag(0);
     * auto result = FileScanner::scanSyncWithCallback(
     *     {QUrl::fromLocalFile("/home/user")},
     *     FileScanner::ScanOption::NoOption,
     *     [&stopFlag](const ScanResult &result) -> bool {
     *         return stopFlag.loadRelaxed() == 0;  // 返回 false 停止扫描
     *     });
     * @endcode
     */
    using ProgressCallback = std::function<bool(const ScanResult &)>;
    static ScanResult scanSyncWithCallback(const QList<QUrl> &urls,
                                           ScanOptions options,
                                           ProgressCallback progressCallback);

public Q_SLOTS:
    /**
     * @brief 开始扫描
     * @param urls 要扫描的URL列表
     *
     * 如果已经在运行，会先停止当前任务
     */
    void start(const QList<QUrl> &urls);

    /**
     * @brief 停止扫描
     *
     * 异步停止，会发送 finished() 信号
     */
    void stop();

Q_SIGNALS:
    /**
     * @brief 进度更新信号
     * @param result 当前统计结果
     *
     * 发送频率：每500ms或每处理1000个文件
     */
    void progressChanged(const ScanResult &result);

    /**
     * @brief 扫描完成信号
     * @param result 最终统计结果
     */
    void finished(const ScanResult &result);

private:
    QScopedPointer<FileScannerPrivate> d;
    Q_DISABLE_COPY(FileScanner)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FileScanner::ScanOptions)

/**
 * @brief 文件扫描工作对象
 *
 * 运行在工作线程中，执行实际的文件系统遍历
 */
class ScannerWorker : public QObject
{
    Q_OBJECT

public:
    explicit ScannerWorker(QObject *parent = nullptr);
    ~ScannerWorker() override;

    void setUrls(const QList<QUrl> &urls);
    void setOptions(FileScanner::ScanOptions options);

public Q_SLOTS:
    /**
     * @brief 开始扫描（在工作线程中执行）
     */
    void start();

    /**
     * @brief 停止扫描
     */
    void stop();

Q_SIGNALS:
    /**
     * @brief 结果就绪信号
     * @param result 扫描结果
     * @param isFinal 是否为最终结果
     */
    void resultReady(const FileScanner::ScanResult &result, bool isFinal);

    /**
     * @brief 工作完成信号
     */
    void finished();

private:
    /**
     * @brief 检查是否应该停止
     */
    bool shouldStop() const;

private:
    QList<QUrl> urls;
    FileScanner::ScanOptions options { FileScanner::ScanOption::NoOption };

    // 控制标志
    std::atomic<bool> stopped { false };
};

}   // namespace dfmbase

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::FileScanner::ScanResult)

#endif   // FILESCANNER_H
