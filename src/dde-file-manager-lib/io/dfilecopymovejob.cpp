    /*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dfilecopymovejob.h"
#include "private/dfilecopymovejob_p.h"

#include "dfileservices.h"
#include "dabstractfileinfo.h"
#include "dfiledevice.h"
#include "dfilehandler.h"
#include "ddiriterator.h"
#include "dfilestatisticsjob.h"
#include "dlocalfiledevice.h"
#include "models/trashfileinfo.h"
#include "controllers/vaultcontroller.h"
#include "controllers/masteredmediacontroller.h"
#include "controllers/avfsfilecontroller.h"
#include "interfaces/dfmstandardpaths.h"
#include "shutil/fileutils.h"
#include "dgiofiledevice.h"
#include "deviceinfo/udisklistener.h"
#include "app/define.h"
#include "dialogs/dialogmanager.h"

#include <QMutex>
#include <QTimer>
#include <QLoggingCategory>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include <qplatformdefs.h>

#include <unistd.h>
#include <zlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fts.h>

#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public

DFM_BEGIN_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(fileJob, "file.job")
#else
Q_LOGGING_CATEGORY(fileJob, "file.job", QtInfoMsg)
#endif
//<syscall.h> == <sys/syscall.h>
#if defined(Q_OS_LINUX) && (defined(__GLIBC__) || QT_HAS_INCLUDE(<syscall.h>))
#  include <syscall.h>

# if defined(Q_OS_ANDROID) && !defined(SYS_gettid)
#  define SYS_gettid __NR_gettid
# endif


#define MAX_BUFFER_LEN 1024 * 1024 * 1
#define BIG_FILE_SIZE 500 * 1024 * 1024
#define THREAD_SLEEP_TIME 200
QQueue<DFileCopyMoveJob*> DFileCopyMoveJobPrivate::CopyLargeFileOnDiskQueue;
QMutex DFileCopyMoveJobPrivate::CopyLargeFileOnDiskMutex;

static long qt_gettid()
{
    // no error handling
    // this syscall has existed since Linux 2.4.11 and cannot fail
    return syscall(SYS_gettid);
}
#elif defined(Q_OS_DARWIN)
#  include <pthread.h>
static int qt_gettid()
{
    // no error handling: this call cannot fail
    __uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return tid;
}
#elif defined(Q_OS_FREEBSD_KERNEL) && defined(__FreeBSD_version) && __FreeBSD_version >= 900031
#  include <pthread_np.h>
static int qt_gettid()
{
    return pthread_getthreadid_np();
}
#else
static QT_PREPEND_NAMESPACE(qint64) qt_gettid()
{
    QT_USE_NAMESPACE
    return qintptr(QThread::currentThreadId());
}
#endif

static QByteArray fileReadAll(const QString &file_path)
{
    QFile file(file_path);

    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    }

    return QByteArray();
}

class ElapsedTimer
{
public:
    inline void start()
    {
        invalidElapsed = 0;
        elapsedOfPause = -1;
        timer.start();
    }

    void togglePause()
    {
        if (!timer.isValid()) {
            return;
        }

        if (elapsedOfPause < 0) {
            elapsedOfPause = timer.elapsed();
        } else {
            invalidElapsed += (timer.elapsed() - elapsedOfPause);
            elapsedOfPause = -1;
        }
    }

    inline bool isPaused() const
    {
        return elapsedOfPause > 0;
    }

    inline bool isRunning() const
    {
        return timer.isValid();
    }

    inline qint64 elapsed() const
    {
        return timer.elapsed() - invalidElapsed;
    }

    inline qint64 totalElapsed() const
    {
        return timer.elapsed();
    }

private:
    qint64 elapsedOfPause = -1;
    qint64 invalidElapsed = 0;

    QElapsedTimer timer;
};

DFileCopyMoveJobPrivate::DFileCopyMoveJobPrivate(DFileCopyMoveJob *qq)
    : q_ptr(qq)
    , updateSpeedElapsedTimer(new ElapsedTimer())
{
    m_pool.setMaxThreadCount(FileUtils::getCpuProcessCount());
}

DFileCopyMoveJobPrivate::~DFileCopyMoveJobPrivate()
{
    if (updateSpeedElapsedTimer) {
        delete updateSpeedElapsedTimer;
        updateSpeedElapsedTimer = nullptr;
    }
    if (fileStatistics) {
        fileStatistics->stop();
        fileStatistics->deleteLater();
        fileStatistics = nullptr;
    }
    if (updateSpeedTimer) {
        QMetaObject::invokeMethod(updateSpeedTimer, "stop");
        updateSpeedTimer->deleteLater();
        updateSpeedTimer = nullptr;
    }
    stopAllDeviceOperation();
}

QString DFileCopyMoveJobPrivate::errorToString(DFileCopyMoveJob::Error error)
{
    switch (error) {
    case DFileCopyMoveJob::PermissionError:
        return qApp->translate("DFileCopyMoveJob", "Permission error");
    case DFileCopyMoveJob::SpecialFileError:
        return qApp->translate("DFileCopyMoveJob", "The action is denied");
    case DFileCopyMoveJob::FileExistsError:
        return "Target file is exists";
    case DFileCopyMoveJob::DirectoryExistsError:
        return "Target directory is exists";
    case DFileCopyMoveJob::OpenError:
        return qApp->translate("DFileCopyMoveJob", "Failed to open the file");
    case DFileCopyMoveJob::ReadError:
        return qApp->translate("DFileCopyMoveJob", "Failed to read the file");
    case DFileCopyMoveJob::WriteError:
        return qApp->translate("DFileCopyMoveJob", "Failed to write the file");
    case DFileCopyMoveJob::MkdirError:
        return qApp->translate("DFileCopyMoveJob", "Failed to create the directory");
    case DFileCopyMoveJob::RemoveError:
        return qApp->translate("DFileCopyMoveJob", "Failed to delete the file");
    case DFileCopyMoveJob::RenameError:
        return qApp->translate("DFileCopyMoveJob", "Failed to move the file");
    case DFileCopyMoveJob::NonexistenceError:
        return qApp->translate("DFileCopyMoveJob", "Original file does not exist");
    case DFileCopyMoveJob::FileSizeTooBigError:
        return qApp->translate("DFileCopyMoveJob", "Failed, file size must be less than 4GB");
    case DFileCopyMoveJob::NotEnoughSpaceError:
        return qApp->translate("DFileCopyMoveJob", "Not enough free space on the target disk");
    case DFileCopyMoveJob::IntegrityCheckingError:
        return qApp->translate("DFileCopyMoveJob", "File integrity was damaged");
    case DFileCopyMoveJob::TargetReadOnlyError:
        return qApp->translate("DFileCopyMoveJob", "The target device is read only");
    case DFileCopyMoveJob::TargetIsSelfError:
        return qApp->translate("DFileCopyMoveJob", "Target folder is inside the source folder");
    case DFileCopyMoveJob::NotSupportedError:
        return qApp->translate("DFileCopyMoveJob", "The action is not supported");
    case DFileCopyMoveJob::PermissionDeniedError:
        return qApp->translate("DFileCopyMoveJob", "You do not have permission to traverse files in it");
    case DFileCopyMoveJob::SeekError:
        return qApp->translate("DFileCopyMoveJob", "Failed to position the file pointer!");
    default:
        break;
    }

    return QString();
}

qint64 DFileCopyMoveJobPrivate::getWriteBytes(long tid)
{
    QFile file(QStringLiteral("/proc/self/task/%1/io").arg(tid));

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed on open the" << file.fileName() << ", will be not update the job speed and progress";

        return 0;
    }

    const QByteArray &line_head = QByteArrayLiteral("write_bytes: ");
    const QByteArray &all_data = file.readAll();

    file.close();

    QTextStream text_stream(all_data);

    while (!text_stream.atEnd()) {
        const QByteArray &line = text_stream.readLine().toLatin1();

        if (line.startsWith(line_head)) {
            bool ok = false;
            qint64 size = line.mid(line_head.size()).toLongLong(&ok);

            if (!ok) {
                qWarning() << "Failed to convert to qint64, line string=" << line;

                return 0;
            }

//            qCDebug(fileJob(), "Did Write size on block device: %lld", size);

            return size;
        }
    }

    qWarning() << "Failed to find \"" << line_head << "\" from the" << file.fileName();

    return 0;
}

qint64 DFileCopyMoveJobPrivate::getWriteBytes() const
{
    return getWriteBytes(tid);
}

qint64 DFileCopyMoveJobPrivate::getSectorsWritten() const
{
    const QByteArray data = fileReadAll(targetSysDevPath + "/stat");
    //fix: 删除文件时出现报错(回收箱和光驱处理删除文件)，获取data为空指针
    if (data == nullptr) {
        return 0;
    } else {
        return data.simplified().split(' ').value(6).toLongLong();
    }
}

qint64 DFileCopyMoveJobPrivate::getCompletedDataSize() const
{
    if (canUseWriteBytes) {
        return getWriteBytes();
    }

    if (targetDeviceStartSectorsWritten >= 0) {
        if ((getSectorsWritten() == 0) && (targetDeviceStartSectorsWritten > 0)) {
            return 0;
        } else {
            return (getSectorsWritten() - targetDeviceStartSectorsWritten) * targetLogSecionSize;
        }
    }

    return completedDataSize;
}

void DFileCopyMoveJobPrivate::setState(DFileCopyMoveJob::State s)
{
    if (state == s) {
        return;
    }

    state = s;

    Q_Q(DFileCopyMoveJob);

    if (updateSpeedTimer->thread()->loopLevel() <= 0) {
        qWarning() << "The thread of update speed timer no event loop" << updateSpeedTimer->thread();
    }

    if (s == DFileCopyMoveJob::RunningState) {
        if (updateSpeedElapsedTimer->isRunning()) {
            if (updateSpeedElapsedTimer->isPaused())
                updateSpeedElapsedTimer->togglePause();
        } else {
            updateSpeedElapsedTimer->start();
        }
        _q_updateProgress();
        QMetaObject::invokeMethod(updateSpeedTimer, "start", Q_ARG(int, 500));
    } else if (s == DFileCopyMoveJob::StoppedState) {
        cansetnoerror = true;
    } else if (s != DFileCopyMoveJob::IOWaitState) {
        updateSpeedElapsedTimer->togglePause();

        QMetaObject::invokeMethod(updateSpeedTimer, "stop");
    }

    Q_EMIT q->stateChanged(s);

    qCDebug(fileJob()) << "state changed, new state:" << s;
}

void DFileCopyMoveJobPrivate::setError(DFileCopyMoveJob::Error e, const QString &es)
{
    if (DFileCopyMoveJob::CancelError <= error && error == e) {
        return;
    }

    if (DFileCopyMoveJob::CancelError < error && !cansetnoerror && DFileCopyMoveJob::StoppedState != state) {
        QThread::msleep(10);
    }
    if (DFileCopyMoveJob::CancelError < e) {
        cansetnoerror = false;
    } else {
        cansetnoerror = true;
    }

    error = e;
    errorString = es.isEmpty() ? errorToString(e) : es;

    Q_Q(DFileCopyMoveJob);

    if (actionOfError[error] == DFileCopyMoveJob::NoAction) {
        Q_EMIT q->errorChanged(e);
    }
    if (DFileCopyMoveJob::CancelError < e) {
        qCDebug(fileJob()) << "new error, type=" << e << ", message=" << es << QThread::currentThreadId();
    }
}

void DFileCopyMoveJobPrivate::unsetError()
{
    setError(DFileCopyMoveJob::NoError);
}

DFileCopyMoveJob::Action DFileCopyMoveJobPrivate::handleError(const DAbstractFileInfoPointer sourceInfo,
                                                               const DAbstractFileInfoPointer targetInfo)
{
    //当任务对话框结束返回cancel
    auto lastErrorHandleAction = DFileCopyMoveJob::NoAction;
    if (error == DFileCopyMoveJob::NoError) {
        cansetnoerror = true;
        emit q_ptr->errorCanClear();
        return lastErrorHandleAction;
    }
    if (m_bTaskDailogClose) {
        return DFileCopyMoveJob::CancelAction;
    }
    if (actionOfError[error] != DFileCopyMoveJob::NoAction) {
        lastErrorHandleAction = actionOfError[error];
        cansetnoerror = true;
        unsetError();
        qCDebug(fileJob()) << "from actionOfError list," << "action:" << lastErrorHandleAction
                           << "source url:" << sourceInfo->fileUrl()
                           << "target url:" << (targetInfo ? targetInfo->fileUrl() : DUrl());

        // not update speed onerror
        if (updateSpeedTimer->isActive()) {
            //updateSpeedTimer->stop();
//            QMetaObject::invokeMethod(updateSpeedTimer, "stop");
        }
        updateProgress();

        return lastErrorHandleAction;
    }

    if (!handle) {
        switch (error) {
        case DFileCopyMoveJob::PermissionError:
        case DFileCopyMoveJob::UnknowUrlError:
        case DFileCopyMoveJob::TargetIsSelfError:
            lastErrorHandleAction = DFileCopyMoveJob::SkipAction;
            cansetnoerror = true;
            emit q_ptr->errorCanClear();
            unsetError();
            break;
        case DFileCopyMoveJob::FileExistsError:
        case DFileCopyMoveJob::DirectoryExistsError:
            lastErrorHandleAction = DFileCopyMoveJob::CoexistAction;
            cansetnoerror = true;
            emit q_ptr->errorCanClear();
            unsetError();
            break;
        default:
            lastErrorHandleAction = DFileCopyMoveJob::CancelAction;
            cansetnoerror = true;
            emit q_ptr->errorCanClear();
            setError(DFileCopyMoveJob::CancelError);
            break;
        }

        qCDebug(fileJob()) << "no handle," << "default action:" << lastErrorHandleAction
                           << "source url:" << (sourceInfo ? sourceInfo->fileUrl() : DUrl())
                           << "target url:" << (targetInfo ? targetInfo->fileUrl() : DUrl());

        return lastErrorHandleAction;
    }

    setState(DFileCopyMoveJob::SleepState);
    //暂停所有线程池的线程
    do {

        if (threadOfErrorHandle && threadOfErrorHandle->loopLevel() > 0) {
            lastErrorHandleAction = DThreadUtil::runInThread(threadOfErrorHandle, handle, &DFileCopyMoveJob::Handle::handleError,
                                                             q_ptr, error, sourceInfo, targetInfo);
        } else {
            lastErrorHandleAction = handle->handleError(q_ptr, error, sourceInfo, targetInfo);
        }
        if (!stateCheck()) {
            lastErrorHandleAction = DFileCopyMoveJob::CancelAction;
            break;
        }
    } while (lastErrorHandleAction == DFileCopyMoveJob::NoAction);
    emit q_ptr->errorCanClear();
    qCDebug(fileJob()) << "from user," << "action:" << lastErrorHandleAction;
    cansetnoerror = true;

    if (state == DFileCopyMoveJob::SleepState) {
        setState(DFileCopyMoveJob::RunningState);
    }

    unsetError();

    if (lastErrorHandleAction == DFileCopyMoveJob::CancelAction) {
        setError(DFileCopyMoveJob::CancelError);
    }

    qCDebug(fileJob()) << "from user," << "action:" << lastErrorHandleAction
                       << "source url:" << (sourceInfo ? sourceInfo->fileUrl() : DUrl())
                       << "target url:" << (targetInfo ? targetInfo->fileUrl() : DUrl());

    return lastErrorHandleAction;
}

DFileCopyMoveJob::Action DFileCopyMoveJobPrivate::setAndhandleError(DFileCopyMoveJob::Error e,
                                                                    const DAbstractFileInfoPointer sourceInfo,
                                                                    const DAbstractFileInfoPointer targetInfo,
                                                                    const QString &es)
{
    if (!stateCheck()) {
        setLastErrorAction(DFileCopyMoveJob::CancelAction);
        return DFileCopyMoveJob::CancelAction;
    }
    if ((DFileCopyMoveJob::FileExistsError == e || DFileCopyMoveJob::DirectoryExistsError == e)
            && (sourceInfo->fileUrl() == targetInfo->fileUrl() || DStorageInfo::isSameFile(sourceInfo->fileUrl().path(), targetInfo->fileUrl().path()))) {
        setLastErrorAction(DFileCopyMoveJob::CoexistAction);
        return DFileCopyMoveJob::CoexistAction;
    }
    setError(e, es);
    if (DFileCopyMoveJob::NoError == e) {
        setLastErrorAction(DFileCopyMoveJob::NoAction);
        return DFileCopyMoveJob::NoAction;
    }
    if (DFileCopyMoveJob::CancelError == e) {
        setLastErrorAction(DFileCopyMoveJob::CancelAction);
        return DFileCopyMoveJob::CancelAction;
    }
    auto action = handleError(sourceInfo, targetInfo);
    setLastErrorAction(action);
    return action;
}

#define TASK_RUNNING_MAX_COUNT 5

bool DFileCopyMoveJobPrivate::isRunning()
{
    return (this->state == DFileCopyMoveJob::RunningState);//&& (m_taskRunningCount <= TASK_RUNNING_MAX_COUNT ) ;
}

bool DFileCopyMoveJobPrivate::jobWait()
{
    QMutex lock;

    lock.lock();
    waitCondition.wait(&lock);
    lock.unlock();

    return state == DFileCopyMoveJob::RunningState;
}

bool DFileCopyMoveJobPrivate::stateCheck()
{
    Q_Q(DFileCopyMoveJob);
    if (state == DFileCopyMoveJob::RunningState) {
        if (needUpdateProgress) {
            needUpdateProgress = false;

            updateProgress();
            updateSpeed();
        }

        return true;
    }

    if (state == DFileCopyMoveJob::PausedState) {
        qInfo() << "Will be suspended";
        Q_EMIT q->stateChanged(DFileCopyMoveJob::PausedState);
        if (!jobWait()) {
            QMutexLocker lk(&m_checkStatMutex);
            qInfo() << "Will be abort = " << (state != DFileCopyMoveJob::StoppedState);

            return state != DFileCopyMoveJob::StoppedState;
        }
    } else if (state == DFileCopyMoveJob::StoppedState) {
        QMutexLocker lk(&m_checkStatMutex);
        cansetnoerror = true;
        setError(DFileCopyMoveJob::CancelError);
        qCDebug(fileJob()) << "Will be abort";

        //! re-calculate vault size.
        bool isVaultFile = VaultController::isVaultFile(targetUrl.toLocalFile());
        DUrlList::iterator it = sourceUrlList.begin();
        while (!isVaultFile && it != sourceUrlList.end()) {
            isVaultFile = VaultController::isVaultFile(it->toLocalFile());
            it++;
        }
        if (isVaultFile) {
            QMetaObject::invokeMethod(VaultController::ins(), "refreshTotalSize", Qt::QueuedConnection);
        }

        return false;
    }

    return true;
}

bool DFileCopyMoveJobPrivate::checkFileSize(qint64 size) const
{
    if (directoryStack.count() <= 0) {
        return true;
    }
    const DStorageInfo &targetStorageInfo = directoryStack.top().targetStorageInfo;

    if (!targetStorageInfo.isValid()) {
        return true;
    }

    const QString &fs_type = targetStorageInfo.fileSystemType();

    // for vfat file system
    if (fs_type == "vfat") {
        // 4GB
        if (size >= 4l * 1024 * 1024 * 1024) {
            return false;
        }
    }

    return true;
}

bool DFileCopyMoveJobPrivate::checkFreeSpace(qint64 needSize)
{
    if (directoryStack.isEmpty()) {
        qWarning() << "directoryStack.isEmpty() return true";
        return true;
    }
    DStorageInfo &targetStorageInfo = directoryStack.top().targetStorageInfo;

    if (!targetStorageInfo.isValid()) {
        return true;
    }

    targetStorageInfo.refresh();

    // invalid size info
    if (targetStorageInfo.bytesTotal() <= 0) {
        return true;
    }

    //fix:修正文件发送到光盘时会报磁盘空间不足的问题，这里实现分类处理（常规可挂载移动存储设备和光驱设备）
    const QString &fs_type = targetStorageInfo.fileSystemType();

    if (fs_type == "iso9660") {
        return true;
    } else {
        return targetStorageInfo.bytesAvailable() >= needSize;
    }
}

QString DFileCopyMoveJobPrivate::formatFileName(const QString &name) const
{
    if (fileHints.testFlag(DFileCopyMoveJob::DontFormatFileName)) {
        return name;
    }
    if (directoryStack.count() <= 0) {
        return name;
    }
    const DStorageInfo &targetStorageInfo = directoryStack.top().targetStorageInfo;

    if (!targetStorageInfo.isValid()) {
        return name;
    }

    const QString &fs_type = targetStorageInfo.fileSystemType();

    if (fs_type == "vfat") {
        QString new_name = name;

        return new_name.replace(QRegExp("[\"*:<>?\\|]"), "_");
    }

    return name;
}

QString DFileCopyMoveJobPrivate::getNewFileName(const DAbstractFileInfoPointer sourceFileInfo, const DAbstractFileInfoPointer targetDirectory)
{
    const QString &copy_text = QCoreApplication::translate("DFileCopyMoveJob", "copy",
                                                           "Extra name added to new file name when used for file name.");

    DAbstractFileInfoPointer target_file_info;
    QString file_base_name = sourceFileInfo->baseName();
    QString suffix = sourceFileInfo->suffix();
    QString filename = sourceFileInfo->fileName();
    //在7z分卷压缩后的名称特殊处理7z.003
    if (filename.contains(QRegularExpression(".7z.[0-9]{3,10}$"))) {
        file_base_name = filename.left(filename.indexOf(QRegularExpression(".7z.[0-9]{3,10}$")));
        suffix = filename.mid(filename.indexOf(QRegularExpression(".7z.[0-9]{3,10}$")) + 1);
    }

    int number = 0;

    QString new_file_name;

    do {
        new_file_name = number > 0 ? QString("%1(%2 %3)").arg(file_base_name, copy_text).arg(number) : QString("%1(%2)").arg(file_base_name, copy_text);

        if (!suffix.isEmpty()) {
            new_file_name.append('.').append(suffix);
        }

        ++number;
        target_file_info = DFileService::instance()->createFileInfo(nullptr, targetDirectory->getUrlByChildFileName(new_file_name), false);
    } while (target_file_info->exists());

    return new_file_name;
}

bool DFileCopyMoveJobPrivate::doProcess(const DUrl &from, const DAbstractFileInfoPointer source_info, const DAbstractFileInfoPointer target_info, const bool isNew)
{
//    Q_Q(DFileCopyMoveJob);
    bool isErrorOccur = false;

    if (!source_info) {
        //错误队列处理
        errorQueueHandling();
        bool ok = setAndhandleError(DFileCopyMoveJob::UnknowUrlError, source_info, DAbstractFileInfoPointer(nullptr),
                                    QObject::tr("Failed to create file info")) == DFileCopyMoveJob::SkipAction;
        //当前错误处理完成
        errorQueueHandled(ok);
        return ok;
    }

    if (!source_info->exists()) {
        DFileCopyMoveJob::Error errortype = (source_info->path().startsWith("/root/") && !target_info->path().startsWith("/root/")) ?
                                            DFileCopyMoveJob::PermissionError : DFileCopyMoveJob::NonexistenceError;
        errortype = source_info->path().startsWith(MOBILE_ROOT_PATH) ? DFileCopyMoveJob::NotSupportedError : errortype;
        //错误队列处理
        errorQueueHandling();
        bool ok = setAndhandleError(errortype, source_info,
                                    DAbstractFileInfoPointer(nullptr)) == DFileCopyMoveJob::SkipAction;
        //当前错误处理完成
        errorQueueHandled(ok);
        return ok;
    }

    switch (source_info->fileType()) {
    case DAbstractFileInfo::CharDevice:
    case DAbstractFileInfo::BlockDevice:
    case DAbstractFileInfo::FIFOFile:
    case DAbstractFileInfo::SocketFile: {
        isErrorOccur = true;
        //错误队列处理
        errorQueueHandling();
        bool ok = setAndhandleError(DFileCopyMoveJob::SpecialFileError, source_info,
                                    DAbstractFileInfoPointer(nullptr)) == DFileCopyMoveJob::SkipAction;
        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled(ok);
            isErrorOccur = false;
        }
        if (ok) {
            //跳过文件大小统计
            if (source_info->isSymLink()) {
                skipFileSize += FileUtils::getMemoryPageSize();
            } else if (source_info->isDir()) {
                skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
            } else {
                skipFileSize += source_info->size() <= 0 ? FileUtils::getMemoryPageSize() : source_info->size();
            }
        }
        return ok;
    }
    default:
        break;
    }

    QSharedPointer<DFileHandler> handler(DFileService::instance()->createFileHandler(nullptr, from));

    if (!handler) {
        //错误队列处理
        errorQueueHandling();
        bool ok = setAndhandleError(DFileCopyMoveJob::UnknowUrlError, source_info,
                                    DAbstractFileInfoPointer(nullptr), QObject::tr("Failed to create file handler")) == DFileCopyMoveJob::SkipAction;
        //当前错误处理完成
        errorQueueHandled(ok);
        if (ok) {
            //跳过文件大小统计
            if (source_info->isSymLink()) {
                skipFileSize += FileUtils::getMemoryPageSize();
            } else if (source_info->isDir()) {
                skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
            } else {
                skipFileSize += source_info->size() <= 0 ? FileUtils::getMemoryPageSize() : source_info->size();
            }
        }
        return ok;
    }

    // only remove
    if (!target_info) {
        auto fileInfo = source_info;
        if (source_info->fileUrl().parentUrl() == DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath)))
            convertTrashFile(fileInfo);

        // BUG 59250 回收站文件夹下删除子文件会受到上级目录的权限管控
        DUrl parentUrl;
        QFile::Permissions parentUrlPermissions;
        // 权限更改标识
        bool permissionChangedFlag = false;

        // 确定当前是从回收站强制删除且没有写权限(trashRemoveFlag 流控只针对回收站)
        if (fileHints.testFlag(DFileCopyMoveJob::ForceDeleteFile)
                && fileInfo->path().contains("/.local/share/Trash")) {
            // 获取上级目录路径，获取缓存权限，判断当前是否进行权限更改
            parentUrl = fileInfo->fileUrl().parentUrl();

            //获取上层目录的Info对象
            DAbstractFileInfoPointer parentInfo = DFileService::instance()->createFileInfo(nullptr, parentUrl, false);
            if (parentInfo) {
                parentUrlPermissions = parentInfo->permissions();
            } else {
                isErrorOccur = true;
                //错误队列处理
                errorQueueHandling();
                bool ok = setAndhandleError(DFileCopyMoveJob::UnknowUrlError,
                                            parentInfo,DAbstractFileInfoPointer(nullptr),
                                            QObject::tr("Failed to create file info")) == DFileCopyMoveJob::SkipAction;
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled(ok);
                    isErrorOccur = false;
                }
                return ok;
            }

            //没有写权限则不可操作子目录
            permissionChangedFlag = !parentUrlPermissions.testFlag(QFileDevice::WriteUser);
            // 触发当前权限更改可写入的流程操作
            if (permissionChangedFlag) {
                // 设置上级目录加入当前用户可写
                handler->setPermissions(parentUrl, parentUrlPermissions | QFileDevice::WriteUser);
            }
        }


        bool ok = false;
        //可以显示进度条
        m_isNeedShowProgress = true;
        qint64 size = fileInfo->isSymLink() ? 0 : fileInfo->size();

        if (fileInfo->isFile() || fileInfo->isSymLink()) {
            ok = removeFile(handler, fileInfo);
            if (ok) {
                joinToCompletedFileList(from, DUrl(), size);
            }
        } else {
            // 删除文件夹时先设置其权限
            if (fileHints.testFlag(DFileCopyMoveJob::ForceDeleteFile)) {
                handler->setPermissions(fileInfo->fileUrl(), QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser);
            }

            ok = mergeDirectory(handler, fileInfo, DAbstractFileInfoPointer(nullptr));
            if (ok) {
                joinToCompletedDirectoryList(from, DUrl(), size);
            }
        }

        //上级目录权限回设
        if (permissionChangedFlag) {
            handler->setPermissions(parentUrl,parentUrlPermissions);
        }

        return ok;
    }

    QString file_name;
    //! 是否退回到回收站
    if (m_fileNameList.isEmpty()) {
        file_name = isNew ? source_info->fileName() :
                    (handle ? handle->getNewFileName(q_ptr, source_info) : source_info->fileName());
    } else {
        file_name = m_fileNameList.dequeue().split('/').last();
    }

    // 回收站可能重名文件，因此回收站中的文件实际filename是经过处理的,这里需要取真正需要展示的filename
    if (source_info->filePath().startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))) {
        QExplicitlySharedDataPointer<TrashFileInfo> info(new TrashFileInfo(DUrl::fromTrashFile("/" + source_info->fileName())));

        // fix bug45213 从回收站复制/剪切2个计算机图标到桌面，计算机图标变成普通文件
        QFileInfo actual_info(info->sourceFilePath());
        if (!actual_info.isSymLink() && FileUtils::isDesktopFile(actual_info)) {
            //目标文件是桌面程序文件时，需要使用原名字作为文件名来创建文件(createFileInfo)，否则将导致新创建的文件不是桌面程序文件
            file_name = actual_info.fileName();
        } else {
            file_name = info->fileDisplayName();
        }
    }
create_new_file_info:
    const DAbstractFileInfoPointer &new_file_info = DFileService::instance()->createFileInfo(nullptr, target_info->getUrlByChildFileName(file_name), false);

    if (!new_file_info) {
        qWarning() << "fileinfo create failed!" << target_info->getUrlByChildFileName(file_name);
        return false;
    }

    if (new_file_info->exists()) {
        //忽略DStorageInfo::isSameFile判断链接文件的结果
        if ((mode == DFileCopyMoveJob::MoveMode || mode == DFileCopyMoveJob::CutMode) &&
                (new_file_info->fileUrl() == from || (DStorageInfo::isSameFile(from.path(), new_file_info->fileUrl().path()) && !new_file_info->isSymLink()))) {
            // 不用再进行后面的操作
            //跳过文件大小统计
            if (source_info->isSymLink()) {
                skipFileSize += FileUtils::getMemoryPageSize();
            } else if (source_info->isDir()) {
                skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
            } else {
                skipFileSize += source_info->size() <= 0 ? FileUtils::getMemoryPageSize() : source_info->size();
            }
            return true;
        }
        //可以显示进度条
        m_isNeedShowProgress = true;

        // 禁止目录复制/移动到自己里面
        if (new_file_info->isAncestorsUrl(source_info->fileUrl())) {
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::TargetIsSelfError,
                                                                source_info, new_file_info);
            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                                  action == DFileCopyMoveJob::EnforceAction);
                isErrorOccur = false;
            }
            if (action == DFileCopyMoveJob::SkipAction) {
                //跳过文件大小统计
                if (source_info->isSymLink()) {
                    skipFileSize += FileUtils::getMemoryPageSize();
                } else if (source_info->isDir()) {
                    skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
                } else {
                    skipFileSize += source_info->size() <= 0 ? FileUtils::getMemoryPageSize() : source_info->size();
                }
                return true;
            }

            if (action != DFileCopyMoveJob::EnforceAction) {
                return false;
            }
        }

        bool source_is_file = source_info->isFile() || source_info->isSymLink();
        bool target_is_file = new_file_info->isFile() || new_file_info->isSymLink();
        //如果目标目录有相同名称的文件，但是拷贝的是目录，或者相反，就直接创建一个新的名称
        if (source_is_file != target_is_file) {
            file_name = handle ? handle->getNonExistsFileName(source_info, target_info)
                        : getNewFileName(source_info, target_info);
            goto create_new_file_info;
        }
        DFileCopyMoveJob::Error errortype =  target_is_file ?
                                             DFileCopyMoveJob::FileExistsError : DFileCopyMoveJob::DirectoryExistsError;
        isErrorOccur = true;
        //错误队列处理
        errorQueueHandling();
        switch (setAndhandleError(errortype, source_info, new_file_info)) {
        case DFileCopyMoveJob::ReplaceAction:
            if (new_file_info->fileUrl() == from) {
                // 不用再进行后面的操作
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled();
                    isErrorOccur = false;
                }
                return true;
            }

            if (source_is_file && source_is_file == target_is_file) {
                break;
            } else {
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled(false);
                    isErrorOccur = false;
                }
                return false;
            }
        case DFileCopyMoveJob::MergeAction:
            if (!source_is_file && source_is_file == target_is_file) {
                break;
            } else {
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled(false);
                    isErrorOccur = false;
                }
                return false;
            }
        case DFileCopyMoveJob::SkipAction:
            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled();
                isErrorOccur = false;
            }
            //跳过文件大小统计
            if (source_info->isSymLink()) {
                skipFileSize += FileUtils::getMemoryPageSize();
            } else if (source_info->isDir()) {
                skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
            } else {
                skipFileSize += source_info->size() <= 0 ? FileUtils::getMemoryPageSize() : source_info->size();
            }
            return true;
        case DFileCopyMoveJob::CoexistAction:
            // fix bug 62226
            // 回收站的同名文件是以uuid形式存在的，如果这里使用source_info去构建副本的话，就会造成
            // file_name的值为uuid（副本）的情况，因此修改为new_file_info，getNonExistsFileName
            // 接口中只会使用到new_file_info的filename相关属性，因此对非回收站的文件也不会存在影响
            file_name = handle ? handle->getNonExistsFileName(new_file_info, target_info)
                        : getNewFileName(new_file_info, target_info);
            goto create_new_file_info;
        default:
            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled(false);
                isErrorOccur = false;
            }
            return false;
        }
        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled();
            isErrorOccur = false;
        }
    }

    //当前错误处理完成
    if (isErrorOccur) {
        errorQueueHandled();
        isErrorOccur = false;
    }

    m_isNeedShowProgress = true;

    if (source_info->isSymLink()) {
        bool ok = false;

        if (mode == DFileCopyMoveJob::CopyMode) {
            DAbstractFileInfoPointer new_source_info = source_info;
            if (fileHints.testFlag(DFileCopyMoveJob::FollowSymlink)) {
                do {
                    const DAbstractFileInfoPointer &symlink_target = DFileService::instance()->createFileInfo(nullptr, source_info->symLinkTarget(), false);

                    if (!symlink_target->exists()) {
                        break;
                    }

                    new_source_info = symlink_target;
                } while (new_source_info->isSymLink());

                if (new_source_info->exists()) {
                    goto process_file;
                }
            }

            ok = linkFile(handler, new_file_info, new_source_info->symlinkTargetPath());
        } else {
            ok = renameFile(handler, source_info, new_file_info);
        }

        if (ok) {
            joinToCompletedFileList(from, new_file_info->fileUrl(), 0);
        }

        countrefinesize(FileUtils::getMemoryPageSize());

        return ok;
    }

process_file:
    if (source_info->isFile()) {
        bool ok = false;
        qint64 size = source_info->size();

        while (!checkFreeSpace(size)) {
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::NotEnoughSpaceError,
                                                                source_info, new_file_info);
            if (action == DFileCopyMoveJob::RetryAction) {
                continue;
            }
            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                                  action == DFileCopyMoveJob::EnforceAction);
                isErrorOccur = false;
            }
            if (action == DFileCopyMoveJob::SkipAction) {
                //跳过文件大小统计
                if (source_info->isSymLink()) {
                    skipFileSize += FileUtils::getMemoryPageSize();
                } else if (source_info->isDir()) {
                    skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
                } else {
                    skipFileSize += source_info->size() <= 0 ? FileUtils::getMemoryPageSize() : source_info->size();
                }
                return true;
            }

            if (action == DFileCopyMoveJob::EnforceAction) {
                break;
            }

            return false;
        }

        if (!checkFileSize(size)) {
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::FileSizeTooBigError,
                                                                source_info, new_file_info);
            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                                  action == DFileCopyMoveJob::EnforceAction);
                isErrorOccur = false;
            }
            if (action == DFileCopyMoveJob::SkipAction) {
                //跳过文件大小统计
                if (source_info->isSymLink()) {
                    skipFileSize += FileUtils::getMemoryPageSize();
                } else if (source_info->isDir()) {
                    skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
                } else {
                    skipFileSize += source_info->size() <= 0 ? FileUtils::getMemoryPageSize() : source_info->size();
                }
                return true;
            }

            if (action != DFileCopyMoveJob::EnforceAction) {
                return false;
            }
        }

        if (mode == DFileCopyMoveJob::CopyMode) {
            if (new_file_info->isSymLink() || fileHints.testFlag(DFileCopyMoveJob::RemoveDestination)) {
                if (!removeFile(handler, new_file_info)) {
                    return false;
                }
            } else if (new_file_info->exists()) {
                // 复制文件时，如果需要覆盖，必须添加可写入权限
                handler->setPermissions(new_file_info->fileUrl(), QFileDevice::WriteUser | QFileDevice::ReadUser);
            }

            ok = copyFile(source_info, new_file_info, handler);
        } else {
            // 光盘中的文件不能进行写操作，因此复制它
            const QString &sourcePath = source_info->fileUrl().toLocalFile();
            if (deviceListener->isFileFromDisc(sourcePath)) {
                qInfo() << "canRename : " << source_info->canRename();
                ok = copyFile(source_info, new_file_info, handler);
            } else {
                ok = renameFile(handler, source_info, new_file_info);
            }
        }

        if (ok) {
            joinToCompletedFileList(from, new_file_info->fileUrl(), size);
        }

        return ok;
    } else if (source_info->isDir()) {
        // 禁止目录复制/移动到自己里面
        // fix bug 99308，转换网络smb挂载到本地路径，在判断是否目录复制/移动到自己里面
        bool isNewGvfsMountFile = new_file_info->isGvfsMountFile();
        bool isSourceGvfsMountFile = source_info->isGvfsMountFile();
        bool isAncestorsUrl = false;
        if ((isNewGvfsMountFile && !isSourceGvfsMountFile) ||
                (!isNewGvfsMountFile && isSourceGvfsMountFile)) {
            isAncestorsUrl = FileUtils::isNetworkAncestorUrl(new_file_info->fileUrl(), isNewGvfsMountFile,
                                                             source_info->fileUrl(), isSourceGvfsMountFile);
        }
        // 禁止目录复制/移动到自己里面
        if (new_file_info->isAncestorsUrl(source_info->fileUrl()) || isAncestorsUrl) {
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::TargetIsSelfError,
                                                                source_info, new_file_info);
            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                                  action == DFileCopyMoveJob::EnforceAction);
                isErrorOccur = false;
            }
            if (action == DFileCopyMoveJob::SkipAction) {
                skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
                return true;
            }

            if (action != DFileCopyMoveJob::EnforceAction) {
                return false;
            }
        }

        bool ok = true;
        qint64 size = source_info->size();
        const QDateTime si_last_read = source_info->lastRead();
        const QDateTime si_last_modified = source_info->lastModified();

        if (mode == DFileCopyMoveJob::CopyMode) {
            ok = mergeDirectory(handler, source_info, new_file_info);
        } else if (!handler->rename(source_info->fileUrl(), new_file_info->fileUrl())) { // 尝试直接rename操作
            qCDebug(fileJob(), "Failed on rename, Well be copy and delete the directory");
            ok = mergeDirectory(handler, source_info, new_file_info);
        }

        if (ok) {
            handler->setFileTime(new_file_info->fileUrl(), si_last_read, si_last_modified);
            joinToCompletedDirectoryList(from, new_file_info->fileUrl(), size);
        }

        return ok;
    }

    return false;
}

bool DFileCopyMoveJobPrivate::mergeDirectory(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo)
{
    bool isNew = false;
    bool isErrorOccur = false;
    if (toInfo && !toInfo->exists()) {
        DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

        do {
            // 当为保险箱路径时，判断目录名的长度，如果长度大于85，则不让其创建成功，并报错“文件名过长”
            QString strPath = toInfo->fileUrl().toString();
            if (VaultController::isVaultFile(strPath)) {
                // 获得目录名
                QString strDirName = strPath.section("/", -1, -1);
                if (strDirName.toUtf8().length() > 255) {
                    isErrorOccur = true;
                    //错误队列处理
                    errorQueueHandling();
                    action = setAndhandleError(DFileCopyMoveJob::MkdirError, fromInfo, toInfo,
                                               qApp->translate("DFileCopyMoveJob", "Failed to open the directory, cause: file name too long"));
                    break;
                }
            }

            if (!handler->mkdir(toInfo->fileUrl())) {
                const DAbstractFileInfoPointer &parent_info = DFileService::instance()->createFileInfo(nullptr, toInfo->parentUrl(), false);
                DFileCopyMoveJob::Error errortype = (!parent_info->exists() || parent_info->isWritable()) ?
                                                    DFileCopyMoveJob::MkdirError : DFileCopyMoveJob::PermissionError;
                QString errorstr = (!parent_info->exists() || parent_info->isWritable()) ?
                                   qApp->translate("DFileCopyMoveJob", "Failed to create the directory, cause: %1").arg(handler->errorString()) : QString();
                isErrorOccur = true;
                //错误队列处理
                errorQueueHandling();
                action = setAndhandleError(errortype, fromInfo, toInfo, errorstr);
            }
            isNew = true;
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());
        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                              action == DFileCopyMoveJob::NoAction);
            isErrorOccur = false;
        }
        if (action != DFileCopyMoveJob::NoAction) {
            if (action == DFileCopyMoveJob::SkipAction)
                skipFileSize += m_currentDirSize <= 0 ? FileUtils::getMemoryPageSize() : m_currentDirSize;
            return action == DFileCopyMoveJob::SkipAction;
        }
    }

    if (toInfo && fromInfo->filesCount() <= 0 && mode == DFileCopyMoveJob::CopyMode) {
        QFileDevice::Permissions permissions = fromInfo->permissions();
        QString filePath = fromInfo->fileUrl().toLocalFile();
        if (VaultController::ins()->isVaultFile(filePath)) {
            permissions = VaultController::ins()->getPermissions(filePath);
        } else if (deviceListener->isFileFromDisc(fromInfo->path())) {
            permissions |= MasteredMediaController::getPermissionsCopyToLocal();
        }

        //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
        if (permissions != 0000)
            handler->setPermissions(toInfo->fileUrl(), permissions);
        return true;
    }

    bool sortInode = toInfo && !fileHints.testFlag(DFileCopyMoveJob::DontSortInode);
    const DDirIteratorPointer &iterator = DFileService::instance()->createDirIterator(nullptr, fromInfo->fileUrl(), QStringList(),
                                                                                      QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
                                                                                      sortInode ? static_cast<QDirIterator::IteratorFlag>(DDirIterator::SortINode)
                                                                                      : QDirIterator::NoIteratorFlags, true);

    if (!iterator) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create dir iterator");
        return false;
    }

    bool existsSkipFile = false;
    bool enter_dir = toInfo;

    if (enter_dir) {
        enterDirectory(fromInfo->fileUrl(), toInfo->fileUrl());
    }

    //目录没有执行权限时不能正确的遍历到子文件的信息，后续删除或剪切复制逻辑无法成立
    //弹出错误弹窗，提示无权限
    if (!fromInfo->isExecutable() && iterator->hasNext()) {
        //错误队列处理
        errorQueueHandling();
        bool ok = setAndhandleError(DFileCopyMoveJob::PermissionDeniedError, fromInfo, DAbstractFileInfoPointer(nullptr)) == DFileCopyMoveJob::SkipAction;
        //当前错误处理完成
        errorQueueHandled(ok);
        return ok;
    }

    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const DUrl &url = iterator->next();
        const DAbstractFileInfoPointer &info = iterator->fileInfo();

        if (!process(url, info, toInfo, isNew)) {
            return false;
        }

        if (getLastErrorAction() == DFileCopyMoveJob::SkipAction) {
            existsSkipFile = true;
        }
    }

    if (enter_dir) {
        leaveDirectory();
    }

    if (toInfo) {
        // vault file fetch permissons separately.
        QFileDevice::Permissions permissions = fromInfo->permissions();
        QString filePath = fromInfo->fileUrl().toLocalFile();
        if (VaultController::ins()->isVaultFile(filePath)) {
            permissions = VaultController::ins()->getPermissions(filePath);
        } else if (deviceListener->isFileFromDisc(fromInfo->path())) {
            permissions |= MasteredMediaController::getPermissionsCopyToLocal();
        }
        // 修复bug-59124
        // 权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
        if (permissions != 0000) {
            if (m_refineStat == DFileCopyMoveJob::RefineBlock) {
                FileCopyInfoPointer copyinfo(new FileCopyInfo());
                copyinfo->isdir = true;
                copyinfo->permission = permissions;
                copyinfo->handler = handler;
                copyinfo->toinfo = toInfo;
                copyinfo->frominfo = fromInfo;
                writeQueueEnqueue(copyinfo);
            } else if (m_refineStat == DFileCopyMoveJob::RefineLocal) {
                QSharedPointer<DirSetPermissonInfo> dirinfo(new DirSetPermissonInfo);
                dirinfo->handler = handler;
                dirinfo->target = toInfo->fileUrl();
                dirinfo->permission = permissions;
                m_dirPermissonList.push_front(dirinfo);
            } else {
                handler->setPermissions(toInfo->fileUrl(), permissions);
            }
        }
    }

    if (mode == DFileCopyMoveJob::CopyMode) {
        return true;
    }

    if (existsSkipFile) {
        return true;
    }

    //  光盘中的目录不能被删除
    if (!fromInfo->canRename() || deviceListener->isFileFromDisc(fromInfo->fileUrl().toLocalFile())) {
        qInfo() << "canReaname : " << fromInfo->canRename();
        return true;
    }
    // 完成操作后删除原目录
    return removeFile(handler, fromInfo);
}

bool DFileCopyMoveJobPrivate::doCopyFile(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize)
{
    //多线程拷贝时发送当前拷贝信息
    if (m_refineStat != DFileCopyMoveJob::NoRefine){
        sendCopyInfo(fromInfo, toInfo);
    }
    //预先读取
    readAheadSourceFile(fromInfo);

    QSharedPointer<DFileDevice> fromDevice = nullptr;
    if (fromInfo->isGvfsMountFile()) {
        fromDevice.reset(new DGIOFileDevice(fromInfo->fileUrl()));
    } else {
        DLocalFileDevice *localDevice = new DLocalFileDevice();
        localDevice->setFileUrl(fromInfo->fileUrl());
        fromDevice.reset(localDevice);
    }

    if (!fromDevice)
        return handleUnknowUrlError(fromInfo, toInfo);

    QSharedPointer<DFileDevice> toDevice = nullptr;
    if (toInfo->exists() && !toInfo->isGvfsMountFile()) {
        DLocalFileDevice *localDevice = new DLocalFileDevice();
        localDevice->setFileUrl(toInfo->fileUrl());
        toDevice.reset(localDevice);
    } else {
        toDevice.reset(new DGIOFileDevice(toInfo->fileUrl()));
    }

    if (!toDevice)
        return handleUnknowUrlError(fromInfo, toInfo);

    bool isErrorOccur = false;
open_file: {
        DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

        do {
            // 如果打开文件在保险箱内
            QString strPath = toInfo->fileUrl().toString();
            if (VaultController::isVaultFile(strPath)) {
                QString strFileName = strPath.section("/", -1, -1);
                if (strFileName.toUtf8().length() > 255) {
                    qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();
                    isErrorOccur = true;
                    //错误队列处理
                    errorQueueHandling();
                    action = setAndhandleError(DFileCopyMoveJob::OpenError, fromInfo, toInfo,
                                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: file name too long"));
                    break;
                }
            }

            if (fromDevice->open(QIODevice::ReadOnly)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();
                DFileCopyMoveJob::Error errortype = fromInfo->isReadable() ? DFileCopyMoveJob::OpenError :
                                                    DFileCopyMoveJob::PermissionError;
                QString errorstr = fromInfo->isReadable() ?
                                   qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(fromDevice->errorString()) :
                                   QString();
                isErrorOccur = true;
                //错误队列处理
                errorQueueHandling();
                action = setAndhandleError(errortype, fromInfo, toInfo, errorstr);
            }
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());  // bug: 26333, while set the stop status shoule break the process!
        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                              action == DFileCopyMoveJob::NoAction);
            isErrorOccur = false;
        }

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        do {
            if (toDevice->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << toInfo->fileUrl() << QThread::currentThreadId();
                DFileCopyMoveJob::Error errortype = (!toInfo->exists() || toInfo->isWritable()) ? DFileCopyMoveJob::OpenError :
                                                    DFileCopyMoveJob::PermissionError;
                // task-36496 "Permission denied"没有被翻译 翻译为“没有权限”
                QString errorstr("");
                if ("Permission denied" == toDevice->errorString()) {
                    errorstr = (!toInfo->exists() || toInfo->isWritable()) ?
                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: Permission denied") :
                               QString();
                } else {
                    errorstr = (!toInfo->exists() || toInfo->isWritable()) ?
                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(toDevice->errorString()) :
                               QString();
                }
                isErrorOccur = true;
                //错误队列处理
                errorQueueHandling();
                action = setAndhandleError(errortype, fromInfo, toInfo, errorstr);
                if (action == DFileCopyMoveJob::RetryAction) {
                    QThread::msleep(THREAD_SLEEP_TIME);
                }
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                              action == DFileCopyMoveJob::NoAction);
            isErrorOccur = false;
        }
        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        if (fileHints.testFlag(DFileCopyMoveJob::ResizeDestinationFile)) {
            do {
                if (toDevice->resize(fromInfo->size())) {
                    action = DFileCopyMoveJob::NoAction;
                } else {
                    isErrorOccur = true;
                    //错误队列处理
                    errorQueueHandling();
                    action = setAndhandleError(DFileCopyMoveJob::ResizeError, fromInfo, toInfo, toDevice->errorString());
                }
                //防止卡死
                if (action == DFileCopyMoveJob::RetryAction) {
                    QThread::msleep(THREAD_SLEEP_TIME);
                }
            } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());
            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                                  action == DFileCopyMoveJob::NoAction);
                isErrorOccur = false;
            }
            if (action == DFileCopyMoveJob::SkipAction) {
                fromDevice->close();
                toDevice->close();
                return true;
            } else if (action != DFileCopyMoveJob::NoAction) {
                fromDevice->close();
                toDevice->close();
                return false;
            }
        }
    }

#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (fromDevice->handle() > 0) {
        posix_fadvise(fromDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }

    if (toDevice->handle() > 0) {
        posix_fadvise(toDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif

    currentJobDataSizeInfo.first = fromInfo->size();
    currentJobFileHandle = toDevice->handle();
    uLong source_checksum = adler32(0L, nullptr, 0);
    DGIOFileDevice *fromgio = qobject_cast<DGIOFileDevice *>(fromDevice.data());
    DGIOFileDevice *togio = qobject_cast<DGIOFileDevice *>(toDevice.data());
    if (fromgio) {
        saveCurrentDevice(fromInfo->fileUrl(),fromDevice);
    }
    if (togio) {
        saveCurrentDevice(toInfo->fileUrl(),toDevice);
    }

    qint64 block_Size = fromInfo->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : fromInfo->size();

    char *data = new char[block_Size + 1];

    Q_FOREVER {
        qint64 current_pos = fromDevice->pos();
    read_data:
        if (Q_UNLIKELY(!stateCheck())) {
            cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
            return false;
        }

        qint64 size_read = fromDevice->read(data, block_Size);
        if (Q_UNLIKELY(!stateCheck())) {
            cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
            return false;
        }

        if (Q_UNLIKELY(size_read <= 0)) {
            if (size_read == 0 && fromDevice->atEnd()) {
                break;
            }

            const_cast<DAbstractFileInfo *>(fromInfo.data())->refresh();

            DFileCopyMoveJob::Error errortype = fromInfo->exists() ? DFileCopyMoveJob::ReadError :
                                                DFileCopyMoveJob::NonexistenceError;
            QString errorstr = fromInfo->exists() ?
                               qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: %1").arg(fromDevice->errorString()) :
                               QString();
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            switch (setAndhandleError(errortype, fromInfo, toInfo, errorstr)) {
            case DFileCopyMoveJob::RetryAction: {
                //处理网络文件是否是可以访问的
                DFileCopyMoveJob::GvfsRetryType retryType = gvfsFileRetry(data, isErrorOccur, current_pos, fromInfo, toInfo, fromDevice, toDevice, false);
                if (DFileCopyMoveJob::GvfsRetrySkipAction == retryType) {
                    return true;
                } else if (DFileCopyMoveJob::GvfsRetryCancelAction == retryType) {
                    return false;
                } else if (DFileCopyMoveJob::GvfsRetryNoAction == retryType) {
                    goto read_data;
                }

                if (!fromDevice->seek(current_pos)) {
                    cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                    return handleUnknowError(fromInfo, toInfo, fromDevice->errorString());
                }

                goto read_data;
            }
            case DFileCopyMoveJob::SkipAction:
                cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                return true;
            default:
                cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                return false;
            }
        }

        current_pos = toDevice->pos();
    write_data:
        if (Q_UNLIKELY(!stateCheck())) {
            cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
            return false;
        }
        qint64 size_write = toDevice->write(data, size_read);
        if (Q_UNLIKELY(!stateCheck())) {
            cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
            return false;
        }
        //如果写失败了，直接推出
        if (size_write < 0) {
            if (!stateCheck()) {
                cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
                return false;
            }
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            switch (setAndhandleError(DFileCopyMoveJob::WriteError, fromInfo, toInfo,
                                      qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString()))) {
            case DFileCopyMoveJob::RetryAction: {
                //处理网络文件是否是可以访问的
                DFileCopyMoveJob::GvfsRetryType retryType = gvfsFileRetry(data, isErrorOccur, current_pos, fromInfo, toInfo, fromDevice, toDevice, true);
                if (DFileCopyMoveJob::GvfsRetrySkipAction == retryType) {
                    return true;
                } else if (DFileCopyMoveJob::GvfsRetryCancelAction == retryType) {
                    return false;
                } else if (DFileCopyMoveJob::GvfsRetryNoAction == retryType) {
                    goto read_data;
                }

                if (!toDevice->seek(current_pos)) {
                    cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
                    return handleUnknowError(fromInfo, toInfo, fromDevice->errorString());
                }

                goto write_data;
            }
            case DFileCopyMoveJob::SkipAction:
                cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled();
                    isErrorOccur = false;
                }
                return true;
            default:
                cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled(false);
                    isErrorOccur = false;
                }
                return false;
            }
        }

        //fix 修复vfat格式u盘卡死问题，写入数据后立刻同步
        if (m_isEveryReadAndWritesSnc && size_write > 0) {
            toDevice->inherits("");
            toDevice->syncToDisk(m_isVfat);
        }
        countrefinesize(size_write);

        if (Q_UNLIKELY(size_write != size_read)) {
            do {
                // 在某些情况下（往sftp挂载目录写入），可能一次未能写入那么多数据
                // 但不代表写入失败，应该继续尝试，直到所有数据全部写入
                if (size_write > 0) {
                    const char *surplus_data = data;
                    qint64 surplus_size = size_read;

                    do {
                        currentJobDataSizeInfo.second += size_write;
                        completedDataSize += size_write;
                        //        writtenDataSize += size_write;

                        surplus_data += size_write;
                        surplus_size -= size_write;
                        size_write = toDevice->write(surplus_data, surplus_size);
                        if (Q_UNLIKELY(!stateCheck())) {
                            return false;
                        }
                    } while (size_write > 0 && size_write != surplus_size);

                    // 表示全部数据写入完成
                    if (size_write > 0) {
                        break;
                    }
                }

                DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::WriteError;
                QString errorstr = qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString());
                if (!checkFreeSpace(currentJobDataSizeInfo.first - currentJobDataSizeInfo.second)) {
                    errortype = DFileCopyMoveJob::NotEnoughSpaceError;
                    errorstr = QString();
                }
                isErrorOccur = true;
                //错误队列处理
                errorQueueHandling();
                switch (setAndhandleError(errortype, fromInfo, toInfo, errorstr)) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!toDevice->seek(current_pos)) {
                        //当前错误处理完成
                        if (isErrorOccur) {
                            errorQueueHandled(false);
                            isErrorOccur = false;
                        }
                        cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
                        return handleUnknowError(fromInfo, toInfo, fromDevice->errorString());
                    }

                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    //当前错误处理完成
                    if (isErrorOccur) {
                        errorQueueHandled();
                        isErrorOccur = false;
                    }
                    cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
                    return true;
                default:
                    //当前错误处理完成
                    if (isErrorOccur) {
                        errorQueueHandled(false);
                        isErrorOccur = false;
                    }
                    cleanDoCopyFileSource(data, fromInfo, toInfo, fromDevice, toDevice);
                    return false;
                }
            } while (false);
        }

        currentJobDataSizeInfo.second += size_write;
        completedDataSize += size_write;
        completedDataSizeOnBlockDevice += size_write;

        if (Q_LIKELY(!fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking))) {
            source_checksum = adler32(source_checksum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(size_read));
        }

    }
    delete[] data;
    data = nullptr;
    fromDevice->close();
    toDevice->close();
    countrefinesize(fromInfo->size() <= 0 ? FileUtils::getMemoryPageSize() : 0);

    //对文件加权
    handler->setFileTime(toInfo->fileUrl(), fromInfo->lastRead(), fromInfo->lastModified());

    QFileDevice::Permissions permissions = fromInfo->permissions();
    //! use stat function to read vault file permission.
    QString path = fromInfo->fileUrl().path();
    if (VaultController::isVaultFile(path)) {
        permissions = VaultController::getPermissions(path);
    } else if (deviceListener->isFileFromDisc(fromInfo->path())) { // fix bug 52610: 从光盘中复制出来的文件权限为只读，与 ubuntu 策略保持一致，拷贝出来权限为 rw-rw-r--
        permissions |= MasteredMediaController::getPermissionsCopyToLocal();
    }
    //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000)
        handler->setPermissions(toInfo->fileUrl(), permissions);


    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    if (fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking)) {
        return true;
    }

    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

    do {
        if (toDevice->open(QIODevice::ReadOnly)) {
            break;
        } else {
            QString errorstr = "Unable to open file for integrity check, , cause: " + toDevice->errorString();
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            action = setAndhandleError(DFileCopyMoveJob::OpenError, toInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());
    //当前错误处理完成
    if (isErrorOccur) {
        errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                          action == DFileCopyMoveJob::NoAction);
        isErrorOccur = false;
    }
    if (action == DFileCopyMoveJob::SkipAction) {
        return true;
    }
    //当打开目标文件失败了，就不去校验数据完整性
    else if (action == DFileCopyMoveJob::CancelAction) {
        return false;
    }
    //校验数据完整性

    char *data1 = new char[blockSize + 1];
    ulong target_checksum = adler32(0L, nullptr, 0);

    qint64 elapsed_time_checksum = 0;

    if (fileJob().isDebugEnabled()) {
        elapsed_time_checksum = updateSpeedElapsedTimer->elapsed();
    }

    Q_FOREVER {
        qint64 size = toDevice->read(data1, blockSize);

        if (Q_UNLIKELY(size <= 0)) {
            if (size == 0 && toDevice->atEnd()) {
                break;
            }

            QString errorstr = qApp->translate("DFileCopyMoveJob", "File integrity was damaged, cause: %1").arg(toDevice->errorString());
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            switch (setAndhandleError(DFileCopyMoveJob::IntegrityCheckingError, fromInfo, toInfo, errorstr)) {
            case DFileCopyMoveJob::RetryAction: {
                continue;
            }
            case DFileCopyMoveJob::SkipAction:
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled();
                    isErrorOccur = false;
                }
                delete [] data1;
                return true;
            default:
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled(false);
                    isErrorOccur = false;
                }
                delete [] data1;
                return false;
            }
        }

        target_checksum = adler32(target_checksum, reinterpret_cast<Bytef *>(data1), static_cast<uInt>(size));

        if (Q_UNLIKELY(!stateCheck())) {
            delete [] data1;
            return false;
        }
    }
    delete [] data1;

    qCDebug(fileJob(), "Time spent of integrity check of the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed_time_checksum);

    if (source_checksum != target_checksum) {
        qCWarning(fileJob(), "Failed on file integrity checking, source file: 0x%lx, target file: 0x%lx", source_checksum, target_checksum);
        isErrorOccur = true;
        //错误队列处理
        errorQueueHandling();
        DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::IntegrityCheckingError, fromInfo, toInfo);

        if (action == DFileCopyMoveJob::RetryAction) {
            goto open_file;
        }

        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled(action == DFileCopyMoveJob::SkipAction);
            isErrorOccur = false;
        }

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        }

        return false;
    }

    //当前错误处理完成
    if (isErrorOccur) {
        errorQueueHandled();
        isErrorOccur = false;
    }
    qCDebug(fileJob(), "adler value: 0x%lx", source_checksum);

    return true;
}

bool DFileCopyMoveJobPrivate::doCopySmallFilesOnDisk(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo,
                                                     const QSharedPointer<DFileDevice> &fromDevice, const QSharedPointer<DFileDevice> &toDevice,
                                                     const QSharedPointer<DFileHandler> &handler)
{
    sendCopyInfo(fromInfo, toInfo);
    //预先读取
    readAheadSourceFile(fromInfo);

    bool isErrorOccur = false;
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    if (!fromDevice) {
        return handleUnknowUrlError(fromInfo, toInfo);
    }

    if (!toDevice) {
        return handleUnknowUrlError(fromInfo, toInfo);
    }

open_file:{
        do {
            // 如果打开文件在保险箱内
            QString strPath = toInfo->fileUrl().toString();

            if (fromDevice->open(QIODevice::ReadOnly)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();
                DFileCopyMoveJob::Error errortype = fromInfo->isReadable() ? DFileCopyMoveJob::OpenError :
                                                    DFileCopyMoveJob::PermissionError;
                QString errorstr = fromInfo->isReadable() ?
                                   qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(fromDevice->errorString()) :
                                   QString();
                isErrorOccur = true;
                //错误队列处理
                errorQueueHandling();
                action = setAndhandleError(errortype, fromInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            }
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());  // bug: 26333, while set the stop status shoule break the process!
        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                              action == DFileCopyMoveJob::NoAction);
            isErrorOccur = false;
        }

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        do {
            if (toDevice->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << toInfo->fileUrl() << QThread::currentThreadId();
                DFileCopyMoveJob::Error errortype = (!toInfo->exists() || toInfo->isWritable()) ? DFileCopyMoveJob::OpenError :
                                                    DFileCopyMoveJob::PermissionError;
                // task-36496 "Permission denied"没有被翻译 翻译为“没有权限”
                QString errorstr("");
                if ("Permission denied" == toDevice->errorString()) {
                    errorstr = (!toInfo->exists() || toInfo->isWritable()) ?
                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: Permission denied") :
                               QString();
                } else {
                    errorstr = (!toInfo->exists() || toInfo->isWritable()) ?
                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(toDevice->errorString()) :
                               QString();
                }
                isErrorOccur = true;
                //错误队列处理
                errorQueueHandling();
                action = setAndhandleError(errortype, toInfo, DAbstractFileInfoPointer(nullptr), errorstr);
                if (action == DFileCopyMoveJob::RetryAction) {
                    QThread::msleep(THREAD_SLEEP_TIME);
                }
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                              action == DFileCopyMoveJob::NoAction);
            isErrorOccur = false;
        }
        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        if (fileHints.testFlag(DFileCopyMoveJob::ResizeDestinationFile)) {
            do {
                if (toDevice->resize(fromInfo->size())) {
                    action = DFileCopyMoveJob::NoAction;
                } else {
                    isErrorOccur = true;
                    //错误队列处理
                    errorQueueHandling();
                    action = setAndhandleError(DFileCopyMoveJob::ResizeError, toInfo, DAbstractFileInfoPointer(nullptr),
                                               toDevice->errorString());
                }
                //防止卡死
                if (action == DFileCopyMoveJob::RetryAction) {
                    QThread::msleep(THREAD_SLEEP_TIME);
                }
            } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());
            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                                  action == DFileCopyMoveJob::NoAction);
                isErrorOccur = false;
            }
            if (action == DFileCopyMoveJob::SkipAction) {
                fromDevice->close();
                toDevice->close();
                return true;
            } else if (action != DFileCopyMoveJob::NoAction) {
                fromDevice->close();
                toDevice->close();
                return false;
            }
        }
    }

#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (fromDevice->handle() > 0) {
        posix_fadvise(fromDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }

    if (toDevice->handle() > 0) {
        posix_fadvise(toDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif

    currentJobDataSizeInfo.first = fromInfo->size();
    currentJobFileHandle = toDevice->handle();
    DGIOFileDevice *fromgio = qobject_cast<DGIOFileDevice *>(fromDevice.data());
    DGIOFileDevice *togio = qobject_cast<DGIOFileDevice *>(toDevice.data());
    if (fromgio) {
        saveCurrentDevice(fromInfo->fileUrl(),fromDevice);
    }
    if (togio) {
        saveCurrentDevice(toInfo->fileUrl(),toDevice);
    }

    qint64 block_Size = fromInfo->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : fromInfo->size();
    uLong source_checksum = adler32(0L, nullptr, 0);
    char *data = new char[block_Size + 1];

    Q_FOREVER {
        qint64 current_pos = fromDevice->pos();
    read_data:
        if (Q_UNLIKELY(!stateCheck())) {
            cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
            return false;
        }

        qint64 size_read = fromDevice->read(data, block_Size);
        if (Q_UNLIKELY(!stateCheck())) {
            cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
            return false;
        }

        if (Q_UNLIKELY(size_read <= 0)) {
            if (size_read == 0 && fromDevice->atEnd()) {
                break;
            }

            const_cast<DAbstractFileInfo *>(fromInfo.data())->refresh();

            DFileCopyMoveJob::Error errortype = fromInfo->exists() ? DFileCopyMoveJob::ReadError :
                                                DFileCopyMoveJob::NonexistenceError;
            QString errorstr = fromInfo->exists() ?
                               qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: %1").arg(fromDevice->errorString()) :
                               QString();
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            switch (setAndhandleError(errortype, fromInfo, toInfo, errorstr)) {
            case DFileCopyMoveJob::RetryAction: {
                if (!fromDevice->seek(current_pos)) {
                    cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                    return handleUnknowError(fromInfo, toInfo, fromDevice->errorString());
                }

                goto read_data;
            }
            case DFileCopyMoveJob::SkipAction:
                cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                return true;
            default:
                cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                return false;
            }
        }

        current_pos = toDevice->pos();
    write_data:
        if (Q_UNLIKELY(!stateCheck())) {
            cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
            return false;
        }
        qint64 size_write = toDevice->write(data, size_read);
        if (Q_UNLIKELY(!stateCheck())) {
            cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
            return false;
        }
        //如果写失败了，直接推出
        if (size_write < 0) {
            if (!stateCheck()) {
                cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                return false;
            }
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            switch (setAndhandleError(DFileCopyMoveJob::WriteError, fromInfo, toInfo,
                                      qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString()))) {
            case DFileCopyMoveJob::RetryAction: {
                if (!toDevice->seek(current_pos)) {
                    cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                    return handleUnknowError(fromInfo, toInfo, fromDevice->errorString());
                }

                goto write_data;
            }
            case DFileCopyMoveJob::SkipAction:
                cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                return true;
            default:
                cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                return false;
            }
        }

        countrefinesize(size_write);

        if (Q_UNLIKELY(size_write != size_read)) {
            do {
                // 在某些情况下（往sftp挂载目录写入），可能一次未能写入那么多数据
                // 但不代表写入失败，应该继续尝试，直到所有数据全部写入
                if (size_write > 0) {
                    const char *surplus_data = data;
                    qint64 surplus_size = size_read;

                    do {
                        currentJobDataSizeInfo.second += size_write;
                        completedDataSize += size_write;

                        surplus_data += size_write;
                        surplus_size -= size_write;
                        size_write = toDevice->write(surplus_data, surplus_size);
                        if (Q_UNLIKELY(!stateCheck())) {
                            return false;
                        }
                    } while (size_write > 0 && size_write != surplus_size);

                    // 表示全部数据写入完成
                    if (size_write > 0) {
                        break;
                    }
                }

                DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::WriteError;
                QString errorstr = qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString());
                if (!checkFreeSpace(currentJobDataSizeInfo.first - currentJobDataSizeInfo.second)) {
                    errortype = DFileCopyMoveJob::NotEnoughSpaceError;
                    errorstr = QString();
                }
                isErrorOccur = true;
                //错误队列处理
                errorQueueHandling();
                switch (setAndhandleError(errortype, fromInfo, toInfo, errorstr)) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!toDevice->seek(current_pos)) {
                        cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                        return handleUnknowError(fromInfo, toInfo, fromDevice->errorString());
                    }

                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                    return true;
                default:
                    cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
                    return false;
                }
            } while (false);
        }

        currentJobDataSizeInfo.second += size_write;
        completedDataSize += size_write;
        completedDataSizeOnBlockDevice += size_write;
        if (Q_LIKELY(!fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking))) {
            source_checksum = adler32(source_checksum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(size_read));
        }
    }
    delete[] data;
    fromDevice->close();
    toDevice->close();
    countrefinesize(fromInfo->size() <= 0 ? FileUtils::getMemoryPageSize() : 0);

    //对文件加权
    handler->setFileTime(toInfo->fileUrl(), fromInfo->lastRead(), fromInfo->lastModified());

    QFileDevice::Permissions permissions = fromInfo->permissions();
    //! use stat function to read vault file permission.
    QString path = fromInfo->fileUrl().path();
    if (VaultController::isVaultFile(path)) {
        permissions = VaultController::getPermissions(path);
    } else if (deviceListener->isFileFromDisc(fromInfo->path())) { // fix bug 52610: 从光盘中复制出来的文件权限为只读，与 ubuntu 策略保持一致，拷贝出来权限为 rw-rw-r--
        permissions |= MasteredMediaController::getPermissionsCopyToLocal();
    }
    //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000)
        handler->setPermissions(toInfo->fileUrl(), permissions);

    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    if (fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking)) {
        return true;
    }

    do {
     if (toDevice->open(QIODevice::ReadOnly)) {
         break;
     } else {
         QString errorstr = "Unable to open file for integrity check, , cause: " + toDevice->errorString();
         isErrorOccur = true;
         //错误队列处理
         errorQueueHandling();
         action = setAndhandleError(DFileCopyMoveJob::OpenError, toInfo, DAbstractFileInfoPointer(nullptr), errorstr);
         //防止卡死
         if (action == DFileCopyMoveJob::RetryAction) {
             QThread::msleep(THREAD_SLEEP_TIME);
         }
     }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());
    //当前错误处理完成
    if (isErrorOccur) {
     errorQueueHandled(action == DFileCopyMoveJob::SkipAction);
     isErrorOccur = false;
    }
    if (action == DFileCopyMoveJob::SkipAction) {
     return true;
    }
    //当打开目标文件失败了，就不去校验数据完整性
    else if (action == DFileCopyMoveJob::CancelAction) {
     return false;
    }
    //校验数据完整性

    char *data1 = new char[block_Size + 1];
    ulong target_checksum = adler32(0L, nullptr, 0);

    qint64 elapsed_time_checksum = 0;

    if (fileJob().isDebugEnabled()) {
        elapsed_time_checksum = updateSpeedElapsedTimer->elapsed();
    }

    Q_FOREVER {
        qint64 size = toDevice->read(data1, block_Size);

        if (Q_UNLIKELY(size <= 0)) {
            if (size == 0 && toDevice->atEnd()) {
                break;
            }

            QString errorstr = qApp->translate("DFileCopyMoveJob", "File integrity was damaged, cause: %1").arg(toDevice->errorString());
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            switch (setAndhandleError(DFileCopyMoveJob::IntegrityCheckingError, fromInfo, toInfo, errorstr)) {
            case DFileCopyMoveJob::RetryAction: {
                continue;
            }
            case DFileCopyMoveJob::SkipAction:
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled();
                    isErrorOccur = false;
                }
                delete [] data1;
                return true;
            default:
                //当前错误处理完成
                if (isErrorOccur) {
                    errorQueueHandled(false);
                    isErrorOccur = false;
                }
                delete [] data1;
                return false;
            }
        }

        target_checksum = adler32(target_checksum, reinterpret_cast<Bytef *>(data1), static_cast<uInt>(size));

        if (Q_UNLIKELY(!stateCheck())) {
            delete [] data1;
            return false;
        }
    }
    delete [] data1;

    qCDebug(fileJob(), "Time spent of integrity check of the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed_time_checksum);

    if (source_checksum != target_checksum) {
        qCWarning(fileJob(), "Failed on file integrity checking, source file: 0x%lx, target file: 0x%lx", source_checksum, target_checksum);
        isErrorOccur = true;
        //错误队列处理
        errorQueueHandling();
        DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::IntegrityCheckingError, fromInfo, toInfo);

        if (action == DFileCopyMoveJob::RetryAction) {
            goto open_file;
        }

        //当前错误处理完成
        if (isErrorOccur) {
            errorQueueHandled(action == DFileCopyMoveJob::SkipAction);
            isErrorOccur = false;
        }

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        }

        return false;
    }

    //当前错误处理完成
    if (isErrorOccur) {
        errorQueueHandled();
        isErrorOccur = false;
    }
    qCDebug(fileJob(), "adler value: 0x%lx", source_checksum);

    return true;
}

bool DFileCopyMoveJobPrivate::doThreadPoolCopyFile()
{
    setLastErrorAction(DFileCopyMoveJob::NoAction);
    if (!stateCheck()) {
        q_ptr->stop();
        return false;
    }

    QSharedPointer<ThreadCopyInfo> threadInfo(nullptr);
    {
        QMutexLocker lk(&m_threadMutex);
        if (m_threadInfos.count() <= 0) {
            q_ptr->stop();
            return false;
        }
        threadInfo = m_threadInfos.dequeue();
    }

    if (!threadInfo)
        return false;

    const DAbstractFileInfoPointer fromInfo = threadInfo->fromInfo;
    const DAbstractFileInfoPointer toInfo = threadInfo->toInfo;
    const QSharedPointer<DFileHandler> handler = threadInfo->handler;
    bool ok = doCopySmallFilesOnDisk(fromInfo, toInfo, threadInfo->fromDevice, threadInfo->toDevice, threadInfo->handler);
    removeCurrentDevice(fromInfo->fileUrl());
    removeCurrentDevice(toInfo->fileUrl());
    if (!ok)
        q_ptr->stop();
    return ok;
}

bool DFileCopyMoveJobPrivate::doCopyFileOnBlock(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize)
{
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    int fromfd = -1;

    do {
        // 如果打开文件在保险箱内
        QString strPath = toInfo->fileUrl().toString();
        if (VaultController::isVaultFile(strPath)) {
            QString strFileName = strPath.section("/", -1, -1);
            if (strFileName.toUtf8().length() > 255) {
                action = setAndhandleError(DFileCopyMoveJob::OpenError, fromInfo, DAbstractFileInfoPointer(nullptr),
                                           qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: file name too long"));
                break;
            }
        }
        fromfd = open(fromInfo->fileUrl().toLocalFile().toUtf8().toStdString().data(), O_RDONLY);
        if (-1 != fromfd) {
            action = DFileCopyMoveJob::NoAction;
        } else {
            qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();
            DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::PermissionError;
            QString errorstr;
            if (fromInfo->isReadable()) {
                errorstr = qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: ")/*.arg(fromDevice->errorString())*/;
                errortype = DFileCopyMoveJob::OpenError;
            }

            action = setAndhandleError(errortype, fromInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

    if (action == DFileCopyMoveJob::SkipAction) {
        completedProgressDataSize += fromInfo->size() <= 0 ? FileUtils::getMemoryPageSize() : fromInfo->size();
        countrefinesize(fromInfo->size() <= 0 ? FileUtils::getMemoryPageSize() : fromInfo->size());
        return true;
    } else if (action != DFileCopyMoveJob::NoAction) {
        //出错就停止
        q_ptr->stop();
        return false;
    }
#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (fromfd > 0) {
        posix_fadvise(fromfd, 0, 0, POSIX_FADV_SEQUENTIAL);
    }

#endif
    qint64 size_block = fromInfo->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : fromInfo->size();
    size_block = blockSize;
    FileCopyInfoPointer copyinfo(new FileCopyInfo());
    copyinfo->handler = handler;
    copyinfo->frominfo = fromInfo;
    copyinfo->toinfo = toInfo;
    lseek(fromfd, 0, SEEK_SET);
    qint64 current_pos = 0;
    while (true) {
        while(checkWritQueueCount()) {
            QThread::msleep(1);
        }
        copyinfo->currentpos = current_pos;
        char *buffer = new char[size_block + 1];

        if (Q_UNLIKELY(!stateCheck())) {
            delete[]  buffer;
            close(fromfd);
            return false;
        }

        if (skipReadFileDealWriteThread(fromInfo->fileUrl()))
        {
            completedProgressDataSize += fromInfo->size() <= 0
                    ? FileUtils::getMemoryPageSize() : fromInfo->size() - current_pos;
            countrefinesize(fromInfo->size() <= 0
                            ? FileUtils::getMemoryPageSize() : fromInfo->size() - current_pos);
            delete[]  buffer;
            close(fromfd);
            return true;
        }

        qint64 size_read = read(fromfd, buffer, static_cast<size_t>(size_block));

        if (Q_UNLIKELY(!stateCheck())) {
            delete[]  buffer;
            close(fromfd);
            return false;
        }

        if (Q_UNLIKELY(size_read <= 0)) {
            if (size_read == 0 && current_pos == fromInfo->size()) {
                copyinfo->buffer = buffer;
                copyinfo->size = size_read;
                break;
            }

            const_cast<DAbstractFileInfo *>(copyinfo->frominfo.data())->refresh();
            DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::NonexistenceError;
            QString errorstr;
            if (copyinfo->frominfo->exists()) {
                errortype = DFileCopyMoveJob::ReadError;
                errorstr = qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: ")/*.arg(fromDevice->errorString())*/;
            }
            switch (setAndhandleError(errortype, copyinfo->frominfo, copyinfo->toinfo, errorstr)) {
            case DFileCopyMoveJob::RetryAction: {
                if (!lseek(fromfd, current_pos, SEEK_SET)) {
                    setError(DFileCopyMoveJob::UnknowError, "");
                    delete[]  buffer;
                    close(fromfd);
                    q_ptr->stop();
                    return false;
                }
                break;
            }
            case DFileCopyMoveJob::SkipAction:
                m_skipFileQueueMutex.lock();
                m_skipFileQueue.push_back(fromInfo->fileUrl());
                m_skipFileQueueMutex.unlock();
                completedProgressDataSize += fromInfo->size() <= 0
                        ? FileUtils::getMemoryPageSize() : fromInfo->size() - current_pos;
                countrefinesize(fromInfo->size() <= 0
                                ? FileUtils::getMemoryPageSize() : fromInfo->size() - current_pos);
                delete[]  buffer;
                return true;
            default:
                close(fromfd);
                q_ptr->stop();
                delete[]  buffer;
                return false;
            }
        } else {
            FileCopyInfoPointer tmpinfo(new FileCopyInfo);
            tmpinfo->closeflag = false;
            tmpinfo->frominfo = copyinfo->frominfo;
            tmpinfo->toinfo = copyinfo->toinfo;
            tmpinfo->currentpos = current_pos;
            tmpinfo->buffer = buffer;
            tmpinfo->size = size_read;
            current_pos += size_read;

            writeQueueEnqueue(tmpinfo);
            if (!m_isWriteThreadStart.load()) {
                m_isWriteThreadStart.store(true);
                m_writeResult = QtConcurrent::run([this]() {
                    writeRefineThread();
                });
            }
        }
    }

    if (fromInfo->size() <= 0) {
        completedProgressDataSize += FileUtils::getMemoryPageSize();
        countrefinesize(FileUtils::getMemoryPageSize());
    }

    writeQueueEnqueue(copyinfo);
    if (!m_isWriteThreadStart.load()) {
        m_isWriteThreadStart.store(true);
        m_writeResult = QtConcurrent::run([this]() {
            writeRefineThread();
        });
    }

    close(fromfd);

    return true;
}

bool DFileCopyMoveJobPrivate::doRemoveFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo, const DAbstractFileInfoPointer &toInfo)
{
    if (!fileInfo->exists()) {
        return true;
    }

    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    bool is_file = fileInfo->isFile() || fileInfo->isSymLink();

    do {
        //! vault file without writable permission cannot processed by system function.
        bool canRemove = true;
        QString absolutePath = fileInfo->absolutePath();
        DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::PermissionError;
        if (VaultController::isVaultFile(absolutePath)) {
            VaultController::FileBaseInfo fbi = VaultController::ins()->getFileInfo(VaultController::localToVault(absolutePath));
            if (!fbi.isWritable) {
                canRemove = false;
            }
        }
        QString errorstr;
        if (canRemove) {
            if (is_file ? handler->remove(fileInfo->fileUrl()) : handler->rmdir(fileInfo->fileUrl())) {
                return true;
            }

            if (fileInfo->absoluteFilePath().startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashExpungedPath)))
                return true;

            if (fileInfo->canRename()) {
                errortype = DFileCopyMoveJob::RemoveError;
                errorstr = qApp->translate("DFileCopyMoveJob", "Failed to delete the file, cause: %1").arg(handler->errorString());
            }
        }

        action = setAndhandleError(errortype, fileInfo, toInfo, errorstr);
        if (action == DFileCopyMoveJob::RetryAction) { // 仅在选择重试时触发休眠
            QThread::msleep(THREAD_SLEEP_TIME); // fix bug 44436 高频执行循环高频发送信号导致主界面卡死
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

    return action == DFileCopyMoveJob::SkipAction;
}

bool DFileCopyMoveJobPrivate::doRenameFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer oldInfo, const DAbstractFileInfoPointer newInfo)
{
    const DStorageInfo &storage_source = directoryStack.top().sourceStorageInfo;
    const DStorageInfo &storage_target = directoryStack.top().targetStorageInfo;

    if (storage_target.device() != "gvfsd-fuse" || storage_source == storage_target) {
        // 先尝试直接rename
        if (handler->rename(oldInfo->fileUrl(), newInfo->fileUrl())) {
            // 剪切合并需要更新进度条
            currentJobDataSizeInfo.first = oldInfo->size();
            currentJobDataSizeInfo.second += newInfo->size();
            completedDataSize += newInfo->size();
            completedDataSizeOnBlockDevice += newInfo->size();
            needUpdateProgress = true;
            if (Q_UNLIKELY(!stateCheck())) {
                return false;
            }
            return true;
        } else { // bug-35066 添加对保险箱的判断
            if (oldInfo->isSymLink()) { // 如果为链接文件

                // 判断当前目录是否存在该名称的链接文件，如果存在，则删除
                if (newInfo->exists()) {
                    if (!removeFile(handler, newInfo)) {
                        return false;
                    }
                }

                // 新建链接文件
                if (!handler->link(oldInfo->symlinkTargetPath(), newInfo->fileUrl())) {
                    //错误队列处理
                    errorQueueHandling();
                    bool ok = setAndhandleError(DFileCopyMoveJob::SymlinkToGvfsError, oldInfo, newInfo,
                                                handler->errorString()) == DFileCopyMoveJob::SkipAction;
                    //当前错误处理完成
                    errorQueueHandled();
                    return ok;
                }

                // 删除旧的链接文件
                if (!doRemoveFile(handler, oldInfo, newInfo)) {
                    return false;
                }

                return true;

            }
        }
    }

    qCDebug(fileJob(), "Failed on rename, Well be copy and delete the file");

    // 先复制再删除
    if (!doCopyFile(oldInfo, newInfo, handler)) {
        return false;
    }

    if (getLastErrorAction() == DFileCopyMoveJob::SkipAction) {
        // 说明复制文件过程被跳过
        return true;
    }

    handler->setFileTime(newInfo->fileUrl(), oldInfo->lastRead(), oldInfo->lastModified());

    if (!doRemoveFile(handler, oldInfo, newInfo)) {
        return false;
    }

    return true;
}

bool DFileCopyMoveJobPrivate::doLinkFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo, const QString &linkPath)
{
    Q_Q(DFileCopyMoveJob);
    if (fileInfo->exists()) {
        if (!removeFile(handler, fileInfo)) {
            return false;
        }
    }

    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

    do {
        if (handler->link(linkPath, fileInfo->fileUrl())) {
            return true;
        }
        action = setAndhandleError(DFileCopyMoveJob::SymlinkError, fileInfo, DAbstractFileInfoPointer(nullptr),
                                   qApp->translate("DFileCopyMoveJob", "Fail to create symlink, cause: %1").arg(handler->errorString()));
        if (action == DFileCopyMoveJob::RetryAction) {// 仅在用户重试时休眠
            q->msleep(THREAD_SLEEP_TIME);
        } // fix bug#30091 文件操作失败的时候，点击对话框的“不再提示+重试”，会导致不停失败不停发送信号通知主线程更新ui，这里加个延时控制响应频率
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

    return action == DFileCopyMoveJob::SkipAction;
}

void DFileCopyMoveJobPrivate::convertTrashFile(DAbstractFileInfoPointer &fileInfo)
{
    // 创建一个用于暂存即将被删除文件的目录
    QDir expungedDir(DFMStandardPaths::location(DFMStandardPaths::TrashExpungedPath));
    if (!expungedDir.exists())
        expungedDir.mkdir(DFMStandardPaths::location(DFMStandardPaths::TrashExpungedPath));

    // 获取文件名以及创建文件名
    const QString srcPath = fileInfo->absoluteFilePath();
    const QString tmpPath = DFMStandardPaths::location(DFMStandardPaths::TrashExpungedPath) + "/" + QUuid::createUuid().toString();

    // 将文件移动到expunged目录下
    QByteArray srcPathBytes(srcPath.toLocal8Bit());
    QByteArray tmpPathBytes(tmpPath.toLocal8Bit());
    if (::rename(srcPathBytes.data(), tmpPathBytes.data()) == 0) {
        DAbstractFileInfoPointer tmpFileInfo = fileService->createFileInfo(nullptr, DUrl::fromLocalFile(tmpPath), false);
        if (tmpFileInfo && tmpFileInfo->exists())
            fileInfo = tmpFileInfo;
    }
}

bool DFileCopyMoveJobPrivate::process(const DUrl from, const DAbstractFileInfoPointer target_info)
{
    const DAbstractFileInfoPointer &source_info = DFileService::instance()->createFileInfo(nullptr, from, false);

    return process(from, source_info, target_info);
}

bool DFileCopyMoveJobPrivate::process(const DUrl from, const DAbstractFileInfoPointer source_info, const DAbstractFileInfoPointer target_info, const bool isNew)
{
    // reset error and action
    if (m_refineStat == DFileCopyMoveJob::NoRefine) {
        unsetError();
    }
    setLastErrorAction(DFileCopyMoveJob::NoAction);

    beginJob(JobInfo::Preprocess, from, (source_info && target_info) ? target_info->getUrlByChildFileName(source_info->fileName()) : DUrl(), source_info->isDir());
    bool ok = doProcess(from, source_info, target_info, isNew);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::copyFile(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize)
{
    qint64 elapsed = 0;
    if (fileJob().isDebugEnabled()) {
        elapsed = updateSpeedElapsedTimer->elapsed();
        updateSpeedElapsedTimer->elapsed();
    }
    beginJob(JobInfo::Copy, fromInfo->fileUrl(), toInfo->fileUrl());
    bool ok = true;
    if (m_refineStat == DFileCopyMoveJob::NoRefine) {
         ok = doCopyFile(fromInfo, toInfo, handler, blockSize);
         //fix bug 62202 不执行优化拷贝结束后直接返回
         removeCurrentDevice(fromInfo->fileUrl());
         removeCurrentDevice(toInfo->fileUrl());
         endJob();
         qCDebug(fileJob(), "Time spent of copy the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed);
         return ok;
    }
    const static int bigFileSize = 500 * 1024 * 1024;
    //判读目标目录和本地目录是不是同盘，并且是大文件
    if (m_refineStat == DFileCopyMoveJob::RefineLocal) {
        if (fromInfo->size() > bigFileSize) {
            while (m_pool.activeThreadCount() > 0) {
                QThread::msleep(10);
            }
            ok = doCopyFile(fromInfo,toInfo,handler,blockSize);
        }
        //1.判断源文件是本地，目标文件也是本地执行读写线程分离处理
        //2.判断源文件是本地，目标文件是（除光盘外的）块设备，
        else {
            if (!stateCheck())
                return false;
            QSharedPointer<ThreadCopyInfo> threadInfo(new ThreadCopyInfo);
            threadInfo->fromInfo = fromInfo;
            threadInfo->toInfo = toInfo;
            threadInfo->toDevice.reset(DFileService::instance()->createFileDevice(nullptr, toInfo->fileUrl()));
            threadInfo->fromDevice.reset(DFileService::instance()->createFileDevice(nullptr, fromInfo->fileUrl()));
            threadInfo->handler = handler;
            {
                QMutexLocker lk(&m_threadMutex);
                m_threadInfos << threadInfo;
            }
            QtConcurrent::run(&m_pool, this, static_cast<bool(DFileCopyMoveJobPrivate::*)()>
                              (&DFileCopyMoveJobPrivate::doThreadPoolCopyFile));
            endJob();
            qCDebug(fileJob(), "Time spent of copy the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed);
            return ok;
        }
    }
    else {
        ok = doCopyFileOnBlock(fromInfo, toInfo, handler, blockSize);
    }
    removeCurrentDevice(fromInfo->fileUrl());
    removeCurrentDevice(toInfo->fileUrl());
    endJob();
    qCDebug(fileJob(), "Time spent of copy the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed);
    return ok;
}

bool DFileCopyMoveJobPrivate::removeFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo)
{
    beginJob(JobInfo::Remove, fileInfo->fileUrl(), DUrl());
    bool ok = doRemoveFile(handler, fileInfo);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::renameFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer oldInfo, const DAbstractFileInfoPointer newInfo)
{
    Q_UNUSED(handler);

    beginJob(JobInfo::Move, oldInfo->fileUrl(), newInfo->fileUrl());
    bool ok = doRenameFile(handler, oldInfo, newInfo);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::linkFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo, const QString &linkPath)
{
    beginJob(JobInfo::Link, DUrl(linkPath), fileInfo->fileUrl());
    bool ok = doLinkFile(handler, fileInfo, linkPath);
    endJob();

    return ok;
}

void DFileCopyMoveJobPrivate::beginJob(JobInfo::Type type, const DUrl from, const DUrl target, const bool isDir)
{
//    qCDebug(fileJob(), "job begin, Type: %d, from: %s, to: %s", type, qPrintable(from.toString()), qPrintable(target.toString()));
    jobStack.push({type, QPair<DUrl, DUrl>(from, target)});
    currentJobDataSizeInfo = qMakePair(-1, 0);
    currentJobFileHandle = -1;
    if (isDir || m_refineStat == DFileCopyMoveJob::NoRefine)
        Q_EMIT q_ptr->currentJobChanged(from, target, false);
}

void DFileCopyMoveJobPrivate::endJob(const bool isNew)
{
    if (isNew) {
        return;
    }
    jobStack.pop();
    currentJobFileHandle = -1;

//    qCDebug(fileJob()) << "job end, error:" << error << "last error handle action:" << lastErrorHandleAction;
}

void DFileCopyMoveJobPrivate::enterDirectory(const DUrl from, const DUrl to)
{
    DirectoryInfo info;

    info.url = qMakePair(from, to);

    if (from.isLocalFile()) {
        info.sourceStorageInfo.setPath(from.toLocalFile());
    }

    if (to.isLocalFile()) {
        if (!directoryStack.isEmpty()) {
            if (directoryStack.top().url.second == to) {
                info.targetStorageInfo = directoryStack.top().targetStorageInfo;
            } else {
                info.targetStorageInfo.setPath(to.toLocalFile());
            }
        } else {
            info.targetStorageInfo.setPath(to.toLocalFile());
        }
    }

    directoryStack.push(info);
}

void DFileCopyMoveJobPrivate::leaveDirectory()
{
    directoryStack.pop();
}

void DFileCopyMoveJobPrivate::joinToCompletedFileList(const DUrl from, const DUrl target, qint64 dataSize)
{
//    qCDebug(fileJob(), "file. from: %s, target: %s, data size: %lld", qPrintable(from.toString()), qPrintable(target.toString()), dataSize);

    if (currentJobDataSizeInfo.first < 0) {
        completedDataSize += dataSize;
    }

    completedProgressDataSize += dataSize <= 0 ? FileUtils::getMemoryPageSize() : 0;

    ++completedFilesCount;

    Q_EMIT q_ptr->completedFilesCountChanged(completedFilesCount);

    if (getLastErrorAction() == DFileCopyMoveJob::SkipAction) {
        return;
    }

    completedFileList << qMakePair(from, target);
}

void DFileCopyMoveJobPrivate::joinToCompletedDirectoryList(const DUrl from, const DUrl target, qint64 dataSize)
{
    Q_UNUSED(dataSize)
//    qCDebug(fileJob(), "directory. from: %s, target: %s, data size: %lld", qPrintable(from.toString()), qPrintable(target.toString()), dataSize);

    // warning: isFromLocalUrls 对于外部挂载存储设备返回true，如果要修改 isFromLocalUrls 的含义
    //          将会影响到以下判断逻辑
    qint64 dirSize = (m_isFileOnDiskUrls && targetUrl.isValid()) ?  m_currentDirSize : FileUtils::getMemoryPageSize();
    completedProgressDataSize += (dirSize <= 0 ? FileUtils::getMemoryPageSize() : dirSize);
    ++completedFilesCount;

    countrefinesize(dirSize);

    Q_EMIT q_ptr->completedFilesCountChanged(completedFilesCount);

    if (getLastErrorAction() == DFileCopyMoveJob::SkipAction) {
        return;
    }

    completedDirectoryList << qMakePair(from, target);
}

void DFileCopyMoveJobPrivate::updateProgress()
{
    switch (mode) {
    case DFileCopyMoveJob::CopyMode:
    case DFileCopyMoveJob::CutMode:
        updateCopyProgress();
        break;
    case DFileCopyMoveJob::MoveMode:
        updateMoveProgress();
        break;
    }

}

void DFileCopyMoveJobPrivate::updateCopyProgress()
{
    // 网络文件使用统计线程的值获取总大小. 非网络文件使用 fts_* 系统 API 统计函数同步统计总大小
    bool fromLocal = (m_isFileOnDiskUrls && targetUrl.isValid());
    const qint64 totalSize = fromLocal ? totalsize : fileStatistics->totalProgressSize();
    //通过getCompletedDataSize取出的已传输的数据大小后期会远超实际数据大小，这种情况下直接使用completedDataSize
    qint64 dataSize(getCompletedDataSize());
    // completedDataSize 可能一直为 0
    if (dataSize > completedDataSize && completedDataSize > 0) {
        dataSize = completedDataSize;
    }

    dataSize += completedProgressDataSize;
    dataSize -= m_gvfsFileInnvliadProgress;

    //优化
    dataSize = m_bDestLocal ? m_refineCopySize : dataSize;

    dataSize += skipFileSize;

    if (totalSize == 0)
        return;

    if ((fromLocal && m_isCountSizeOver) || fileStatistics->isFinished()) {
        qreal realProgress = qreal(dataSize) / totalSize;
        if (realProgress > lastProgress)
            lastProgress = realProgress;
        qCDebug(fileJob(), "completed data size: %lld, total data size: %lld,m_refineCopySize = %lld", dataSize, totalSize, completedProgressDataSize);
    } else {
        //预设一个总大小，让前期进度平滑一些（目前阈值取1mb）
        qreal virtualSize = totalSize < 1000000 ? 1000000 : totalSize;
        if (dataSize < virtualSize /*&& total_size > 0*/) {
            // 取一个时时的总大小来计算一个模糊进度
            qreal fuzzyProgress = qreal(dataSize) / virtualSize;
            if (fuzzyProgress < 0.3 && fuzzyProgress > lastProgress)
                lastProgress = fuzzyProgress;
        }
    }
    // 保证至少出现%1
    if (lastProgress < 0.02) {
        lastProgress = 0.01;
    }

    Q_EMIT q_ptr->progressChanged(qMin(lastProgress, 1.0), dataSize);

    if (currentJobDataSizeInfo.first > 0) {
        Q_EMIT q_ptr->currentFileProgressChanged(qMin(qreal(currentJobDataSizeInfo.second) / currentJobDataSizeInfo.first, 1.0), currentJobDataSizeInfo.second);
    }
}


// use count calculate progress when mode is move
void DFileCopyMoveJobPrivate::updateMoveProgress()
{
    Q_Q(DFileCopyMoveJob);
    int totalCount = q->totalFilesCount();
    if (totalCount > 0) {
        qreal realProgress = qreal(completedFilesCount) / totalCount;
        if (realProgress > lastProgress)
            lastProgress = realProgress;
    } else if (countStatisticsFinished && totalMoveFilesCount > 0) {
        qreal realProgress = qreal(completedFilesCount) / totalMoveFilesCount;
        if (realProgress > lastProgress)
            lastProgress = realProgress;
    } else {
        if (completedFilesCount < totalMoveFilesCount && totalMoveFilesCount > 0) {
            qreal fuzzyProgress = qreal(completedFilesCount) / totalMoveFilesCount;
            if (fuzzyProgress < 0.5 && fuzzyProgress > lastProgress)
                lastProgress = fuzzyProgress;
        }
    }
    // 保证至少出现%1
    if (lastProgress < 0.02) {
        lastProgress = 0.01;
    }
    Q_EMIT q_ptr->progressChanged(qMin(lastProgress, 1.0), 0);
}

void DFileCopyMoveJobPrivate::updateSpeed()
{
    const qint64 time = updateSpeedElapsedTimer->elapsed();
    const qint64 total_size = m_bDestLocal ? m_refineCopySize : getCompletedDataSize();
    if (time == 0)
        return;

    qint64 speed = total_size / time * 1000;

    // 如果进度已经是100%，则不应该再有速度波动
    if (fileStatistics->isFinished() && total_size >= fileStatistics->totalSize()) {
        speed = 0;
    }

    // 复制和剪切文件时显示速度
    if (mode != DFileCopyMoveJob::MoveMode) {
        Q_EMIT q_ptr->speedUpdated(speed);
    }
}

void DFileCopyMoveJobPrivate::_q_updateProgress()
{
    ++timeOutCount;

    updateSpeed();
    updateProgress();

    // 因为sleep状态时可能会导致进度信息长时间无法得到更新，故在此处直接更新进度信息
    if (state == DFileCopyMoveJob::IOWaitState) {
        updateProgress();
    } else {
        needUpdateProgress = true;
    }
}

void DFileCopyMoveJobPrivate::countrefinesize(const qint64 &size)
{
    QMutexLocker lock(&m_refineMutex);
    m_refineCopySize += size;
}

void DFileCopyMoveJobPrivate::checkTagetNeedSync()
{
    if (!targetUrl.isValid()) {
        return;
    }
    m_isEveryReadAndWritesSnc = m_isTagGvfsFile;
    DStorageInfo targetStorageInfo(targetUrl.toLocalFile());
    if (!m_isEveryReadAndWritesSnc && targetStorageInfo.isValid()) {
        const QString &fs_type = targetStorageInfo.fileSystemType();
        m_isVfat = fs_type.contains("vfat");
        m_isEveryReadAndWritesSnc = (fs_type == "cifs" || fs_type == "vfat");
    }
}

void DFileCopyMoveJobPrivate::checkTagetIsFromBlockDevice()
{
    if (!targetUrl.isValid()) {
        return;
    }
    m_isTagFromBlockDevice.store(!deviceListener->isFileFromDisc(targetUrl.toLocalFile()) &&
                                 deviceListener->isBlockFile(targetUrl.toLocalFile()));
}

bool DFileCopyMoveJobPrivate::checkWritQueueEmpty()
{
    QMutexLocker lk(&m_copyInfoQueueMutex);
    return m_writeFileQueue.isEmpty();
}

bool DFileCopyMoveJobPrivate::checkWritQueueCount()
{
    QMutexLocker lk(&m_copyInfoQueueMutex);
    return m_writeFileQueue.count() > 300;
}

QSharedPointer<DFileCopyMoveJobPrivate::FileCopyInfo> DFileCopyMoveJobPrivate::writeQueueDequeue()
{
    QMutexLocker lk(&m_copyInfoQueueMutex);
    return m_writeFileQueue.dequeue();
}

void DFileCopyMoveJobPrivate::writeQueueEnqueue(const QSharedPointer<DFileCopyMoveJobPrivate::FileCopyInfo> &copyinfo)
{
    QMutexLocker lk(&m_copyInfoQueueMutex);
    m_writeFileQueue.enqueue(copyinfo);
}

void DFileCopyMoveJobPrivate::errorQueueHandling()
{
    if (!stateCheck())
        return;
    bool isLoop = false;
    {
        QMutexLocker lk(&m_errorQueueMutex);
        if (!stateCheck())
            return;
        if ((m_errorQueue.count() <= 0 || m_errorQueue.first() != QThread::currentThreadId()) && stateCheck())
            m_errorQueue.enqueue(QThread::currentThreadId());

        isLoop = m_errorQueue.count() > 1 && m_errorQueue.first() != QThread::currentThreadId();
        qDebug() << "new error comming  " << m_errorQueue << isLoop << QThread::currentThreadId();
    }
    while (isLoop && stateCheck()) {
        QMutex mutex;
        mutex.lock();
        m_errorCondition.wait(&mutex);
        mutex.unlock();
        {
            QMutexLocker lk(&m_errorQueueMutex);
            isLoop = m_errorQueue.count() > 1 && m_errorQueue.first() != QThread::currentThreadId();
        }
    }
}

void DFileCopyMoveJobPrivate::errorQueueHandled(const bool &isNotCancel)
{
    if (!isNotCancel)
        q_ptr->stop();
    QMutexLocker lk(&m_errorQueueMutex);
    if (!m_errorQueue.isEmpty()) {
        m_errorQueue.removeAll(QThread::currentThreadId());
        qDebug() << "error handler over === " << QThread::currentThreadId() << m_errorQueue;
    }
    if (m_errorQueue.count() > 0)
        m_errorCondition.wakeAll();
}

void DFileCopyMoveJobPrivate::releaseCopyInfo(const DFileCopyMoveJobPrivate::FileCopyInfoPointer &info)
{
    if (info->buffer) {
        delete [] info->buffer;
        info->buffer = nullptr;
    }
    for (auto fd : m_writeOpenFd) {
        close(fd);
    }
    m_writeOpenFd.clear();
}

bool DFileCopyMoveJobPrivate::writeRefineThread()
{
    bool ok = true;
    while (checkRefineCopyProccessSate(DFileCopyMoveJob::ReadFileProccessOver)) {
        ok = writeToFileByQueue();
        if (!ok) {
            break;
        }
    }
    if (ok && stateCheck()) {
        ok = writeToFileByQueue();
    } else {
        q_ptr->stop();
    }
    cancelReadFileDealWriteThread();
    qInfo() << "write thread finish!";
    return true;
}

//为考虑线程之间的同步问题（错误同步，假如，读线程出错了，
//1.重试时写线程是不是要全部停住(已暂停)
//2.重试失败，跳过当前文件应该怎么处理（加入一个跳过写入队列，写线程检查当前的写队列，移除都所有的要移除的选项）
//3.重试失败，直接退出拷贝怎么处理（处理所有的写入队列，清理掉）。
//写线程出错时，1.弹出提示框时，是否要暂停读线程2.点击跳过应该怎么处理，清理掉有相同的文件描述符的写队列、关闭文件读文件是否要暂停3.关闭拷贝时，要怎么处理）
bool DFileCopyMoveJobPrivate::writeToFileByQueue()
{
    while (!checkWritQueueEmpty()) {
        auto info = writeQueueDequeue();
        if (Q_UNLIKELY(!stateCheck())) {
            releaseCopyInfo(info);
            return false;
        }
        //检查info的有效性
        if (!info->frominfo || !info->toinfo) {
            releaseCopyInfo(info);
            return false;
        }
        //对文件夹加权
        if (info->isdir) {
            info->handler->setPermissions(info->toinfo->fileUrl(), info->permission); \
            continue;
        }
        if (skipReadFileDealWriteThread(info->frominfo->fileUrl())) {
            completedProgressDataSize += info->size;
            countrefinesize(info->size);
            releaseCopyInfo(info);
            continue;
        }
        int toFd = -1;
        bool isErrorOccur = false;
        //获取目标文件描述符
        if (!m_writeOpenFd.contains(info->toinfo->fileUrl())) {
            DUrl fromUrl = info->frominfo->fileUrl();
            DUrl toUrl = info->toinfo->fileUrl();
            Q_EMIT q_ptr->currentJobChanged(fromUrl, toUrl, false);
            DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
            do {
                std::string path = info->toinfo->fileUrl().path().toStdString();
                toFd = open(path.c_str(), m_openFlag, 0777);
                if (toFd > -1) {
                    m_writeOpenFd.insert(info->toinfo->fileUrl(), toFd);
                    action = DFileCopyMoveJob::NoAction;
                } else {
                    qCDebug(fileJob()) << "open error:" << info->toinfo->fileUrl() << QThread::currentThreadId();
                    DFileCopyMoveJob::Error errortype = (!info->toinfo->exists() || info->toinfo->isWritable()) ? DFileCopyMoveJob::OpenError :
                                                        DFileCopyMoveJob::PermissionError;
                    // task-36496 "Permission denied"没有被翻译 翻译为“没有权限”
                    QString errorstr("");

                    errorstr = (!info->toinfo->exists() || info->toinfo->isWritable()) ?
                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: Permission denied") :
                               QString("Failed to open the file!");
                    isErrorOccur = true;
                    //错误队列处理
                    errorQueueHandling();
                    action = setAndhandleError(errortype, info->toinfo, DAbstractFileInfoPointer(nullptr), errorstr);
                    if (action == DFileCopyMoveJob::RetryAction) {
                        QThread::msleep(THREAD_SLEEP_TIME);
                    }
                }
            } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                                  action == DFileCopyMoveJob::NoAction);
                isErrorOccur = false;
            }
            if (action == DFileCopyMoveJob::SkipAction) {
                countrefinesize(info->size);
                completedProgressDataSize += info->size;
                releaseCopyInfo(info);
                QMutexLocker lk(&m_skipFileQueueMutex);
                m_skipFileQueue.enqueue(info->frominfo->fileUrl());
                continue;
            } else if (action != DFileCopyMoveJob::NoAction) {
                releaseCopyInfo(info);
                return false;
            }
        }
        else {
            toFd = m_writeOpenFd.value(info->toinfo->fileUrl());
        }

        bool bSkiped = false;

write_data: {
            qint64 size_write = write(toFd, info->buffer, static_cast<size_t>(info->size));
            QString errorstr = strerror(errno);
            if (Q_UNLIKELY(!stateCheck())) {
                releaseCopyInfo(info);
                return false;
            }
            //如果写失败了，直接推出
            if (size_write < 0) {
                if (!stateCheck()) {
                    releaseCopyInfo(info);
                    return false;
                }

                //错误队列处理
                if (!isErrorOccur)
                    errorQueueHandling();
                isErrorOccur = true;
                switch (setAndhandleError(DFileCopyMoveJob::WriteError, info->frominfo, info->toinfo,
                                          qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause:").
                                          arg(errorstr))) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!lseek(toFd, info->currentpos, SEEK_SET)) {
                        setError(DFileCopyMoveJob::UnknowError, "");
                        //当前错误处理完成
                        if (isErrorOccur) {
                            errorQueueHandled(false);
                            isErrorOccur = false;
                        }
                        releaseCopyInfo(info);
                        return false;
                    }
                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction: {
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    completedProgressDataSize += info->size;
                    countrefinesize(info->size);
                    releaseCopyInfo(info);

                    //当前错误处理完成
                    if (isErrorOccur) {
                        errorQueueHandled();
                        isErrorOccur = false;
                    }
                    bSkiped = true;

                    QMutexLocker lk(&m_skipFileQueueMutex);
                    m_skipFileQueue.push_back(info->frominfo->fileUrl());
                    break;
                }
                default:
                    //当前错误处理完成
                    if (isErrorOccur) {
                        errorQueueHandled(false);
                        isErrorOccur = false;
                    }
                    releaseCopyInfo(info);

                    return false;
                }
            }

            //当前错误处理完成
            if (isErrorOccur) {
                errorQueueHandled();
                isErrorOccur = false;
            }

            if (bSkiped)
                continue;

            if (Q_UNLIKELY(size_write != info->size)) {
                do {
                    // 在某些情况下（往sftp挂载目录写入），可能一次未能写入那么多数据
                    // 但不代表写入失败，应该继续尝试，直到所有数据全部写入
                    if (size_write > 0) {
                        const char *surplus_data = info->buffer;
                        qint64 surplus_size = info->size;

                        do {
                            currentJobDataSizeInfo.second += size_write;
                            completedDataSize += size_write;

                            surplus_data += size_write;
                            surplus_size -= size_write;

                            size_write = write(toFd, surplus_data, static_cast<size_t>(surplus_size));
                        } while (size_write > 0 && size_write != surplus_size);

                        // 表示全部数据写入完成
                        if (size_write > 0) {
                            break;
                        }
                    }

                    DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::NotEnoughSpaceError;
                    QString errorstr;

                    if (checkFreeSpace(currentJobDataSizeInfo.first - currentJobDataSizeInfo.second)) {
                        errortype = DFileCopyMoveJob::WriteError;
                        errorstr = qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause:").arg(strerror(errno));
                    }

                    switch (setAndhandleError(errortype, info->frominfo, info->toinfo, errorstr)) {
                    case DFileCopyMoveJob::RetryAction: {
                        if (!lseek(toFd, info->currentpos, SEEK_SET)) {
                            setError(DFileCopyMoveJob::UnknowError, "");
                            releaseCopyInfo(info);
                            //当前错误处理完成
                            if (isErrorOccur) {
                                errorQueueHandled(false);
                                isErrorOccur = false;
                            }

                            return false;
                        }

                        goto write_data;
                    }
                    case DFileCopyMoveJob::SkipAction:{
                        releaseCopyInfo(info);

                        //当前错误处理完成
                        if (isErrorOccur) {
                            errorQueueHandled();
                            isErrorOccur = false;
                        }

                        QMutexLocker lk(&m_skipFileQueueMutex);
                        m_skipFileQueue.push_back(info->frominfo->fileUrl());
                        break;
                    }
                    default:
                        //当前错误处理完成
                        releaseCopyInfo(info);
                        if (isErrorOccur) {
                            errorQueueHandled(false);
                            isErrorOccur = false;
                        }
                        return false;
                    }
                } while (false);
            }

            if (bSkiped)
                continue;

            currentJobDataSizeInfo.second += size_write;
            completedDataSize += size_write;
            completedDataSizeOnBlockDevice += size_write;

            countrefinesize(size_write);
            if (info->buffer) {
                delete [] info->buffer;
                info->buffer = nullptr;
            }
        }
        //异步执行同步
        syncfs(toFd);
        //关闭文件并加权
        if (info->closeflag) {
            close(toFd);
            m_writeOpenFd.remove(info->toinfo->fileUrl());
            QSharedPointer<DFileHandler> handler = info->handler ? info->handler :
                                                   QSharedPointer<DFileHandler>(DFileService::instance()->createFileHandler(nullptr, info->frominfo->fileUrl()));
            handler->setFileTime(info->toinfo->fileUrl(), info->frominfo->lastRead(), info->frominfo->lastModified());

            QFileDevice::Permissions permissions = info->frominfo->permissions();
            //! use stat function to read vault file permission.
            QString path = info->frominfo->fileUrl().path();
            if (VaultController::isVaultFile(path)) {
                permissions = VaultController::getPermissions(path);
            } else if (deviceListener->isFileFromDisc(info->frominfo->path())) { // fix bug 52610: 从光盘中复制出来的文件权限为只读，与 ubuntu 策略保持一致，拷贝出来权限为 rw-rw-r--
                permissions |= MasteredMediaController::getPermissionsCopyToLocal();
            }
            if (permissions != 0000)
                handler->setPermissions(info->toinfo->fileUrl(), permissions);
        }
    }
    qDebug() << "write queue over!";
    return true;
}

bool DFileCopyMoveJobPrivate::skipReadFileDealWriteThread(const DUrl &url)
{
    QMutexLocker lk(&m_skipFileQueueMutex);

    return m_skipFileQueue.contains(url);
}


void DFileCopyMoveJobPrivate::cancelReadFileDealWriteThread()
{
    QMutexLocker lk(&m_copyInfoQueueMutex);
    for (auto fd : m_writeOpenFd) {
        close(fd);
    }
    m_writeOpenFd.clear();
    while (!m_writeFileQueue.isEmpty()) {
        auto info = m_writeFileQueue.dequeue();
        if (info->buffer)
            delete[] info->buffer;
    }
}

void DFileCopyMoveJobPrivate::setRefineCopyProccessSate(const DFileCopyMoveJob::RefineCopyProccessSate &stat)
{
    m_copyRefineFlag = stat;
}

bool DFileCopyMoveJobPrivate::checkRefineCopyProccessSate(const DFileCopyMoveJob::RefineCopyProccessSate &stat)
{
    return m_copyRefineFlag < stat;
}

void DFileCopyMoveJobPrivate::setCutTrashData(QVariant fileNameList)
{
    for (DUrl temp : qvariant_cast<DUrlList>(fileNameList)) {
        m_fileNameList.enqueue(temp.path());
    }
}

void DFileCopyMoveJobPrivate::saveCurrentDevice(const DUrl &url, const QSharedPointer<DFileDevice> device)
{
    QMutexLocker lk(&m_currentDeviceMutex);
    m_currentDevice[url] = device;
}

void DFileCopyMoveJobPrivate::removeCurrentDevice(const DUrl &url)
{
    QMutexLocker lk(&m_currentDeviceMutex);
    m_currentDevice.remove(url);
}

void DFileCopyMoveJobPrivate::stopAllDeviceOperation()
{
    QMutexLocker lk(&m_currentDeviceMutex);
    for (auto url : m_currentDevice.keys()) {
        m_currentDevice.value(url)->cancelAllOperate();
    }
    m_currentDevice.clear();
}

void DFileCopyMoveJobPrivate::clearThreadPool()
{
    QMutexLocker lk(&m_threadMutex);
    m_threadInfos.clear();
}

DFileCopyMoveJob::DFileCopyMoveJob(QObject *parent)
    : DFileCopyMoveJob(new DFileCopyMoveJobPrivate(this), parent)
{
}

DFileCopyMoveJob::~DFileCopyMoveJob()
{
    stop();

    Q_D(DFileCopyMoveJob);
    //停止清理掉自己的大文件拷贝
    copyBigFileOnDiskJobRun();

    d->stopAllDeviceOperation();

    d->clearThreadPool();

    d->cancelReadFileDealWriteThread();
    qDebug() << "release  DFileCopyMoveJob!" << this << QThread::currentThread();
}

DFileCopyMoveJob::Handle *DFileCopyMoveJob::errorHandle() const
{
    Q_D(const DFileCopyMoveJob);

    return d->handle;
}

void DFileCopyMoveJob::setErrorHandle(DFileCopyMoveJob::Handle *handle, QThread *threadOfHandle)
{
    Q_D(DFileCopyMoveJob);

    d->handle = handle;
    d->threadOfErrorHandle = threadOfHandle;
}

void DFileCopyMoveJob::setActionOfErrorType(DFileCopyMoveJob::Error error, DFileCopyMoveJob::Action action)
{
    Q_D(DFileCopyMoveJob);
    Q_ASSERT(d->state != RunningState);

    d->actionOfError[error] = action;
}

DFileCopyMoveJob::Mode DFileCopyMoveJob::mode() const
{
    Q_D(const DFileCopyMoveJob);

    return d->mode;
}

DFileCopyMoveJob::State DFileCopyMoveJob::state() const
{
    Q_D(const DFileCopyMoveJob);

    return static_cast<State>(d->state.load());
}

DFileCopyMoveJob::Error DFileCopyMoveJob::error() const
{
    Q_D(const DFileCopyMoveJob);

    return d->error;
}

DFileCopyMoveJob::FileHints DFileCopyMoveJob::fileHints() const
{
    Q_D(const DFileCopyMoveJob);

    return d->fileHints;
}

QString DFileCopyMoveJob::errorString() const
{
    Q_D(const DFileCopyMoveJob);

    return d->errorString;
}

DUrlList DFileCopyMoveJob::sourceUrlList() const
{
    Q_D(const DFileCopyMoveJob);

    return d->sourceUrlList;
}

DUrlList DFileCopyMoveJob::targetUrlList() const
{
    Q_D(const DFileCopyMoveJob);

    return d->targetUrlList;
}

DUrl DFileCopyMoveJob::targetUrl() const
{
    Q_D(const DFileCopyMoveJob);

    return d->targetUrl;
}

bool DFileCopyMoveJob::fileStatisticsIsFinished() const
{
    Q_D(const DFileCopyMoveJob);

    return d->fileStatistics->isFinished();
}

qint64 DFileCopyMoveJob::totalDataSize() const
{
    Q_D(const DFileCopyMoveJob);

    if (d->m_isFileOnDiskUrls) {
        if (!d->m_isCountSizeOver) {
            return -1;
        }
        return d->totalsize;
    }

    if (d->fileStatistics->isRunning()) {
        return -1;
    }

    return d->fileStatistics->totalSize();
}

int DFileCopyMoveJob::totalFilesCount() const
{
    Q_D(const DFileCopyMoveJob);

    if (d->fileStatistics->isRunning()) {
        return -1;
    }

    return d->fileStatistics->filesCount() + d->fileStatistics->directorysCount();
}

QList<QPair<DUrl, DUrl>> DFileCopyMoveJob::completedFiles() const
{
    Q_D(const DFileCopyMoveJob);
    Q_ASSERT(d->state != RunningState);

    return d->completedFileList;
}

QList<QPair<DUrl, DUrl>> DFileCopyMoveJob::completedDirectorys() const
{
    Q_D(const DFileCopyMoveJob);
    Q_ASSERT(d->state != RunningState);

    return d->completedDirectoryList;
}
//获取当前是否可以显示进度条
bool DFileCopyMoveJob::isCanShowProgress() const
{
    Q_D(const DFileCopyMoveJob);
    return d->m_isNeedShowProgress;
}

void DFileCopyMoveJob::setRefine(const RefineState &refinestat)
{
    Q_D(DFileCopyMoveJob);

    d->m_refineStat = refinestat;
}

void DFileCopyMoveJobPrivate::waitRefineThreadFinish()
{
    qDebug() << "wait thread pool finished!";
    while (m_pool.activeThreadCount() > 0) {
        if (state == DFileCopyMoveJob::StoppedState)
            q_ptr->stop();
        QThread::msleep(50);
    }
    qDebug() << "wait write thread finished!";
    if (m_isWriteThreadStart.load()) {
        while (!m_writeResult.isFinished()) {
            if (state == DFileCopyMoveJob::StoppedState) {
                cancelReadFileDealWriteThread();
                m_writeResult.waitForFinished();
                return;
            }
            QThread::msleep(50);
        }
    }
    qDebug() << "add Permisson to dir!";
    for (auto info : m_dirPermissonList)
    {
        info->handler->setPermissions(info->target, info->permission);
    }
}

void DFileCopyMoveJobPrivate::setLastErrorAction(const DFileCopyMoveJob::Action &action)
{
    QMutexLocker lk(&m_lastErrorHandleActionMutex);
    m_lastErrorHandleAction.insert(QThread::currentThread(), action);
}

DFileCopyMoveJob::Action DFileCopyMoveJobPrivate::getLastErrorAction()
{
    QMutexLocker lk(&m_lastErrorHandleActionMutex);
    QThread *current = QThread::currentThread();
    if (m_lastErrorHandleAction.contains(current) )
        return m_lastErrorHandleAction.value(current);
    return DFileCopyMoveJob::NoAction;
}
/*!
 * \brief DFileCopyMoveJobPrivate::reopenGvfsFiles 拷贝文件到网络文件或者从网络文件上拷贝文件，
 * 在拷贝途中断网再恢复后才能调用，不然就会卡死。主要功能是调整断点续传时重新打开远程文件，调整文件位置，方便再次拷贝写入
 * \param fromInfo 源文件的信息
 * \param toInfo 目标文件的信息
 * \param fromDevice 源文件的iodevice
 * \param toDevice 目标文件的iodevice
 * \param isWriteError 是否是写时错误
 * \return qint64 返回当前文件拷贝到的位置
 */
qint64 DFileCopyMoveJobPrivate::reopenGvfsFiles(const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo, QSharedPointer<DFileDevice> &fromDevice,
                                               QSharedPointer<DFileDevice> &toDevice, const bool &isWriteError)
{
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    if (isWriteError) {
        toInfo->refresh(true);
        toDevice->close();
        action = openGvfsFile(toInfo, toDevice, (QIODevice::WriteOnly | QIODevice::Truncate));
    } else {
        fromInfo->refresh(true);
        toInfo->refresh(true);
        fromDevice->close();
        action = openGvfsFile(fromInfo, fromDevice, QIODevice::ReadOnly);
    }

    if (action == DFileCopyMoveJob::SkipAction) {
        return -1;
    } else if (action != DFileCopyMoveJob::NoAction) {
        return -2;
    }
    fromInfo->refresh(true);
    toInfo->refresh(true);
    m_gvfsFileInnvliadProgress = const_cast<DFileCopyMoveJobPrivate*>(this)->getCompletedDataSize();
    qint64 currentPos = 0;
    if (isWriteError) {
        action = seekFile(fromInfo, fromDevice, currentPos);
    } else {
        action = seekFile(toInfo, toDevice, currentPos);
    }

    if (action == DFileCopyMoveJob::SkipAction) {
        return -1;
    } else if (action != DFileCopyMoveJob::NoAction) {
        return -2;
    }

    return currentPos;
}

/*!
 * \brief DFileCopyMoveJobPrivate::seekFile seek文件指针到指定位置，错误就阻塞弹窗重试就继续
 *  需要注意的是调用此接口必须是文件打开的情况
 * \param fileInfo 文件信息
 * \param device 文件的iodevice
 * \param pos 要seek的位置
 * \return DFileCopyMoveJob::Action 拷贝操作
 */
DFileCopyMoveJob::Action DFileCopyMoveJobPrivate::seekFile(const DAbstractFileInfoPointer &fileInfo,
                                                               QSharedPointer<DFileDevice> &device, const qint64 &pos)
{
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    bool isErrorOccur = false;
    do {
        if (device->seek(pos)) {
            action = DFileCopyMoveJob::NoAction;
        } else {
            QString errorstr = qApp->translate("DFileCopyMoveJob", "Failed to position the file pointer, cause: %1").arg(device->errorString());
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::OpenError;
            action = setAndhandleError(errortype, fileInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

    //当前错误处理完成
    if (isErrorOccur) {
        errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                          action == DFileCopyMoveJob::NoAction);
        isErrorOccur = false;
    }
    return action;
}

/*!
 * \brief DFileCopyMoveJobPrivate::openGvfsFile 打开文件，错误就阻塞弹窗重试就继续
 * \param fileInfo 文件信息
 * \param device 文件的iodevice
 * \param flags 打开文件的描述符
 * \return DFileCopyMoveJob::Action 拷贝操作
 */
DFileCopyMoveJob::Action DFileCopyMoveJobPrivate::openGvfsFile(const DAbstractFileInfoPointer &fileInfo,
                                                               QSharedPointer<DFileDevice> &device, const QIODevice::OpenMode &flags)
{
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    bool isErrorOccur = false;
    do {
        if (device->open(flags)) {
            action = DFileCopyMoveJob::NoAction;
        } else {
            qCDebug(fileJob()) << "open error:" << fileInfo->fileUrl() << QThread::currentThreadId();
            DFileCopyMoveJob::Error errortype = (!fileInfo->exists() || fileInfo->isWritable()) ?
                        DFileCopyMoveJob::OpenError :
                        DFileCopyMoveJob::PermissionError;
            // task-36496 "Permission denied"没有被翻译 翻译为“没有权限”
            QString errorstr("");
            if ("Permission denied" == device->errorString()) {
                errorstr = (!fileInfo->exists() || fileInfo->isWritable()) ?
                           qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: Permission denied") :
                           QString();
            } else {
                errorstr = (!fileInfo->exists() || fileInfo->isWritable()) ?
                           qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(device->errorString()) :
                           QString();
            }
            isErrorOccur = true;
            //错误队列处理
            errorQueueHandling();
            action = setAndhandleError(errortype, fileInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

    //当前错误处理完成
    if (isErrorOccur) {
        errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                          action == DFileCopyMoveJob::NoAction);
        isErrorOccur = false;
    }
    return action;
}
/*!
 * \brief DFileCopyMoveJobPrivate::cleanCopySources 清理掉拷贝失败后的资源
 * \param data 拷贝时的缓存buffer
 * \param fromDevice 源文件的iodevice
 * \param toDevice 目标文件的iodevice
 * \param isError 是否正在处理错误
 */
void DFileCopyMoveJobPrivate::cleanCopySources(char *data, const QSharedPointer<DFileDevice> &fromDevice,
                                               const QSharedPointer<DFileDevice> &toDevice, bool &isError)
{
    delete[] data;
    data = nullptr;
    fromDevice->close();
    toDevice->close();
    //当前错误处理完成
    if (isError) {
        errorQueueHandled();
        isError = false;
    }
}
/*!
 * \brief DFileCopyMoveJobPrivate::gvfsFileRetry 处理smb文件在拷贝过程中断网，网络恢复后点击重试处理
 * \param data 拷贝的data
 * \param isErrorOccur 是否当前在错误处理
 * \param currentPos 当前的文件文职
 * \param fromInfo 源文件的文件信息
 * \param toInfo 目标文件的文件信息
 * \param fromDevice 源文件的iodevice
 * \param toDevice 目标文件的iodevice
 * \param isWriteError 是写遇到错误重试
 * \return DFileCopyMoveJob::GvfsRetryType 网络文件重试的操作
 */
DFileCopyMoveJob::GvfsRetryType DFileCopyMoveJobPrivate::gvfsFileRetry(char *data, bool &isErrorOccur, qint64 &currentPos, const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo, QSharedPointer<DFileDevice> &fromDevice, QSharedPointer<DFileDevice> &toDevice, const bool &isWriteError)
{
    if (fromInfo->isGvfsMountFile()
            && !DFileService::instance()->checkGvfsMountfileBusy(toInfo->fileUrl(), false)) {
        currentPos = reopenGvfsFiles(fromInfo, toInfo, fromDevice, toDevice, isWriteError);
        if (currentPos == -1) {
            cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
            return DFileCopyMoveJob::GvfsRetrySkipAction;
        } else if (currentPos == -2) {
            cleanCopySources(data, fromDevice, toDevice, isErrorOccur);
            return DFileCopyMoveJob::GvfsRetryCancelAction;
        } else {
            DUrl fromurl = fromInfo->fileUrl();
            DUrl tourl = toInfo->fileUrl();
            {
                QMutexLocker lk(&m_emitUrlMutex);
                m_emitUrl.insert(fromurl,tourl);
                Q_EMIT q_ptr->currentJobChanged(m_emitUrl.lastKey(), m_emitUrl.last(), false);
            }
            return DFileCopyMoveJob::GvfsRetryNoAction;
        }
    }
    return DFileCopyMoveJob::GvfsRetryDefault;
}
/*!
 * \brief DFileCopyMoveJobPrivate::readAheadSourceFile 预读源文件
 * \param fromInfo 源文件的文件信息
 */
void DFileCopyMoveJobPrivate::readAheadSourceFile(const DAbstractFileInfoPointer &fromInfo)
{
    if (!fromInfo)
        return;
    std::string stdStr = fromInfo->fileUrl().path().toUtf8().toStdString();
    int fromfd = open(stdStr.data(), O_RDONLY);
    if (-1 != fromfd) {
        readahead(fromfd, 0, static_cast<size_t>(fromInfo->size()));
        close(fromfd);
    }
}
/*!
 * \brief DFileCopyMoveJobPrivate::handleUnknowUrlError 阻塞处理UnknowUrlError的错误
 * \param fromInfo 源文件的文件信息
 * \param toInfo 目标文件的文件信息
 * \return
 */
bool DFileCopyMoveJobPrivate::handleUnknowUrlError(const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo)
{
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    //错误队列处理
    errorQueueHandling();
    action = setAndhandleError(DFileCopyMoveJob::UnknowUrlError, fromInfo, toInfo, "Failed on create file device");
    errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                      action == DFileCopyMoveJob::NoAction);
    if (action == DFileCopyMoveJob::SkipAction) {
        return true;
    } else {
        return false;
    }
}
/*!
 * \brief DFileCopyMoveJobPrivate::handleUnknowError 阻塞处理UnknowError的错误
 * \param fromInfo 源文件的文件信息
 * \param toInfo 目标文件的文件信息
 * \return
 */
bool DFileCopyMoveJobPrivate::handleUnknowError(const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo, const QString &errorStr)
{
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    //错误队列处理
    errorQueueHandling();
    action = setAndhandleError(DFileCopyMoveJob::UnknowError, fromInfo, toInfo, errorStr);
    errorQueueHandled(action == DFileCopyMoveJob::SkipAction ||
                      action == DFileCopyMoveJob::NoAction);
    if (action == DFileCopyMoveJob::SkipAction) {
        return true;
    } else {
        return false;
    }
}
/*!
 * \brief DFileCopyMoveJobPrivate::sendCopyInfo 发送当前拷贝的信息
 * \param fromInfo 源文件的信息
 * \param toInfo 目标文件的信息
 */
void DFileCopyMoveJobPrivate::sendCopyInfo(const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo)
{
    //多线程拷贝时发送当前拷贝信息
    if (m_refineStat != DFileCopyMoveJob::NoRefine){
        DUrl fromurl = fromInfo ? fromInfo->fileUrl() : DUrl();
        DUrl tourl = toInfo ? toInfo->fileUrl() : DUrl();
        QMutexLocker lk(&m_emitUrlMutex);
        m_emitUrl.insert(fromurl,tourl);
        Q_EMIT q_ptr->currentJobChanged(m_emitUrl.lastKey(), m_emitUrl.last(), false);
    }
}

void DFileCopyMoveJobPrivate::cleanDoCopyFileSource(char *data,const DAbstractFileInfoPointer &fromInfo,const DAbstractFileInfoPointer &toInfo, const QSharedPointer<DFileDevice> &fromDevice, const QSharedPointer<DFileDevice> &toDevice)
{
    if (!fromInfo || !toInfo || !toDevice || !fromDevice) {
        qWarning() << "clean doCopyFile fun source failed! target url = " << targetUrl;
        return;
    }
    // fix bug 63290 只有网络文件断开了网络才使用这个函数
    if (toInfo->isGvfsMountFile() && DFileService::instance()->checkGvfsMountfileBusy(toInfo->fileUrl(), false)) {
        toDevice->closeWriteReadFailed(true);
    } else {
        toDevice->close();
    }
    if (fromInfo->isGvfsMountFile() && DFileService::instance()->checkGvfsMountfileBusy(fromInfo->fileUrl(), false)) {
        fromDevice->closeWriteReadFailed(true);
    } else {
        fromDevice->close();
    }
    delete[] data;
    data = nullptr;
}

void DFileCopyMoveJobPrivate::initRefineState()
{
    // 不是优化就直接退出
    if (m_refineStat == DFileCopyMoveJob::NoRefine)
        return;
    // 不是拷贝mode就直接为NoRefine，走以前
    if (mode != DFileCopyMoveJob::CopyMode) {
        m_refineStat = DFileCopyMoveJob::NoRefine;
        return;
    }
    // 拷贝到移动设备
    if (m_isFileOnDiskUrls && !m_bDestLocal && m_isTagFromBlockDevice.load()) {
        m_refineStat = DFileCopyMoveJob::RefineBlock;
        return;
    }
    // 和系统同磁盘的目录
    if (m_isFileOnDiskUrls && m_bDestLocal) {
        m_refineStat = DFileCopyMoveJob::RefineLocal;
        return;
    }
    // 其他都走以前的流程
    m_refineStat = DFileCopyMoveJob::NoRefine;
    return;
}

//! 用于保存回收站剪切出去的文件在回收站的原始路径
void DFileCopyMoveJob::setCurTrashData(QVariant fileNameList)
{
    Q_D(DFileCopyMoveJob);
    d->setCutTrashData(fileNameList);
}

void DFileCopyMoveJob::setProgressShow(const bool &isShow)
{
    Q_D(DFileCopyMoveJob);
    d->m_isProgressShow.store(isShow);
}

void DFileCopyMoveJob::copyBigFileOnDiskJobWait()
{
    Q_D(DFileCopyMoveJob);

    d->m_isCopyLargeFileOnDiskWait = true;
}

void DFileCopyMoveJob::copyBigFileOnDiskJobRun()
{
    Q_D(DFileCopyMoveJob);

    d->m_isCopyLargeFileOnDiskWait = false;
    d->m_waitConditionCopyLargeFileOnDisk.wakeAll();
}

DFileCopyMoveJob::Actions DFileCopyMoveJob::supportActions(DFileCopyMoveJob::Error error)
{
    switch (error) {
    case PermissionError:
    case OpenError:
    case ReadError:
    case WriteError:
    case SymlinkError:
    case MkdirError:
    case ResizeError:
    case RemoveError:
    case RenameError:
    case SeekError:
    case IntegrityCheckingError:
        return SkipAction | RetryAction | CancelAction;
    case UnknowError:
    case SymlinkToGvfsError:
    case SpecialFileError:
        return SkipAction | CancelAction;
    case FileSizeTooBigError:
        return SkipAction | CancelAction | EnforceAction;
    case NotEnoughSpaceError:
        return SkipAction | RetryAction | CancelAction | EnforceAction;
    case FileExistsError:
        return SkipAction | ReplaceAction | CoexistAction | CancelAction;
    case DirectoryExistsError:
        return SkipAction | MergeAction | CoexistAction | CancelAction;
    case TargetReadOnlyError:
        return SkipAction | EnforceAction;
    case TargetIsSelfError:
        return SkipAction | EnforceAction;
    default:
        break;
    }

    return CancelAction;
}

void DFileCopyMoveJob::start(const DUrlList &sourceUrls, const DUrl &targetUrl)
{
    Q_ASSERT(!isRunning());
    Q_D(DFileCopyMoveJob);

    if (d->mode == UnknowMode) {
        qInfo() << "error mode UnknowMode!";
        return;
    }

    d->sourceUrlList = sourceUrls;
    d->targetUrl = targetUrl;
    d->m_isFileOnDiskUrls = sourceUrls.isEmpty() ? true :
                                                   FileUtils::isFileOnDisk(sourceUrls.first().path());
    if (!d->m_isFileOnDiskUrls) {
        if (d->fileStatistics->isRunning()) {
            d->fileStatistics->stop();
            d->fileStatistics->wait();
        }
        d->fileStatistics->start(sourceUrls);
    }

    // DFileStatisticsJob 统计数量很慢，自行统计
    QPointer<DFileCopyMoveJob> dp = this;
    QtConcurrent::run([sourceUrls, dp, d]() {
        if (dp.isNull())
            return;
        if (d->mode == MoveMode || d->mode == CutMode) {
            d->countStatisticsFinished = false;
            for (const auto &url : sourceUrls) {
                QStringList list;
                FileUtils::recurseFolder(url.toLocalFile(), "", &list);
                if (!dp.isNull())
                    d->totalMoveFilesCount += (list.size() + 1); // +1 的目的是当前选中的目录要统计到
                else
                    break;
            }
            if (!dp.isNull())
                d->countStatisticsFinished = true;
        }
    });

    QThread::start();
}

void DFileCopyMoveJob::stop()
{

    Q_D(DFileCopyMoveJob);

    QMutexLocker lk(&d->m_stopMutex);

    if (d->state == StoppedState) {
        return;
    }

    d->fileStatistics->stop();

    QMetaObject::invokeMethod(d->updateSpeedTimer, "stop");

    d->setState(StoppedState);
    d->waitCondition.wakeAll();
    //停止清理掉自己的大文件拷贝
    copyBigFileOnDiskJobRun();

    //清理所有的拷贝小文件的错误队列
    {
        QMutexLocker lk(&d->m_errorQueueMutex);
        d->m_errorQueue.clear();
        d->m_errorCondition.wakeAll();
    }

    d->stopAllDeviceOperation();

    d->clearThreadPool();

    d->cancelReadFileDealWriteThread();
}

void DFileCopyMoveJob::togglePause()
{
    Q_D(DFileCopyMoveJob);

    if (d->state == StoppedState) {
        return;
    }

    d->updateProgress();

    d->fileStatistics->togglePause();

    if (d->state == PausedState) {
        d->setState(RunningState);
        d->waitCondition.wakeAll();
    } else {
        d->setState(PausedState);
    }
}

void DFileCopyMoveJob::setMode(DFileCopyMoveJob::Mode mode)
{
    Q_ASSERT(!isRunning());
    Q_D(DFileCopyMoveJob);

    d->mode = mode;
}

void DFileCopyMoveJob::setFileHints(FileHints fileHints)
{
    Q_D(DFileCopyMoveJob);
    Q_ASSERT(d->state != RunningState);

    d->fileHints = fileHints;
    d->fileStatistics->setFileHints(fileHints.testFlag(FollowSymlink) ? DFileStatisticsJob::FollowSymlink : DFileStatisticsJob::FileHints());
}

void DFileCopyMoveJob::taskDailogClose()
{
    Q_D(DFileCopyMoveJob);

    d->m_bTaskDailogClose = true;
}

DFileCopyMoveJob::DFileCopyMoveJob(DFileCopyMoveJobPrivate *dd, QObject *parent)
    : QThread(parent)
    , d_d_ptr(dd)
{
    dd->fileStatistics = new DFileStatisticsJob(this);
    dd->updateSpeedTimer = new QTimer(this);

    connect(dd->fileStatistics, &DFileStatisticsJob::finished, this, &DFileCopyMoveJob::fileStatisticsFinished, Qt::DirectConnection);
    connect(dd->updateSpeedTimer, SIGNAL(timeout()), this, SLOT(_q_updateProgress()), Qt::DirectConnection);
}

void DFileCopyMoveJob::run()
{
    Q_D(DFileCopyMoveJob);

    //记录关键信息
    qInfo() << "start job, mode:" << d->mode << "file url list:" << d->sourceUrlList << ", target url:" << d->targetUrl;
    qint64 timesec = QDateTime::currentMSecsSinceEpoch();
    d->m_sart = timesec;
    d->unsetError();
    d->setState(RunningState);
    //远程下载
    if (d->mode == RemoteMode) {
        d->sourceUrlList = DUrl::fromQUrlList(DFMGlobal::instance()->getRemoteUrls());
        qInfo() << "remote copy source urls list:" << d->sourceUrlList;
        d->mode = CopyMode;
    }

    // 本地文件使用 countAllCopyFile 统计大小非常快, 因此不必开辟线程去统计大小. 同步等待文件大小统计完成
    // 网络文件使用以下方式反而会更慢, 因此使用线程统计类
    if (d->targetUrl.isValid() && d->m_isFileOnDiskUrls) {
        d->totalsize = FileUtils::totalSize(d->sourceUrlList,d->m_currentDirSize,d->totalfilecount);
        d->m_isCountSizeOver = true;
        emit fileStatisticsFinished();
    }

    d->completedDirectoryList.clear();
    d->completedFileList.clear();
    d->targetUrlList.clear();
    d->completedDataSize = 0;
    d->completedDataSizeOnBlockDevice = 0;
    d->completedFilesCount = 0;
    d->tid = qt_gettid();

    DAbstractFileInfoPointer target_info;
    bool mayExecSync = false;
    QPointer<DFileCopyMoveJob> me = this;

    if (d->targetUrl.isValid()) {
        target_info = DFileService::instance()->createFileInfo(nullptr, d->targetUrl, false);

        if (!target_info) {
            d->setError(UnknowUrlError);
            stop();
            goto end;
        }

        if (!target_info->exists()) {
            d->setError(NonexistenceError, "The target directory non-exists or not permission");
            stop();
            goto end;
        }

        if (!target_info->isDir()) {
            d->setError(UnknowError, "The target url is not directory");
            stop();
            goto end;
        }

        // reset
        d->canUseWriteBytes = 0;
        d->targetIsRemovable = 0;
        d->targetLogSecionSize = 512;
        d->targetDeviceStartSectorsWritten = -1;
        d->targetSysDevPath.clear();
        d->targetRootPath.clear();
        d->m_isTagGvfsFile = target_info->isGvfsMountFile();

        //检查是否需要每次读写都去同步
        d->checkTagetNeedSync();
        //检查目标目录是否是块设备
        d->checkTagetIsFromBlockDevice();


        QScopedPointer<DStorageInfo> targetStorageInfo(DFileService::instance()->createStorageInfo(nullptr, d->targetUrl));

        if (targetStorageInfo) {
            d->targetRootPath = targetStorageInfo->rootPath();
            QString rootpath = d->targetRootPath;
            d->m_bDestLocal = FileUtils::isFileOnDisk(rootpath);

            qCDebug(fileJob(), "Target block device: \"%s\", Root Path: \"%s\"",
                    targetStorageInfo->device().constData(), qPrintable(d->targetRootPath));

            if (targetStorageInfo->isLocalDevice()) {
                d->canUseWriteBytes = targetStorageInfo->fileSystemType().startsWith("ext");
                if (targetStorageInfo->fileSystemType().startsWith("vfat")
                        || targetStorageInfo->fileSystemType().startsWith("ntfs")
                        || targetStorageInfo->fileSystemType().startsWith("btrfs")
                        || targetStorageInfo->fileSystemType().startsWith("fuseblk"))
                    d->m_openFlag = d->m_openFlag | O_DIRECT;

                if (d->canUseWriteBytes && !d->m_bDestLocal)
                    d->m_refineStat = NoRefine;

                if (!d->canUseWriteBytes) {
                    const QByteArray dev_path = targetStorageInfo->device();

                    QProcess process;

                    process.start("lsblk", {"-niro", "MAJ:MIN,HOTPLUG,LOG-SEC", dev_path}, QIODevice::ReadOnly);

                    if (process.waitForFinished(3000)) {
                        if (process.exitCode() == 0) {
                            const QByteArray &data = process.readAllStandardOutput();
                            const QByteArrayList &list = data.split(' ');

                            qCDebug(fileJob(), "lsblk result data: \"%s\"", data.constData());

                            if (list.size() == 3) {
                                d->targetSysDevPath = "/sys/dev/block/" + list.first();
                                d->targetIsRemovable = list.at(1) == "1";

                                bool ok = false;
                                d->targetLogSecionSize = static_cast<qint16>(list.at(2).toInt(&ok));

                                if (!ok) {
                                    d->targetLogSecionSize = 512;

                                    qCWarning(fileJob(),);
                                }

                                if (d->targetIsRemovable) {
                                    d->targetDeviceStartSectorsWritten = d->getSectorsWritten();
                                }

                                qCDebug(fileJob(), "Block device path: \"%s\", Sys dev path: \"%s\", Is removable: %d, Log-Sec: %d",
                                        qPrintable(dev_path), qPrintable(d->targetSysDevPath), bool(d->targetIsRemovable), d->targetLogSecionSize);
                            } else {
                                qCWarning(fileJob(), "Failed on parse the lsblk result data, data: \"%s\"", data.constData());
                            }
                        } else {
                            qCWarning(fileJob(), "Failed on exec lsblk command, exit code: %d, error message: \"%s\"", process.exitCode(), process.readAllStandardError().constData());
                        }
                    }
                }

                qCDebug(fileJob(), "canUseWriteBytes = %d, targetIsRemovable = %d", bool(d->canUseWriteBytes), bool(d->targetIsRemovable));
            }
        }
    } else if (d->mode == CopyMode || d->mode == CutMode) {
        d->setError(UnknowError, "Invalid target url");
        goto end;
    } else {
        // remove mode
        qCDebug(fileJob(), "remove mode");
    }
    //初始化优化状态
    d->initRefineState();

    for (DUrl &source : d->sourceUrlList) {
        if (!d->stateCheck()) {
            goto end;
        }

        // fix: 搜索列表中的文件路径需要转化为原始路径
        if (source.isSearchFile()) {
            source = source.searchedFileUrl();
        }

        //! fix: 将保险箱路径转换为本地路径
        if (source.isVaultFile()) {
            source = VaultController::vaultToLocalUrl(source);
        }

        //! fix: 将avfs转化为本地路径
        if (source.isAVFSFile()) {
            source = AVFSFileController::realUrl(source);
        }

        const DAbstractFileInfoPointer &source_info = DFileService::instance()->createFileInfo(nullptr, source, false);
        if (!source_info) {
            qWarning() << "Url not yet supported: " << source;
            continue;
        }
        const DUrl &parent_url = source_info->parentUrl();
        bool enter_dir = d->targetUrl.isValid() && (d->directoryStack.isEmpty() || d->directoryStack.top().url.first != parent_url);

        if (enter_dir) {
            d->enterDirectory(source, d->targetUrl);
        }
        if (!d->process(source, source_info, target_info)) {
            stop();
            goto end;
        }

        // 只要有一个不是skip或者cancel就执行一下sync
        if (!mayExecSync && d->getLastErrorAction() != SkipAction &&
                d->getLastErrorAction() != CancelAction) {
            mayExecSync = true;
        }

        if (enter_dir) {
            d->leaveDirectory();
        }

        DUrl target_url;

        if (!d->completedFileList.isEmpty()) {
            if (d->completedFileList.last().first == source) {
                target_url = d->completedFileList.last().second;
            }
        }

        if (!d->completedDirectoryList.isEmpty()) {
            if (d->completedDirectoryList.last().first == source) {
                target_url = d->completedDirectoryList.last().second;
            }
        }

        d->targetUrlList << target_url;

        Q_EMIT finished(source, target_url);

    }
    if (d->m_refineStat == NoRefine)
        d->setError(NoError);

end:
    //设置优化拷贝线程结束
    d->setRefineCopyProccessSate(ReadFileProccessOver);
    //等待线程池结束,等待异步写线程结束
    d->waitRefineThreadFinish();

    if (!d->m_bDestLocal && d->targetIsRemovable && mayExecSync &&
            d->state != DFileCopyMoveJob::StoppedState) { //主动取消时state已经被设置为stop了
        // 任务完成后执行 sync 同步数据到硬盘, 同时将状态改为 SleepState，用于定时器更新进度和速度信息
        if (d->m_isVfat) {
            d->setState(IOWaitState);
            int syncRet = 0;
            d->m_syncResult = QtConcurrent::run([me, &d, &syncRet]() {
                //! 外设或远程设备在同步数据时发送sendDataSyncing信号在拷贝任务对话框中显示数据同步种与即将完成
                Q_EMIT me->sendDataSyncing(tr("Syncing data"), tr("Please wait"));
                qInfo() << "sync to block disk and target path = " << d->targetRootPath;
                syncRet = QProcess::execute("sync", {"-f", d->targetRootPath});
            });
            // 检测同步时是否被停止，若停止则立即跳出
            while (!d->m_syncResult.isFinished()) {
                if (d->state == DFileCopyMoveJob::StoppedState) {
                    qDebug() << "stop sync";
                    goto end;
                }
                QThread::msleep(10);
            }

            // 同步结果检查只针对拷贝
            if (d->mode == CopyMode && syncRet != 0) {
                DFileCopyMoveJob::Action action = d->setAndhandleError(DFileCopyMoveJob::OpenError, target_info, DAbstractFileInfoPointer(nullptr),
                                                                       "Failed to synchronize to disk u!");

                if (action == DFileCopyMoveJob::RetryAction) {
                    goto end;
                }
            }
            // 恢复状态
            if (d->state == IOWaitState) {
                d->setState(RunningState);
            }
        }
        else if (d->mode == CopyMode){
            while (d->state != DFileCopyMoveJob::StoppedState && d->lastProgress < 1) {
                QThread::msleep(100);
            }
        }
    }

    d->fileStatistics->stop();
    d->setState(StoppedState);

    if (d->error == NoError) {
        d->updateSpeedTimer->stop();
        Q_EMIT progressChanged(1, d->completedDataSize);
        // fix bug 62822 如果进度条显示了，才沉睡0.3秒，来显示进度都100%
        if (d->m_isProgressShow.load())
            QThread::msleep(300);

    }

    qInfo() << "job finished, error:" << error() << ", message:" << errorString() << QDateTime::currentMSecsSinceEpoch() - timesec;
}

QString DFileCopyMoveJob::Handle::getNewFileName(DFileCopyMoveJob *job, const DAbstractFileInfoPointer sourceInfo)
{
    Q_UNUSED(job)

    static QMutex mutex;
    QMutexLocker lk(&mutex);

    return job->d_func()->formatFileName(sourceInfo->fileName());
}

QString DFileCopyMoveJob::Handle::getNonExistsFileName(const DAbstractFileInfoPointer sourceInfo, const DAbstractFileInfoPointer targetDirectory)
{
    return DFileCopyMoveJobPrivate::getNewFileName(sourceInfo, targetDirectory);
}

DFM_END_NAMESPACE

#include "moc_dfilecopymovejob.cpp"
