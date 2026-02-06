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
    if (workerThread.isRunning()) {
        workerThread.quit();
        workerThread.wait(5000);
    }
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
    qCDebug(logDFMBase) << "ScannerWorker: Scanning local paths using fts(3)";

    // 准备路径数组（使用 QByteArray 自动管理内存）
    QVector<QByteArray> pathBytes(urls.size());
    QVector<char *> paths(urls.size() + 1);
    QSet<QString> sourcePaths;   // 记录源路径用于排除

    for (int i = 0; i < urls.size(); ++i) {
        QString path = urls[i].path();
        pathBytes[i] = path.toUtf8();
        paths[i] = pathBytes[i].data();
        sourcePaths.insert(path);
    }
    paths[urls.size()] = nullptr;   // fts_open 要求 NULL 终止的数组

    // 打开 fts
    FTS *ftsp = fts_open(paths.data(), FTS_PHYSICAL | FTS_NOCHDIR, nullptr);
    if (!ftsp) {
        qCWarning(logDFMBase) << "ScannerWorker: fts_open failed:" << strerror(errno);
        return;   // QByteArray 自动释放，无需手动 free
    }

    // 记录作为源路径的目录数量（只有目录才需要排除）
    int sourceDirCount = 0;

    FTSENT *p;
    while ((p = fts_read(ftsp)) != nullptr) {
        if (shouldStop()) {
            qCDebug(logDFMBase) << "ScannerWorker: Stopped by user";
            break;
        }

        switch (p->fts_info) {
        case FTS_D:   // 目录（进入前）
        {
            QString currentPath = QString::fromUtf8(p->fts_path);

            // 检查是否是源路径
            if (sourcePaths.contains(currentPath)) {
                sourceDirCount++;   // 这是源目录，记录数量
            }

            currentResult.directoryCount++;
            currentResult.progressSize += memoryPageSize;   // 估算目录大小

            // 单深度模式：跳过子目录
            if (options & FileScanner::ScanOption::SingleDepth) {
                fts_set(ftsp, p, FTS_SKIP);
            }
            break;
        }

        case FTS_DP:   // 目录（退出后）
            // 已在 FTS_D 处理
            break;

        case FTS_F:   // 常规文件
        {
            // 跳过特殊系统文件（会导致问题的超大虚拟文件）
            static const QSet<QString> kSpecialSystemFiles {
                "/proc/kcore",
                "/dev/core"
            };

            QString currentPath = QString::fromUtf8(p->fts_path);
            if (kSpecialSystemFiles.contains(currentPath)) {
                qCDebug(logDFMBase) << "ScannerWorker: Skipping special system file:" << currentPath;
                break;
            }

            // 优化：只对 st_nlink > 1 的文件进行去重
            if (p->fts_statp->st_nlink > 1) {
                if (!isInodeProcessed(p->fts_statp->st_dev, p->fts_statp->st_ino)) {
                    markInodeProcessed(p->fts_statp->st_dev, p->fts_statp->st_ino);
                    currentResult.totalSize += p->fts_statp->st_size;
                    currentResult.fileCount++;
                } else {
                    // 硬链接已统计，只计数
                    currentResult.fileCount++;
                }
            } else {
                // st_nlink == 1，直接处理
                currentResult.totalSize += p->fts_statp->st_size;
                currentResult.fileCount++;
            }
            currentResult.progressSize += p->fts_statp->st_size;
            break;
        }

        case FTS_SL:   // 符号链接
        case FTS_SLNONE:   // 损坏的符号链接
        {
            // 符号链接只计数，不跟随（不计入大小）
            currentResult.fileCount++;
            break;
        }

        case FTS_DNR:   // 无法读取的目录
        case FTS_ERR:   // 错误
        case FTS_NS:   // stat 失败
            qCWarning(logDFMBase) << "ScannerWorker: Error traversing:" << p->fts_path;
            break;

        default:
            break;
        }

        // 定期发送进度
        emitProgress();
    }

    fts_close(ftsp);

    // QByteArray 自动释放内存，无需手动 free

    // 默认排除源目录本身（除非设置了 IncludeSource 选项）
    if (!(options & FileScanner::ScanOption::IncludeSource)) {
        currentResult.directoryCount -= sourceDirCount;
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

            // 单深度模式：不递归
            if (options & FileScanner::ScanOption::SingleDepth) {
                continue;
            }

            // 创建目录迭代器
            AbstractDirIteratorPointer iterator = DirIteratorFactory::create<AbstractDirIterator>(
                    url,
                    QStringList(),
                    QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

            if (!iterator) {
                qCWarning(logDFMBase) << "ScannerWorker: Failed to create iterator for:" << url;
                continue;
            }

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
                            directoryQueue.enqueue(childUrl);
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

}   // namespace dfmbase
