// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filescanner.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

#include <QTimer>
#include <QCoreApplication>
#include <QDebug>
#include <QQueue>

#include <dirent.h>
#include <unistd.h>

DFMBASE_USE_NAMESPACE

namespace dfmbase {

//===================================================================
// FileScannerPrivate - 私有实现
//===================================================================
class FileScannerPrivate : public QObject
{
public:
    explicit FileScannerPrivate(FileScanner *qq);
    ~FileScannerPrivate() override;

    void startWorker(const QList<QUrl> &urls);
    void stopWorker();

    void onWorkerResultReady(const FileScanner::ScanResult &result, bool isFinal);
    void onWorkerFinished();

public:
    FileScanner *q;

    // 线程管理（栈对象，自动管理生命周期）
    QThread workerThread;
    ScannerWorker *worker { nullptr };

    // 当前结果（在主线程中访问，无需锁）
    FileScanner::ScanResult lastResult;

    // 选项
    FileScanner::ScanOptions options { FileScanner::ScanOption::NoOption };
};

FileScannerPrivate::FileScannerPrivate(FileScanner *qq)
    : QObject(qq), q(qq)
{
}

FileScannerPrivate::~FileScannerPrivate()
{
}

void FileScannerPrivate::startWorker(const QList<QUrl> &urls)
{
    // 如果已有工作线程在运行，先完全停止并清理
    if (workerThread.isRunning()) {
        stopWorker();
        workerThread.quit();
        workerThread.wait();   // 等待线程完全停止，避免竞态条件
        // 线程结束后，worker 已通过 deleteLater 删除
        worker = nullptr;
    }

    // 创建新的工作者（每次都创建新的，避免重用导致的状态混乱）
    worker = new ScannerWorker();
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

    // 设置参数
    worker->setUrls(urls);
    worker->setOptions(options);

    // 连接信号
    connect(worker, &ScannerWorker::resultReady, this, &FileScannerPrivate::onWorkerResultReady);
    connect(worker, &ScannerWorker::finished, this, &FileScannerPrivate::onWorkerFinished);

    // 启动工作线程
    if (!workerThread.isRunning()) {
        workerThread.start();
    }
    QMetaObject::invokeMethod(worker, "start", Qt::QueuedConnection);
}

void FileScannerPrivate::stopWorker()
{
    if (worker) {
        worker->stop();
    }
}

void FileScannerPrivate::onWorkerResultReady(const FileScanner::ScanResult &result, bool isFinal)
{
    lastResult = result;
    emit q->progressChanged(result);
}

void FileScannerPrivate::onWorkerFinished()
{
    // 断开连接并清理指针
    // worker 将通过 QThread::finished 信号触发 deleteLater 自动删除
    worker = nullptr;

    emit q->finished(lastResult);
}

//===================================================================
// FileScanner - 公共接口
//===================================================================
FileScanner::FileScanner(QObject *parent)
    : QObject(parent), d(new FileScannerPrivate(this))
{
    // 注册元类型
    qRegisterMetaType<ScanResult>("FileScanner::ScanResult");

    // 程序退出时不阻塞
    connect(
            qApp, &QCoreApplication::aboutToQuit, this, [this] {
                if (isRunning()) {
                    d->stopWorker();
                    d->workerThread.quit();   // 不调用 wait()
                    qCDebug(logDFMBase) << "FileScanner: Stopping due to application quit";
                }
            },
            Qt::QueuedConnection);
}

FileScanner::~FileScanner()
{
    d->stopWorker();

    if (d->workerThread.isRunning()) {
        d->workerThread.quit();
        d->workerThread.wait();
    }
}

void FileScanner::setOptions(ScanOptions options)
{
    d->options = options;
}

FileScanner::ScanOptions FileScanner::options() const
{
    return d->options;
}

FileScanner::ScanResult FileScanner::result() const
{
    return d->lastResult;
}

bool FileScanner::isRunning() const
{
    return d->workerThread.isRunning();
}

void FileScanner::start(const QList<QUrl> &urls)
{
    if (urls.isEmpty()) {
        qCWarning(logDFMBase) << "FileScanner: Empty URL list";
        emit finished(ScanResult());
        return;
    }

    qCDebug(logDFMBase) << "FileScanner: Starting scan for" << urls.count() << "URLs";
    d->startWorker(urls);
}

void FileScanner::stop()
{
    qCDebug(logDFMBase) << "FileScanner: Stopping scan";
    d->stopWorker();
}

//===================================================================
// ScannerWorker - 工作对象
//===================================================================
ScannerWorker::ScannerWorker(QObject *parent)
    : QObject(parent)
{
    memoryPageSize = FileUtils::getMemoryPageSize();
}

ScannerWorker::~ScannerWorker()
{
}

void ScannerWorker::setUrls(const QList<QUrl> &urls)
{
    this->urls = urls;
}

void ScannerWorker::setOptions(FileScanner::ScanOptions options)
{
    this->options = options;
}

void ScannerWorker::start()
{
    qCDebug(logDFMBase) << "ScannerWorker: Starting work";

    // 重置状态
    stopped = false;
    currentResult.clear();
    processedInodes.clear();
    progressTimer.start();
    lastEmittedSize = 0;

    // 判断是否为本地文件路径
    if (!urls.isEmpty() && urls.first().scheme() == Global::Scheme::kFile) {
        scanLocalPaths();
    } else {
        scanOtherProtocols();
    }

    // 发送最终结果
    emitProgress(true);
    emit finished();
}

void ScannerWorker::stop()
{
    qCDebug(logDFMBase) << "ScannerWorker: Stop requested";
    stopped = true;
}

void ScannerWorker::scanLocalPaths()
{
    qCDebug(logDFMBase) << "ScannerWorker: Scanning local paths using opendir/readdir";

    // ========== 初始化阶段 ==========
    int processedSourceDirs = 0;   // 实际成功处理的源目录数
    QStack<ScanContext> dirStack;

    // 准备源路径
    for (const QUrl &url : urls) {
        QString path = url.path();

        struct stat statBuf;
        if (lstat(path.toUtf8().constData(), &statBuf) == 0) {
            // 判断是否为目录（包括指向目录的符号链接）
            if (isDirectoryPath(path, statBuf)) {
                ScanContext ctx;
                ctx.fullPath = path;
                ctx.depth = 0;
                ctx.isSourcePath = true;
                dirStack.push(ctx);
            } else if (S_ISREG(statBuf.st_mode)) {
                // 普通文件
                processRegularFile(path, statBuf);
            } else if (S_ISLNK(statBuf.st_mode)) {
                // 符号链接
                processSymlink(path);
            } else {
                // 其他特殊文件类型（socket、FIFO、字符设备、块设备等）
                // 只计数，不计大小
                currentResult.fileCount++;
            }
        } else {
            qCWarning(logDFMBase) << "ScannerWorker: lstat failed for source:" << path;
        }
    }

    // ========== 遍历阶段 ==========
    while (!dirStack.isEmpty() && !shouldStop()) {
        ScanContext ctx = dirStack.pop();
        const QString &dirPath = ctx.fullPath;

        // 先计数目录本身（无论是否能读取内容）
        currentResult.directoryCount++;
        currentResult.progressSize += memoryPageSize;

        // 记录成功处理的源目录（用于最终扣除）
        if (ctx.isSourcePath) {
            processedSourceDirs++;
        }

        // 读取目录内容
        QList<DirEntry> entries;
        bool readSuccess = readDirectoryEntries(dirPath, &entries);

        if (!readSuccess) {
            // 目录读取失败（权限不足），但目录已计数
            qCWarning(logDFMBase) << "ScannerWorker: Failed to read directory contents:" << dirPath;
            continue;   // 无法读取内容，跳过条目处理
        }

        // 处理每个条目
        for (const DirEntry &entry : entries) {
            if (shouldStop()) {
                break;
            }

            QString entryPath = dirPath + "/" + QString::fromUtf8(entry.name);

            // 跳过特殊系统文件
            if (entry.statOk && S_ISREG(entry.statBuf.st_mode)) {
                static const QSet<QString> kSpecialSystemFiles {
                    "/proc/kcore",
                    "/dev/core"
                };
                if (kSpecialSystemFiles.contains(entryPath)) {
                    qCDebug(logDFMBase) << "ScannerWorker: Skipping special file:" << entryPath;
                    continue;
                }
            }

            // lstat 失败处理：部分条目可能无法访问，但继续处理其他条目
            if (!entry.statOk) {
                qCDebug(logDFMBase) << "ScannerWorker: lstat failed for:" << entryPath;
                continue;
            }

            // 根据类型处理条目
            if (S_ISDIR(entry.statBuf.st_mode)) {
                // 子目录
                bool isSingleDepth = options & FileScanner::ScanOption::SingleDepth;
                if (isSingleDepth) {
                    // SingleDepth 模式：计数但不递归
                    currentResult.directoryCount++;
                    currentResult.progressSize += memoryPageSize;
                } else {
                    // 递归模式：压栈
                    ScanContext childCtx;
                    childCtx.fullPath = entryPath;
                    childCtx.depth = ctx.depth + 1;
                    childCtx.isSourcePath = false;
                    dirStack.push(childCtx);
                }
            } else if (S_ISREG(entry.statBuf.st_mode)) {
                // 常规文件
                processRegularFile(entryPath, entry.statBuf);
            } else if (S_ISLNK(entry.statBuf.st_mode)) {
                // 符号链接
                processSymlink(entryPath);
            } else {
                // 其他特殊文件类型（socket、FIFO、字符设备、块设备等）
                // 只计数，不计大小
                currentResult.fileCount++;
            }

            // 定期发送进度
            emitProgress();
        }
    }

    // ========== 最终处理 ==========
    // 默认排除源目录本身（只扣除成功处理的源目录）
    if (!(options & FileScanner::ScanOption::IncludeSource)) {
        currentResult.directoryCount -= processedSourceDirs;
    }

    qCDebug(logDFMBase) << "ScannerWorker: Local scan completed - files:" << currentResult.fileCount
                        << "dirs:" << currentResult.directoryCount
                        << "size:" << currentResult.totalSize;
}

void ScannerWorker::scanOtherProtocols()
{
    qCDebug(logDFMBase) << "ScannerWorker: Scanning other protocols using InfoFactory";

    QQueue<QUrl> directoryQueue;
    QSet<QUrl> processedUrls;
    int sourceDirCount = 0;   // 记录源目录数量

    // 初始化队列
    for (const QUrl &url : urls) {
        directoryQueue.enqueue(url);
        processedUrls.insert(url);
    }

    while (!directoryQueue.isEmpty() && !shouldStop()) {
        QUrl url = directoryQueue.dequeue();

        // 使用 InfoFactory 创建文件信息
        FileInfoPointer info = InfoFactory::create<FileInfo>(
                url,
                Global::CreateFileInfoType::kCreateFileInfoSync);

        if (!info) {
            qCWarning(logDFMBase) << "ScannerWorker: Failed to create info for:" << url;
            continue;
        }

        // 检查是否为目录
        if (info->isAttributes(OptInfoType::kIsDir)) {
            // 检查是否是源 URL
            if (urls.contains(url)) {
                sourceDirCount++;
            }

            currentResult.directoryCount++;
            currentResult.progressSize += memoryPageSize;

            // 创建目录迭代器
            AbstractDirIteratorPointer iterator = DirIteratorFactory::create<AbstractDirIterator>(
                    url,
                    QStringList(),
                    QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

            if (!iterator) {
                qCWarning(logDFMBase) << "ScannerWorker: Failed to create iterator for:" << url;
                continue;
            }

            bool isSingleDepth = options & FileScanner::ScanOption::SingleDepth;

            // 遍历目录
            while (iterator->hasNext() && !shouldStop()) {
                QUrl childUrl = iterator->next();

                if (!processedUrls.contains(childUrl)) {
                    processedUrls.insert(childUrl);

                    // 递归处理子文件/目录
                    FileInfoPointer childInfo = InfoFactory::create<FileInfo>(
                            childUrl,
                            Global::CreateFileInfoType::kCreateFileInfoSync);

                    if (childInfo) {
                        if (childInfo->isAttributes(OptInfoType::kIsDir)) {
                            if (isSingleDepth) {
                                // SingleDepth 模式：计数但不递归
                                currentResult.directoryCount++;
                                currentResult.progressSize += memoryPageSize;
                            } else {
                                // 递归模式：入队
                                directoryQueue.enqueue(childUrl);
                            }
                        } else {
                            // 处理文件
                            currentResult.totalSize += childInfo->size();
                            currentResult.fileCount++;
                            currentResult.progressSize += childInfo->size();
                        }
                    }
                }
            }
        } else {
            // 处理文件
            currentResult.totalSize += info->size();
            currentResult.fileCount++;
            currentResult.progressSize += info->size();
        }

        // 定期发送进度
        emitProgress();
    }

    // 默认排除源目录本身（除非设置了 IncludeSource 选项）
    if (!(options & FileScanner::ScanOption::IncludeSource)) {
        currentResult.directoryCount -= sourceDirCount;
    }

    qCDebug(logDFMBase) << "ScannerWorker: Other protocol scan completed - files:" << currentResult.fileCount
                        << "dirs:" << currentResult.directoryCount
                        << "size:" << currentResult.totalSize;
}

bool ScannerWorker::shouldStop() const
{
    return stopped.load();
}

bool ScannerWorker::isInodeProcessed(quint64 device, quint64 inode)
{
    return processedInodes.value(device).contains(inode);
}

void ScannerWorker::markInodeProcessed(quint64 device, quint64 inode)
{
    if (!processedInodes.contains(device)) {
        processedInodes.insert(device, QSet<quint64>());
    }
    processedInodes[device].insert(inode);
}

void ScannerWorker::emitProgress(bool force)
{
    qint64 currentSize = currentResult.totalSize;

    // 节流条件：
    // 1. 距离上次发送超过500ms
    // 2. 或者大小变化超过10MB
    // 3. 或者强制发送（完成时）
    if (force || progressTimer.elapsed() > 500 || qAbs(currentSize - lastEmittedSize) > 10 * 1024 * 1024) {
        emit resultReady(currentResult, force);
        lastEmittedSize = currentSize;
        progressTimer.restart();
    }
}

bool ScannerWorker::readDirectoryEntries(const QString &path, QList<DirEntry> *entries)
{
    Q_ASSERT(entries);
    entries->clear();

    QByteArray pathUtf8 = path.toUtf8();
    DIR *dir = opendir(pathUtf8.constData());
    if (!dir) {
        qCWarning(logDFMBase) << "ScannerWorker: opendir failed for" << path << ":" << strerror(errno);
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 "." 和 ".."
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue;
        }

        DirEntry dirEntry;
        dirEntry.name = entry->d_name;
        dirEntry.d_type = entry->d_type;

        QString fullPath = path + "/" + QString::fromUtf8(entry->d_name);
        dirEntry.statOk = (lstat(fullPath.toUtf8().constData(), &dirEntry.statBuf) == 0);

        entries->append(dirEntry);
    }

    closedir(dir);
    return true;
}

void ScannerWorker::processRegularFile(const QString &path, const struct stat &statBuf)
{
    // 硬链接去重
    if (statBuf.st_nlink > 1) {
        if (!isInodeProcessed(statBuf.st_dev, statBuf.st_ino)) {
            markInodeProcessed(statBuf.st_dev, statBuf.st_ino);
            currentResult.totalSize += statBuf.st_size;
            currentResult.fileCount++;
        } else {
            // 硬链接已统计，只计数
            currentResult.fileCount++;
        }
    } else {
        // st_nlink == 1，直接处理
        currentResult.totalSize += statBuf.st_size;
        currentResult.fileCount++;
    }
    currentResult.progressSize += statBuf.st_size;
}

void ScannerWorker::processSymlink(const QString &path)
{
    // 符号链接只计数，不跟随（不计入大小）
    currentResult.fileCount++;
    currentResult.progressSize += memoryPageSize;   // 估算符号链接大小
}

bool ScannerWorker::isDirectoryPath(const QString &path, const struct stat &lstatBuf)
{
    if (S_ISDIR(lstatBuf.st_mode)) {
        return true;   // 直接是目录
    }
    if (S_ISLNK(lstatBuf.st_mode)) {
        struct stat statBuf;
        if (stat(path.toUtf8().constData(), &statBuf) == 0) {
            return S_ISDIR(statBuf.st_mode);   // 符号链接指向目录
        }
    }
    return false;
}

}   // namespace dfmbase
