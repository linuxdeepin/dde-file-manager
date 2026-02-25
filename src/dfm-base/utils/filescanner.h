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
        IncludeSource = 0x02   ///< 包含源目录本身（默认不包含）
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

        bool isValid() const { return fileCount >= 0 && directoryCount >= 0; }
        void clear() { totalSize = progressSize = fileCount = directoryCount = 0; }
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
     * @brief 扫描本地文件路径
     *
     * 使用 opendir/readdir 进行遍历
     */
    void scanLocalPaths();

    /**
     * @brief 目录项结构（用于批量读取）
     */
    struct DirEntry
    {
        QByteArray name;   // 文件名（UTF-8）
        struct stat statBuf;   // lstat 结果
        bool statOk;   // lstat 是否成功
        unsigned char d_type;   // dirent.d_type (DT_DIR, DT_LNK 等)
    };

    /**
     * @brief 遍历上下文
     */
    struct ScanContext
    {
        QString fullPath;   // 当前完整路径
        int depth;   // 当前深度（0 = 源目录）
        bool isSourcePath;   // 是否为源路径
    };

    /**
     * @brief 读取目录条目
     * @param path 目录路径
     * @param entries 输出参数，目录条目列表
     * @return 是否成功
     */
    bool readDirectoryEntries(const QString &path, QList<DirEntry> *entries);

    /**
     * @brief 处理常规文件
     * @param path 文件路径
     * @param statBuf lstat 结果
     */
    void processRegularFile(const QString &path, const struct stat &statBuf);

    /**
     * @brief 处理符号链接
     * @param path 链接路径
     */
    void processSymlink(const QString &path);

    /**
     * @brief 判断路径是否为目录（跟随符号链接）
     * @param path 路径
     * @param lstatBuf lstat 结果
     * @return 是否为目录
     */
    bool isDirectoryPath(const QString &path, const struct stat &lstatBuf);

    /**
     * @brief 扫描其他协议（如 SMB, SFTP）
     *
     * 使用 InfoFactory 创建文件信息
     */
    void scanOtherProtocols();

    /**
     * @brief 检查是否应该停止
     */
    bool shouldStop() const;

    /**
     * @brief 检查 inode 是否已处理
     *
     * 使用 device + inode 组合避免硬链接重复统计
     * 只对 st_nlink > 1 的文件调用此方法
     */
    bool isInodeProcessed(quint64 device, quint64 inode);

    /**
     * @brief 标记 inode 为已处理
     */
    void markInodeProcessed(quint64 device, quint64 inode);

    /**
     * @brief 发送进度通知
     * @param force 是否强制发送
     */
    void emitProgress(bool force = false);

private:
    QList<QUrl> urls;
    FileScanner::ScanOptions options { FileScanner::ScanOption::NoOption };

    // 当前结果（工作线程独占，无需锁）
    FileScanner::ScanResult currentResult;

    // 控制标志
    std::atomic<bool> stopped { false };

    // 进度节流
    QElapsedTimer progressTimer;
    qint64 lastEmittedSize { 0 };

    // 内存页大小
    qint64 memoryPageSize { 4096 };

    // inode 去重 - 只对 st_nlink > 1 的文件存储
    // QHash<device, QSet<inode>>
    QHash<quint64, QSet<quint64>> processedInodes;
};

}   // namespace dfmbase

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::FileScanner::ScanResult)

#endif   // FILESCANNER_H
