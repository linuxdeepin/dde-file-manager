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
#include <QDir>
#include <QQueue>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

DFMBASE_USE_NAMESPACE

namespace dfmbase {

//===================================================================
// FileScannerCore - 核心扫描逻辑（纯算法，无 QObject 依赖）
//===================================================================
class FileScannerCore
{
public:
    // 进度回调函数类型：返回 true 继续扫描，返回 false 停止扫描
    using ProgressCallback = std::function<bool(const FileScanner::ScanResult &)>;

    // 目录条目结构
    struct DirEntry
    {
        QString name;
        unsigned char d_type;
        struct stat statBuf;
        bool statOk { false };
    };

    // 扫描上下文结构
    struct ScanContext
    {
        QString fullPath;
        int depth { 0 };
        bool isSourcePath { false };
    };

    // 核心扫描方法
    static FileScanner::ScanResult scanImpl(
            const QList<QUrl> &urls,
            FileScanner::ScanOptions options,
            ProgressCallback progressCallback = nullptr);

private:
    // 扫描状态（在扫描过程中维护）
    struct ScanState
    {
        FileScanner::ScanResult result;
        FileScanner::ScanOptions options;
        ProgressCallback progressCallback;

        // 进度节流
        QElapsedTimer progressTimer;
        qint64 lastEmittedSize { 0 };
        qint64 memoryPageSize { 4096 };

        // inode 去重
        QHash<quint64, QSet<quint64>> processedInodes;

        // 停止标志（用于回调返回 false 时停止）
        bool shouldStop { false };
    };

    // 核心扫描逻辑
    static void scanLocalPathsImpl(ScanState &state, const QList<QUrl> &urls);
    static void scanOtherProtocolsImpl(ScanState &state, const QList<QUrl> &urls);

    // 辅助方法
    static qint64 progressDeltaForFileSize(const ScanState &state, qint64 fileSize);
    static bool tryScanOtherProtocolCountOnlyByLocalPath(
            ScanState &state,
            const FileInfoPointer &info,
            bool isSingleDepth,
            QQueue<QUrl> *directoryQueue,
            QSet<QUrl> *processedUrls);
    static bool readDirectoryEntries(const QString &path, QList<DirEntry> *entries, bool countOnly = false);
    static void processRegularFile(ScanState &state, const QString &path, const struct stat &statBuf);
    static void processSymlink(ScanState &state, const QString &path);
    static bool isDirectoryPath(const QString &path, const struct stat &lstatBuf);
    static void collectFileIfEnabled(ScanState &state, const QUrl &url, bool isSourcePath);
    static void emitProgress(ScanState &state, bool force = false);
    static bool isInodeProcessed(const ScanState &state, quint64 device, quint64 inode);
    static void markInodeProcessed(ScanState &state, quint64 device, quint64 inode);
};

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
    Q_UNUSED(isFinal)
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

FileScanner::ScanResult FileScanner::scanSync(const QList<QUrl> &urls, ScanOptions options)
{
    if (urls.isEmpty()) {
        qCWarning(logDFMBase) << "FileScanner::scanSync: Empty URL list";
        return ScanResult();
    }

    qCDebug(logDFMBase) << "FileScanner::scanSync: Starting sync scan for" << urls.count() << "URLs";

    // 直接调用核心逻辑（无进度回调）
    ScanResult result = FileScannerCore::scanImpl(urls, options, nullptr);

    qCDebug(logDFMBase) << "FileScanner::scanSync: Completed - files:" << result.fileCount
                        << "dirs:" << result.directoryCount << "size:" << result.totalSize;

    return result;
}

FileScanner::ScanResult FileScanner::scanSyncWithCallback(const QList<QUrl> &urls,
                                                          ScanOptions options,
                                                          ProgressCallback progressCallback)
{
    if (urls.isEmpty()) {
        qCWarning(logDFMBase) << "FileScanner::scanSyncWithCallback: Empty URL list";
        return ScanResult();
    }

    qCDebug(logDFMBase) << "FileScanner::scanSyncWithCallback: Starting interruptible sync scan for" << urls.count() << "URLs";

    // 调用核心逻辑（带进度回调，支持中断）
    ScanResult result = FileScannerCore::scanImpl(urls, options, progressCallback);

    qCDebug(logDFMBase) << "FileScanner::scanSyncWithCallback: Completed - files:" << result.fileCount
                        << "dirs:" << result.directoryCount << "size:" << result.totalSize;

    return result;
}

//===================================================================
// FileScannerCore - 核心扫描逻辑实现
//===================================================================
FileScanner::ScanResult FileScannerCore::scanImpl(
        const QList<QUrl> &urls,
        FileScanner::ScanOptions options,
        ProgressCallback progressCallback)
{
    ScanState state;
    state.options = options;
    state.progressCallback = progressCallback;
    state.memoryPageSize = FileUtils::getMemoryPageSize();
    state.progressTimer.start();

    // 判断是否为本地文件路径
    if (!urls.isEmpty() && urls.first().scheme() == Global::Scheme::kFile) {
        scanLocalPathsImpl(state, urls);
    } else {
        scanOtherProtocolsImpl(state, urls);
    }

    return state.result;
}

qint64 FileScannerCore::progressDeltaForFileSize(const ScanState &state, qint64 fileSize)
{
    return fileSize <= 0 ? state.memoryPageSize : fileSize;
}

void FileScannerCore::scanLocalPathsImpl(ScanState &state, const QList<QUrl> &urls)
{
    qCDebug(logDFMBase) << "FileScannerCore: Scanning local paths using opendir/readdir";

    // ========== 初始化阶段 ==========
    int processedSourceDirs = 0;
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
                // 收集源目录URL（如果启用 CollectFiles 选项）
                collectFileIfEnabled(state, url, true);
            } else {
                // 非目录：直接收集（普通文件、符号链接等）
                if (state.options & FileScanner::ScanOption::CountOnly) {
                    state.result.fileCount++;
                } else if (S_ISREG(statBuf.st_mode)) {
                    processRegularFile(state, path, statBuf);
                } else if (S_ISLNK(statBuf.st_mode)) {
                    processSymlink(state, path);
                } else {
                    state.result.fileCount++;
                }
                // 收集源文件URL
                collectFileIfEnabled(state, url, true);
            }
        } else {
            qCWarning(logDFMBase) << "FileScannerCore: lstat failed for source:" << path;
        }
    }

    // ========== 遍历阶段 ==========
    while (!dirStack.isEmpty() && !state.shouldStop) {
        ScanContext ctx = dirStack.pop();
        const QString &dirPath = ctx.fullPath;

        // 先计数目录本身（无论是否能读取内容）
        state.result.directoryCount++;
        state.result.progressSize += state.memoryPageSize;

        // 记录成功处理的源目录（用于最终扣除）
        if (ctx.isSourcePath) {
            processedSourceDirs++;
        }

        // 读取目录内容
        QList<DirEntry> entries;
        bool countOnly = state.options & FileScanner::ScanOption::CountOnly;
        bool readSuccess = readDirectoryEntries(dirPath, &entries, countOnly);

        if (!readSuccess) {
            // 目录读取失败（权限不足），但目录已计数
            qCWarning(logDFMBase) << "FileScannerCore: Failed to read directory contents:" << dirPath;
            continue;
        }

        // 处理每个条目
        for (const DirEntry &entry : entries) {
            if (state.shouldStop) {
                break;
            }

            const QString entryPath = dirPath + "/" + entry.name;
            const QUrl entryUrl = QUrl::fromLocalFile(entryPath);

            if (countOnly) {
                // CountOnly 模式：直接用 d_type 计数，无需 stat
                if (entry.d_type == DT_DIR) {
                    bool isSingleDepth = state.options & FileScanner::ScanOption::SingleDepth;
                    if (isSingleDepth) {
                        state.result.directoryCount++;
                    } else {
                        ScanContext childCtx;
                        childCtx.fullPath = entryPath;
                        childCtx.depth = ctx.depth + 1;
                        childCtx.isSourcePath = false;
                        dirStack.push(childCtx);
                    }
                } else {
                    // 普通文件、符号链接、其他类型统一计数
                    state.result.fileCount++;
                }
                collectFileIfEnabled(state, entryUrl, false);
                emitProgress(state);
                continue;
            }

            // 跳过特殊系统文件
            if (entry.statOk && S_ISREG(entry.statBuf.st_mode)) {
                static const QSet<QString> kSpecialSystemFiles {
                    "/proc/kcore",
                    "/dev/core"
                };
                if (kSpecialSystemFiles.contains(entryPath)) {
                    qCDebug(logDFMBase) << "FileScannerCore: Skipping special file:" << entryPath;
                    continue;
                }
            }

            // 失败处理
            if (!entry.statOk) {
                qCWarning(logDFMBase) << "FileScannerCore: stat failed for:" << entryPath;
                continue;
            }

            // 根据类型处理条目
            if (S_ISDIR(entry.statBuf.st_mode)) {
                // 子目录
                bool isSingleDepth = state.options & FileScanner::ScanOption::SingleDepth;
                if (isSingleDepth) {
                    // SingleDepth 模式：计数但不递归
                    state.result.directoryCount++;
                    state.result.progressSize += state.memoryPageSize;
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
                processRegularFile(state, entryPath, entry.statBuf);
            } else if (S_ISLNK(entry.statBuf.st_mode)) {
                // 符号链接
                processSymlink(state, entryPath);
            } else {
                // 其他特殊文件类型
                state.result.fileCount++;
            }

            // 收集文件URL
            collectFileIfEnabled(state, entryUrl, false);

            // 定期发送进度
            emitProgress(state);
        }
    }

    // ========== 最终处理 ==========
    // 默认排除源目录本身
    if (!(state.options & FileScanner::ScanOption::IncludeSource)) {
        state.result.directoryCount -= processedSourceDirs;
    }

    qCDebug(logDFMBase) << "FileScannerCore: Local scan completed - files:" << state.result.fileCount
                        << "dirs:" << state.result.directoryCount << "size:" << state.result.totalSize;
}

void FileScannerCore::scanOtherProtocolsImpl(ScanState &state, const QList<QUrl> &urls)
{
    qCDebug(logDFMBase) << "FileScannerCore: Scanning other protocols using InfoFactory";

    QQueue<QUrl> directoryQueue;
    QSet<QUrl> processedUrls;
    int sourceDirCount = 0;

    // 初始化队列
    for (const QUrl &url : urls) {
        directoryQueue.enqueue(url);
        processedUrls.insert(url);
    }

    while (!directoryQueue.isEmpty() && !state.shouldStop) {
        QUrl url = directoryQueue.dequeue();

        // 判断是否为源路径
        bool isSourcePath = urls.contains(url);

        // 使用 InfoFactory 创建文件信息
        FileInfoPointer info = InfoFactory::create<FileInfo>(
                url,
                Global::CreateFileInfoType::kCreateFileInfoSync);

        if (!info) {
            qCWarning(logDFMBase) << "FileScannerCore: Failed to create info for:" << url;
            continue;
        }

        // 检查是否为目录
        if (info->isAttributes(OptInfoType::kIsDir)) {
            // 记录源目录数量
            if (isSourcePath) {
                sourceDirCount++;
            }

            state.result.directoryCount++;
            state.result.progressSize += state.memoryPageSize;

            // 收集目录URL
            collectFileIfEnabled(state, url, isSourcePath);

            bool isSingleDepth = state.options & FileScanner::ScanOption::SingleDepth;
            bool countOnly = state.options & FileScanner::ScanOption::CountOnly;
            if (countOnly && tryScanOtherProtocolCountOnlyByLocalPath(state, info, isSingleDepth, &directoryQueue, &processedUrls)) {
                continue;
            }

            // 创建目录迭代器（快速路径不可用时回退）
            AbstractDirIteratorPointer iterator = DirIteratorFactory::create<AbstractDirIterator>(
                    url,
                    QStringList(),
                    QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

            if (!iterator) {
                qCWarning(logDFMBase) << "FileScannerCore: Failed to create iterator for:" << url;
                continue;
            }

            // 遍历目录
            while (iterator->hasNext() && !state.shouldStop) {
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
                                state.result.directoryCount++;
                                state.result.progressSize += state.memoryPageSize;
                            } else {
                                // 递归模式：入队
                                directoryQueue.enqueue(childUrl);
                            }
                            // 收集子目录URL
                            collectFileIfEnabled(state, childUrl, false);
                        } else {
                            // 处理文件
                            state.result.fileCount++;
                            if (!(state.options & FileScanner::ScanOption::CountOnly)) {
                                qint64 sz = childInfo->size();
                                state.result.totalSize += sz;
                                state.result.progressSize += progressDeltaForFileSize(state, sz);
                            }
                            // 收集子文件URL
                            collectFileIfEnabled(state, childUrl, false);
                        }
                    }
                }
            }
        } else {
            // 处理文件
            state.result.fileCount++;
            if (!(state.options & FileScanner::ScanOption::CountOnly)) {
                qint64 sz = info->size();
                state.result.totalSize += sz;
                state.result.progressSize += progressDeltaForFileSize(state, sz);
            }

            // 收集文件URL
            collectFileIfEnabled(state, url, false);
        }

        // 定期发送进度
        emitProgress(state);
    }

    // 默认排除源目录本身
    if (!(state.options & FileScanner::ScanOption::IncludeSource)) {
        state.result.directoryCount -= sourceDirCount;
    }

    qCDebug(logDFMBase) << "FileScannerCore: Other protocol scan completed - files:" << state.result.fileCount
                        << "dirs:" << state.result.directoryCount << "size:" << state.result.totalSize;
}

bool FileScannerCore::tryScanOtherProtocolCountOnlyByLocalPath(
        ScanState &state,
        const FileInfoPointer &info,
        bool isSingleDepth,
        QQueue<QUrl> *directoryQueue,
        QSet<QUrl> *processedUrls)
{
    Q_ASSERT(directoryQueue);
    Q_ASSERT(processedUrls);

    // CountOnly 模式下，优先复用 readDirectoryEntries，避免 DirIteratorFactory 的高开销
    const QString dirPath = info->pathOf(PathInfoType::kAbsoluteFilePath);
    const bool canUseLocalPath = QDir::isAbsolutePath(dirPath) && QDir(dirPath).exists();
    if (!canUseLocalPath) {
        return false;
    }

    QList<DirEntry> entries;
    if (!readDirectoryEntries(dirPath, &entries, true)) {
        return false;
    }

    for (const DirEntry &entry : entries) {
        if (state.shouldStop) {
            break;
        }

        QUrl childUrl = info->getUrlByType(UrlInfoType::kGetUrlByChildFileName, entry.name);
        if (!childUrl.isValid()) {
            continue;
        }

        if (processedUrls->contains(childUrl)) {
            continue;
        }
        processedUrls->insert(childUrl);

        if (entry.d_type == DT_DIR) {
            if (isSingleDepth) {
                // SingleDepth 模式：计数但不递归
                state.result.directoryCount++;
                state.result.progressSize += state.memoryPageSize;
            } else {
                // 递归模式：入队
                directoryQueue->enqueue(childUrl);
            }
        } else {
            // CountOnly 模式：普通文件、符号链接、其他类型统一计数
            state.result.fileCount++;
        }

        // 收集子文件/目录URL
        collectFileIfEnabled(state, childUrl, false);
        emitProgress(state);
    }

    return true;
}

bool FileScannerCore::readDirectoryEntries(const QString &path, QList<DirEntry> *entries, bool countOnly)
{
    Q_ASSERT(entries);
    entries->clear();

    QByteArray pathUtf8 = path.toUtf8();
    DIR *dir = opendir(pathUtf8.constData());
    if (!dir) {
        qCWarning(logDFMBase) << "FileScannerCore: opendir failed for" << path << ":" << strerror(errno);
        return false;
    }

    int dirFd = dirfd(dir);
    if (dirFd == -1) {
        qCWarning(logDFMBase) << "FileScannerCore: invalid dirfd for" << path;
        closedir(dir);
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        const QByteArray nameBytes(entry->d_name);
        DirEntry dirEntry;
        dirEntry.name = QString::fromUtf8(nameBytes);
        dirEntry.d_type = entry->d_type;

        if (countOnly) {
            // CountOnly 模式：优先用 d_type，仅在 DT_UNKNOWN 时 fstatat 取类型
            if (entry->d_type == DT_UNKNOWN) {
                struct stat sb;
                if (fstatat(dirFd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW) == 0) {
                    if (S_ISDIR(sb.st_mode))
                        dirEntry.d_type = DT_DIR;
                    else if (S_ISLNK(sb.st_mode))
                        dirEntry.d_type = DT_LNK;
                    else if (S_ISREG(sb.st_mode))
                        dirEntry.d_type = DT_REG;
                }
            }
            dirEntry.statOk = false;   // CountOnly 不需要 statBuf
        } else {
            dirEntry.statOk = (fstatat(dirFd, entry->d_name, &dirEntry.statBuf, AT_SYMLINK_NOFOLLOW) == 0);
            if (!dirEntry.statOk) {
                qCWarning(logDFMBase) << "FileScannerCore: fstatat failed for" << dirEntry.name << "in" << path;
            }
        }

        entries->append(dirEntry);
    }

    closedir(dir);
    return true;
}

void FileScannerCore::processRegularFile(ScanState &state, const QString &path, const struct stat &statBuf)
{
    // 硬链接去重
    if (statBuf.st_nlink > 1) {
        if (!isInodeProcessed(state, statBuf.st_dev, statBuf.st_ino)) {
            markInodeProcessed(state, statBuf.st_dev, statBuf.st_ino);
            state.result.totalSize += statBuf.st_size;
            state.result.fileCount++;
        } else {
            // 硬链接已统计，只计数
            state.result.fileCount++;
        }
    } else {
        // st_nlink == 1，直接处理
        state.result.totalSize += statBuf.st_size;
        state.result.fileCount++;
    }
    state.result.progressSize += progressDeltaForFileSize(state, statBuf.st_size);
}

void FileScannerCore::processSymlink(ScanState &state, const QString &path)
{
    // 符号链接只计数，不跟随（不计入大小）
    state.result.fileCount++;
    state.result.progressSize += state.memoryPageSize;
}

bool FileScannerCore::isDirectoryPath(const QString &path, const struct stat &lstatBuf)
{
    if (S_ISDIR(lstatBuf.st_mode)) {
        return true;
    }
    if (S_ISLNK(lstatBuf.st_mode)) {
        struct stat statBuf;
        if (stat(path.toUtf8().constData(), &statBuf) == 0) {
            return S_ISDIR(statBuf.st_mode);
        }
    }
    return false;
}

void FileScannerCore::collectFileIfEnabled(ScanState &state, const QUrl &url, bool isSourcePath)
{
    // 只有启用 CollectFiles 选项才收集
    if (!(state.options & FileScanner::ScanOption::CollectFiles)) {
        return;
    }

    // 如果是源路径且未设置 IncludeSource 选项，则不收集
    if (isSourcePath && !(state.options & FileScanner::ScanOption::IncludeSource)) {
        return;
    }

    state.result.allFiles.append(url);
}

void FileScannerCore::emitProgress(ScanState &state, bool force)
{
    qint64 currentSize = state.result.totalSize;

    // 节流条件
    if (force || state.progressTimer.elapsed() > 500 || qAbs(currentSize - state.lastEmittedSize) > 10 * 1024 * 1024) {
        if (state.progressCallback) {
            bool shouldContinue = state.progressCallback(state.result);
            if (!shouldContinue) {
                state.shouldStop = true;
            }
        }
        state.lastEmittedSize = currentSize;
        state.progressTimer.restart();
    }
}

bool FileScannerCore::isInodeProcessed(const ScanState &state, quint64 device, quint64 inode)
{
    return state.processedInodes.value(device).contains(inode);
}

void FileScannerCore::markInodeProcessed(ScanState &state, quint64 device, quint64 inode)
{
    if (!state.processedInodes.contains(device)) {
        state.processedInodes.insert(device, QSet<quint64>());
    }
    state.processedInodes[device].insert(inode);
}

//===================================================================
// ScannerWorker - 工作对象
//===================================================================
ScannerWorker::ScannerWorker(QObject *parent)
    : QObject(parent)
{
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

    // 使用核心逻辑，通过回调发送进度
    auto progressCallback = [this](const FileScanner::ScanResult &result) -> bool {
        if (shouldStop()) {
            return false;   // 返回 false 表示停止扫描
        }
        emit resultReady(result, false);
        return true;   // 继续扫描
    };

    // 调用核心扫描逻辑
    FileScanner::ScanResult finalResult = FileScannerCore::scanImpl(urls, options, progressCallback);

    // 发送最终结果
    emit resultReady(finalResult, true);
    emit finished();
}

void ScannerWorker::stop()
{
    qCDebug(logDFMBase) << "ScannerWorker: Stop requested";
    stopped = true;
}

bool ScannerWorker::shouldStop() const
{
    return stopped.load();
}

}   // namespace dfmbase
