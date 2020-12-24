/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "interfaces/dfmstandardpaths.h"
#include "shutil/fileutils.h"
#include "dgiofiledevice.h"
#include "deviceinfo/udisklistener.h"
#include "app/define.h"

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

#if defined(Q_OS_LINUX) && (defined(__GLIBC__) || QT_HAS_INCLUDE(<sys/syscall.h>))
#  include <sys/syscall.h>

# if defined(Q_OS_ANDROID) && !defined(SYS_gettid)
#  define SYS_gettid __NR_gettid
# endif


#define MAX_BUFFER_LEN 1024 * 1024 * 1
#define BIG_FILE_SIZE 500 * 1024 * 1024
#define THREAD_SLEEP_TIME 200

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
    m_pool.setMaxThreadCount(8);
}

DFileCopyMoveJobPrivate::~DFileCopyMoveJobPrivate()
{
    qDebug() << "DFileCopyMoveJobPrivate " << QDateTime::currentMSecsSinceEpoch() - m_sart;
    while (!m_writeResult.isFinished()) {
        qDebug() << "DFileCopyMoveJobPrivate all thread over ooo" << QDateTime::currentMSecsSinceEpoch() - m_sart;
        qApp->processEvents();
        QThread::msleep(50);
    }
    if (updateSpeedElapsedTimer) {
        delete updateSpeedElapsedTimer;
    }
    if (fileStatistics) {
        fileStatistics->stop();
        fileStatistics->deleteLater();
        fileStatistics = nullptr;
    }
    m_pool.clear();
    if (updateSpeedTimer) {
        updateSpeedTimer->deleteLater();
        updateSpeedTimer = nullptr;
    }
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
        qCDebug(fileJob()) << "new error, type=" << e << ", message=" << es;
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
    if (error == DFileCopyMoveJob::NoError) {
        lastErrorHandleAction = DFileCopyMoveJob::NoAction;
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
            QMetaObject::invokeMethod(updateSpeedTimer, "stop");
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
//    setPauseThreadPool(true);
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
    QMutexLocker lk(&m_errorMutex);
    if ((DFileCopyMoveJob::FileExistsError == e || DFileCopyMoveJob::DirectoryExistsError == e)
            && (sourceInfo->fileUrl() == targetInfo->fileUrl() || DStorageInfo::isSameFile(sourceInfo->fileUrl().path(), targetInfo->fileUrl().path()))) {
        return DFileCopyMoveJob::CoexistAction;
    }
    setError(e, es);
    if (DFileCopyMoveJob::NoError == e) {
        return DFileCopyMoveJob::NoAction;
    }
    if (DFileCopyMoveJob::CancelError == e) {
        return DFileCopyMoveJob::CancelAction;
    }
    return handleError(sourceInfo, targetInfo);
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
        qCDebug(fileJob()) << "Will be suspended";
        Q_EMIT q->stateChanged(DFileCopyMoveJob::PausedState);
        if (!jobWait()) {
            setError(DFileCopyMoveJob::CancelError);
            qCDebug(fileJob()) << "Will be abort";

            return false;
        }
    } else if (state == DFileCopyMoveJob::StoppedState) {
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
    if(filename.contains(QRegularExpression(".7z.[0-9]{3,10}$")))
    {
        file_base_name = filename.left(filename.indexOf(QRegularExpression(".7z.[0-9]{3,10}$")));
        suffix = filename.mid(filename.indexOf(QRegularExpression(".7z.[0-9]{3,10}$"))+1);
    }

    int number = 0;

    QString new_file_name;

    do {
        new_file_name = number > 0 ? QString("%1(%2 %3)").arg(file_base_name, copy_text).arg(number) : QString("%1(%2)").arg(file_base_name, copy_text);

        if (!suffix.isEmpty()) {
            new_file_name.append('.').append(suffix);
        }

        ++number;
        target_file_info = DFileService::instance()->createFileInfo(nullptr, targetDirectory->getUrlByChildFileName(new_file_name));
    } while (target_file_info->exists());

    return new_file_name;
}

bool DFileCopyMoveJobPrivate::doProcess(const DUrl &from, const DAbstractFileInfoPointer source_info, const DAbstractFileInfoPointer target_info, const bool isNew)
{
//    Q_Q(DFileCopyMoveJob);

    if (!source_info) {

        return setAndhandleError(DFileCopyMoveJob::UnknowUrlError, source_info, DAbstractFileInfoPointer(nullptr),
                                 "Failed create file info") == DFileCopyMoveJob::SkipAction;
    }

    if (!source_info->exists()) {
        DFileCopyMoveJob::Error errortype = (source_info->path().startsWith("/root/") && !target_info->path().startsWith("/root/")) ?
                                            DFileCopyMoveJob::PermissionError : DFileCopyMoveJob::NonexistenceError;
        return setAndhandleError(errortype, source_info,
                                 DAbstractFileInfoPointer(nullptr)) == DFileCopyMoveJob::SkipAction;
    }

    switch (source_info->fileType()) {
    case DAbstractFileInfo::CharDevice:
    case DAbstractFileInfo::BlockDevice:
    case DAbstractFileInfo::FIFOFile:
    case DAbstractFileInfo::SocketFile: {

        return setAndhandleError(DFileCopyMoveJob::SpecialFileError, source_info,
                                 DAbstractFileInfoPointer(nullptr)) == DFileCopyMoveJob::SkipAction;
    }
    default:
        break;
    }

    QSharedPointer<DFileHandler> handler(DFileService::instance()->createFileHandler(nullptr, from));

    if (!handler) {

        return setAndhandleError(DFileCopyMoveJob::UnknowUrlError, source_info,
                                 DAbstractFileInfoPointer(nullptr), "Failed create file handler") == DFileCopyMoveJob::SkipAction;
    }

    // only remove
    if (!target_info) {
        bool ok = false;
        //可以显示进度条
        m_isShowProgress = true;
        qint64 size = source_info->isSymLink() ? 0 : source_info->size();

        if (source_info->isFile() || source_info->isSymLink()) {
            ok = removeFile(handler, source_info);
            if (ok) {
                joinToCompletedFileList(from, DUrl(), size);
            }
        } else {
            // 删除文件夹时先设置其权限
            if (fileHints.testFlag(DFileCopyMoveJob::ForceDeleteFile)) {
                handler->setPermissions(source_info->fileUrl(), QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser);
            }

            ok = mergeDirectory(handler, source_info, DAbstractFileInfoPointer(nullptr));
            if (ok) {
                joinToCompletedDirectoryList(from, DUrl(), size);
            }
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
    const DAbstractFileInfoPointer &new_file_info = DFileService::instance()->createFileInfo(nullptr, target_info->getUrlByChildFileName(file_name));

    if (!new_file_info) {
        qWarning() << "fileinfo create failed!" << target_info->getUrlByChildFileName(file_name);
        return false;
    }

    if (new_file_info->exists()) {
        //忽略DStorageInfo::isSameFile判断链接文件的结果
        if ((mode == DFileCopyMoveJob::MoveMode || mode == DFileCopyMoveJob::CutMode) &&
                (new_file_info->fileUrl() == from || (DStorageInfo::isSameFile(from.path(), new_file_info->fileUrl().path()) && !new_file_info->isSymLink()))) {
            // 不用再进行后面的操作
            return true;
        }
        //可以显示进度条
        m_isShowProgress = true;

        // 禁止目录复制/移动到自己里面
        if (new_file_info->isAncestorsUrl(source_info->fileUrl())) {

            DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::TargetIsSelfError,
                                                                source_info, new_file_info);

            if (action == DFileCopyMoveJob::SkipAction) {
                //跳过文件大小统计
                skipFileSize += (source_info->isDir() || source_info->isSymLink()) ? 4096 : source_info->size();
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

        switch (setAndhandleError(errortype, source_info, new_file_info)) {
        case DFileCopyMoveJob::ReplaceAction:
            if (new_file_info->fileUrl() == from) {
                // 不用再进行后面的操作
                return true;
            }

            if (source_is_file && source_is_file == target_is_file) {
                break;
            } else {
                return false;
            }
        case DFileCopyMoveJob::MergeAction:
            if (!source_is_file && source_is_file == target_is_file) {
                break;
            } else {
                return false;
            }
        case DFileCopyMoveJob::SkipAction:
            //跳过文件大小统计
            skipFileSize += (source_info->isDir() || source_info->isSymLink()) ? 4096 : source_info->size();
            return true;
        case DFileCopyMoveJob::CoexistAction:
            file_name = handle ? handle->getNonExistsFileName(source_info, target_info)
                        : getNewFileName(source_info, target_info);
            goto create_new_file_info;
        default:
            return false;
        }
    }

    m_isShowProgress = true;

    if (source_info->isSymLink()) {
        bool ok = false;

        if (mode == DFileCopyMoveJob::CopyMode) {
            DAbstractFileInfoPointer new_source_info = source_info;
            if (fileHints.testFlag(DFileCopyMoveJob::FollowSymlink)) {
                do {
                    const DAbstractFileInfoPointer &symlink_target = DFileService::instance()->createFileInfo(nullptr, source_info->symLinkTarget());

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

        countrefinesize(4096);

        return ok;
    }

process_file:
    if (source_info->isFile()) {
        bool ok = false;
        qint64 size = source_info->size();

        while (!checkFreeSpace(size)) {
            DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::NotEnoughSpaceError,
                                                                source_info, new_file_info);

            if (action == DFileCopyMoveJob::SkipAction) {
                return true;
            }

            if (action == DFileCopyMoveJob::RetryAction) {
                continue;
            }

            if (action == DFileCopyMoveJob::EnforceAction) {
                break;
            }

            return false;
        }

        if (!checkFileSize(size)) {
            DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::FileSizeTooBigError,
                                                                source_info, new_file_info);

            if (action == DFileCopyMoveJob::SkipAction) {
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
        if (new_file_info->isAncestorsUrl(source_info->fileUrl())) {
            DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::TargetIsSelfError,
                                                                source_info, new_file_info);

            if (action == DFileCopyMoveJob::SkipAction) {
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
    if (toInfo && !toInfo->exists()) {
        DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

        do {
            // 当为保险箱路径时，判断目录名的长度，如果长度大于85，则不让其创建成功，并报错“文件名过长”
            QString strPath = toInfo->fileUrl().toString();
            if (VaultController::isVaultFile(strPath)) {
                // 获得目录名
                QString strDirName = strPath.section("/", -1, -1);
                if (strDirName.toUtf8().length() > 255) {
                    action = setAndhandleError(DFileCopyMoveJob::MkdirError, fromInfo, toInfo,
                                               qApp->translate("DFileCopyMoveJob", "Failed to open the directory, cause: file name too long"));
                    break;
                }
            }

            if (!handler->mkdir(toInfo->fileUrl())) {
                const DAbstractFileInfoPointer &parent_info = DFileService::instance()->createFileInfo(nullptr, toInfo->parentUrl());
                DFileCopyMoveJob::Error errortype = (!parent_info->exists() || parent_info->isWritable()) ?
                                                    DFileCopyMoveJob::MkdirError : DFileCopyMoveJob::PermissionError;
                QString errorstr = (!parent_info->exists() || parent_info->isWritable()) ?
                                   qApp->translate("DFileCopyMoveJob", "Failed to create the directory, cause: %1").arg(handler->errorString()) : QString();

                action = setAndhandleError(errortype, fromInfo, toInfo, errorstr);
            }
            isNew = true;
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

        if (action != DFileCopyMoveJob::NoAction) {
            return action == DFileCopyMoveJob::SkipAction;
        }
    }

    if (fromInfo->filesCount() <= 0 && mode == DFileCopyMoveJob::CopyMode) {
        QFileDevice::Permissions permissions = fromInfo->permissions();
        QString filePath = fromInfo->fileUrl().toLocalFile();
        if (VaultController::ins()->isVaultFile(filePath)) {
            permissions = VaultController::ins()->getPermissions(filePath);
        } else if (deviceListener->isFileFromDisc(fromInfo->path())) {
            permissions |= MasteredMediaController::getPermissionsCopyToLocal();
        }

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

    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const DUrl &url = iterator->next();
        const DAbstractFileInfoPointer &info = iterator->fileInfo();

        if (!process(url, info, toInfo, isNew)) {
            return false;
        }

        if (lastErrorHandleAction == DFileCopyMoveJob::SkipAction) {
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

        if (m_refineStat == DFileCopyMoveJob::Refine &&
                ((m_bDestLocal && isFromLocalUrls) || (isFromLocalUrls && !m_bDestLocal && m_isTagFromBlockDevice.load()))) {
            FileCopyInfoPointer copyinfo(new FileCopyInfo());
            copyinfo->isdir = true;
            copyinfo->permission = permissions;
            copyinfo->handler = handler;
            copyinfo->toinfo = toInfo;
            writeQueueEnqueue(copyinfo);
        }
        else {
            handler->setPermissions(toInfo->fileUrl(), permissions);
        }
    }

    if (mode == DFileCopyMoveJob::CopyMode) {
        return true;
    }

    if (existsSkipFile) {
        return true;
    }

    //  光盘中的目录不能被删除
    if (deviceListener->isFileFromDisc(fromInfo->fileUrl().toLocalFile())) {
        qInfo() << "remove file from disc, reject.";
        return true;
    }
    // 完成操作后删除原目录
    return removeFile(handler, fromInfo);
}

bool DFileCopyMoveJobPrivate::doCopyFile(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize)
{
    //预先读取
    {
        int fromfd = open(fromInfo->fileUrl().path().toUtf8().toStdString().data(), O_RDONLY);
        if (-1 != fromfd) {
            readahead(fromfd, 0, static_cast<size_t>(fromInfo->size()));
            close(fromfd);
        }
    }
    QScopedPointer<DFileDevice> fromDevice(DFileService::instance()->createFileDevice(nullptr, fromInfo->fileUrl()));
    if (!fromDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");

        return false;
    }
    QScopedPointer<DFileDevice> toDevice(DFileService::instance()->createFileDevice(nullptr, toInfo->fileUrl()));

    if (!toDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");

        return false;
    }
open_file: {
        DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

        do {
            // 如果打开文件在保险箱内
            QString strPath = toInfo->fileUrl().toString();
            if (VaultController::isVaultFile(strPath)) {
                QString strFileName = strPath.section("/", -1, -1);
                if (strFileName.toUtf8().length() > 255) {
                    qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();
                    action = setAndhandleError(DFileCopyMoveJob::OpenError, fromInfo,
                                               DAbstractFileInfoPointer(nullptr),
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
                action = setAndhandleError(errortype, fromInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            }
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());  // bug: 26333, while set the stop status shoule break the process!

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        do {
            if (toDevice->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << toInfo->fileUrl();
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

                action = setAndhandleError(errortype, toInfo, DAbstractFileInfoPointer(nullptr), errorstr);
                //防止卡死
                if (action == DFileCopyMoveJob::RetryAction) {
                    QThread::msleep(THREAD_SLEEP_TIME);
                }
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

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
                    action = setAndhandleError(DFileCopyMoveJob::ResizeError, toInfo, DAbstractFileInfoPointer(nullptr),
                                               toDevice->errorString());
                }
                //防止卡死
                if (action == DFileCopyMoveJob::RetryAction) {
                    QThread::msleep(THREAD_SLEEP_TIME);
                }
            } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

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
        fromgio->connect(q_ptr,&DFileCopyMoveJob::stopAllGioDervic,q_ptr,[fromgio](){
            fromgio->cancelAllOperate();
        });
    }
    if (togio) {
        togio->connect(q_ptr,&DFileCopyMoveJob::stopAllGioDervic,q_ptr,[togio](){
            togio->cancelAllOperate();
        });
    }

    qint64 block_Size = fromInfo->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : fromInfo->size();

    char *data = new char[block_Size + 1];

    Q_FOREVER {
        qint64 current_pos = fromDevice->pos();
    read_data:
        if (Q_UNLIKELY(!stateCheck())) {
            if (!m_bDestLocal) {
                toDevice->closeWriteReadFailed(true);
            }
            delete[] data;
            return false;
        }

        qint64 size_read = fromDevice->read(data, block_Size);
        if (Q_UNLIKELY(!stateCheck())) {
            if (!m_bDestLocal) {
                toDevice->closeWriteReadFailed(true);
            }
            delete[] data;
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

            switch (setAndhandleError(errortype, fromInfo, toInfo, errorstr)) {
            case DFileCopyMoveJob::RetryAction: {
                if (!fromDevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, fromDevice->errorString());
                    delete[] data;
                    fromDevice->close();
                    toDevice->close();
                    return false;
                }

                goto read_data;
            }
            case DFileCopyMoveJob::SkipAction:
                delete[] data;
                fromDevice->close();
                toDevice->close();
                return true;
            default:
                delete[] data;
                fromDevice->close();
                toDevice->close();
                return false;
            }
        }

        current_pos = toDevice->pos();
    write_data:
        if (Q_UNLIKELY(!stateCheck())) {
            if (!m_bDestLocal) {
                toDevice->closeWriteReadFailed(true);
            }
            return false;
        }
        qint64 size_write = toDevice->write(data, size_read);
        if (Q_UNLIKELY(!stateCheck())) {
            if (!m_bDestLocal) {
                toDevice->closeWriteReadFailed(true);
            }
            return false;


        }
        //如果写失败了，直接推出
        if (size_write < 0) {
            if (!stateCheck()) {
                //临时处理 fix
                //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                if (FileUtils::isGvfsMountFile(toInfo->path())) {
                    toDevice->closeWriteReadFailed(true);
                }
                delete[] data;
                fromDevice->close();
                return false;
            }
            switch (setAndhandleError(DFileCopyMoveJob::WriteError, fromInfo, toInfo,
                                      qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString()))) {
            case DFileCopyMoveJob::RetryAction: {
                if (!toDevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, toDevice->errorString());
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    if (FileUtils::isGvfsMountFile(toInfo->path())) {
                        toDevice->closeWriteReadFailed(true);
                    }
                    fromDevice->close();
                    delete[] data;
                    return false;
                }

                goto write_data;
            }
            case DFileCopyMoveJob::SkipAction:
                //临时处理 fix
                //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                if (FileUtils::isGvfsMountFile(toInfo->path())) {
                    toDevice->closeWriteReadFailed(true);
                }
                fromDevice->close();
                delete[] data;
                return true;
            default:
                //临时处理 fix
                //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                if (FileUtils::isGvfsMountFile(toInfo->path())) {
                    toDevice->closeWriteReadFailed(true);
                }
                fromDevice->close();
                delete[] data;
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

                switch (setAndhandleError(errortype, fromInfo, toInfo, errorstr)) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!toDevice->seek(current_pos)) {
                        setError(DFileCopyMoveJob::UnknowError, toDevice->errorString());
                        if (FileUtils::isGvfsMountFile(toInfo->path())) {
                            toDevice->closeWriteReadFailed(true);
                        }
                        fromDevice->close();
                        delete[] data;
                        return false;
                    }

                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    if (FileUtils::isGvfsMountFile(toInfo->path())) {
                        toDevice->closeWriteReadFailed(true);
                    }
                    fromDevice->close();
                    delete[] data;
                    return true;
                default:
                    delete[] data;
                    if (FileUtils::isGvfsMountFile(toInfo->path())) {
                        toDevice->closeWriteReadFailed(true);
                    }
                    fromDevice->close();
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
// end forever
    // 关闭文件时可能会需要很长时间，因为内核可能要把内存里的脏数据回写到硬盘
    setState(DFileCopyMoveJob::IOWaitState);
    delete[] data;
    fromDevice->close();
    toDevice->close();
    countrefinesize(fromInfo->size() <= 0 ? 4096 : 0);
    if (state == DFileCopyMoveJob::IOWaitState) {
        setState(DFileCopyMoveJob::RunningState);
    }

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

    handler->setPermissions(toInfo->fileUrl(), /*source_info->permissions()*/permissions);


    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    q_ptr->disconnect(q_ptr,&DFileCopyMoveJob::stopAllGioDervic,q_ptr,nullptr);

    if (fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking)) {
        return true;
    }

    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

    do {
        if (toDevice->open(QIODevice::ReadOnly)) {
            break;
        } else {
            QString errorstr = "Unable to open file for integrity check, , cause: " + toDevice->errorString();
            action = setAndhandleError(DFileCopyMoveJob::OpenError, toInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

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

            switch (setAndhandleError(DFileCopyMoveJob::IntegrityCheckingError, fromInfo, toInfo, errorstr)) {
            case DFileCopyMoveJob::RetryAction: {
                continue;
            }
            case DFileCopyMoveJob::SkipAction:
                delete [] data1;
                return true;
            default:
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
        DFileCopyMoveJob::Action action = setAndhandleError(DFileCopyMoveJob::IntegrityCheckingError, fromInfo, toInfo);

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        }

        if (action == DFileCopyMoveJob::RetryAction) {
            goto open_file;
        }

        return false;
    }

    qCDebug(fileJob(), "adler value: 0x%lx", source_checksum);

    return true;
}

bool DFileCopyMoveJobPrivate::doCopyFileBig(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize)
{
    int fromFd = -1;
    int toFd = -1;
        DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

        do {
            // 如果打开文件在保险箱内
            QString strPath = toInfo->fileUrl().toString();
            if (VaultController::isVaultFile(strPath)) {
                QString strFileName = strPath.section("/", -1, -1);
                if (strFileName.toUtf8().length() > 255) {
                    qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();
                    action = setAndhandleError(DFileCopyMoveJob::OpenError, fromInfo,
                                               DAbstractFileInfoPointer(nullptr),
                                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: file name too long"));
                    break;
                }
            }
            fromFd = open(fromInfo->fileUrl().path().toUtf8().toStdString().data(),O_RDONLY);
            if (-1 != fromFd) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();
                DFileCopyMoveJob::Error errortype = fromInfo->isReadable() ? DFileCopyMoveJob::OpenError :
                                                    DFileCopyMoveJob::PermissionError;
                QString errorstr = fromInfo->isReadable() ?
                                   qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(QString()) :
                                   QString();
                action = setAndhandleError(errortype, fromInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            }
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());  // bug: 26333, while set the stop status shoule break the process!

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        do {
            toFd = open(toInfo->fileUrl().toLocalFile().toUtf8().toStdString().data(),O_RDWR | O_CREAT,0666);
            if (-1 != toFd) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << toInfo->fileUrl();
                DFileCopyMoveJob::Error errortype = (!toInfo->exists() || toInfo->isWritable()) ? DFileCopyMoveJob::OpenError :
                                                    DFileCopyMoveJob::PermissionError;
                // task-36496 "Permission denied"没有被翻译 翻译为“没有权限”
                QString errorstr("");
                if (errortype == DFileCopyMoveJob::PermissionError) {
                    errorstr = (!toInfo->exists() || toInfo->isWritable()) ?
                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: Permission denied") :
                               QString();
                } else {
                    errorstr = (!toInfo->exists() || toInfo->isWritable()) ?
                               qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(QString()) :
                               QString();
                }

                action = setAndhandleError(errortype, toInfo, DAbstractFileInfoPointer(nullptr), errorstr);
                //防止卡死
                if (action == DFileCopyMoveJob::RetryAction) {
                    QThread::msleep(THREAD_SLEEP_TIME);
                }
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

        if (action == DFileCopyMoveJob::SkipAction) {
            close(fromFd);
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            close(fromFd);
            return false;
        }
        do {
            if (-1 != ftruncate(toFd,fromInfo->size())) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                action = setAndhandleError(DFileCopyMoveJob::ResizeError, toInfo, DAbstractFileInfoPointer(nullptr),
                                           QObject::tr("ftruncate error!"));
            }
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

        if (action == DFileCopyMoveJob::SkipAction) {
            close(fromFd);
            close(toFd);
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            close(fromFd);
            close(toFd);
            return false;
        }
    char * fromPoint = nullptr;
    char * toPoint = nullptr;
        action = DFileCopyMoveJob::NoAction;
        fromPoint = static_cast<char *>(mmap(nullptr,static_cast<size_t>(fromInfo->size()),PROT_READ,MAP_SHARED,fromFd,0));
        do {
            if (MAP_FAILED != fromPoint) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                action = setAndhandleError(DFileCopyMoveJob::MmapError, toInfo, DAbstractFileInfoPointer(nullptr),
                                           QObject::tr("mmap file to memory fialed!"));
            }
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

        if (action == DFileCopyMoveJob::SkipAction) {
            close(fromFd);
            close(toFd);
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            close(fromFd);
            close(toFd);
            return false;
        }

        toPoint = static_cast<char *>(mmap(nullptr,static_cast<size_t>(fromInfo->size()),PROT_WRITE,MAP_SHARED,toFd,0));
        do {
            if (MAP_FAILED != toPoint) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                action = setAndhandleError(DFileCopyMoveJob::MmapError, toInfo, DAbstractFileInfoPointer(nullptr),
                                           QObject::tr("mmap file to memory fialed!"));
            }
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

        if (action == DFileCopyMoveJob::SkipAction) {
            munmap(fromPoint,static_cast<size_t>(fromInfo->size()));
            close(fromFd);
            close(toFd);
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            munmap(fromPoint,static_cast<size_t>(fromInfo->size()));
            close(fromFd);
            close(toFd);
            return false;
        }

#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (fromFd > 0) {
        posix_fadvise(fromFd, 0, 0, POSIX_FADV_SEQUENTIAL);
    }

    if (toFd > 0) {
        posix_fadvise(toFd, 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif
    //开启线程执行拷贝
    QFuture<void> futrueArray[10];
    QMutex mutex;
    qint64 fromSize = fromInfo->size();
    __off_t len = lseek(fromFd,0,SEEK_END);
    m_countVetor.push_back(QAtomicInt(-1));
    if (m_countVetor.count() <= 0 || m_countVetor.count() <= m_count.load() ) {
        munmap(fromPoint,static_cast<size_t>(fromInfo->size()));
        munmap(toPoint,static_cast<size_t>(fromInfo->size()));
        close(fromFd);
        close(toFd);
        return false;
    }
    for (int i = 0;i < 10; ++i)
    {
        futrueArray[i] = QtConcurrent::run([&mutex,&fromSize,&fromPoint,&toPoint,&len,&blockSize,this]() {
            mutex.lock();
            m_countVetor[m_count].store(m_countVetor[m_count].load() + 1);
            __off_t srcpoint = ((fromSize)/10)*m_countVetor[m_count].load();   //获取对应线程的拷贝起始点
            __off_t destpoint = srcpoint;
            qDebug() << destpoint << srcpoint << m_count\
                     << m_countVetor[m_count].load() << fromSize << fromSize/10 << static_cast<size_t>(fromSize/10) << len;
            mutex.unlock();
            qint64 everySize = (fromSize)/10;
            qint64 copySize = everySize > blockSize ? blockSize : everySize;
            qint64 lastSize = 0;
            if(m_countVetor[m_count].load() != 9)
            {
                while(everySize > 0) {
                    if (Q_UNLIKELY(!stateCheck())) {
                        break;
                    }
                    memcpy(toPoint+destpoint + lastSize,fromPoint+srcpoint + lastSize,static_cast<size_t>(copySize));
                    everySize -= copySize;
                    lastSize += copySize;
                    countrefinesize(copySize);
                    copySize = everySize > blockSize ? blockSize : everySize;
                }
            }
            else
            { //考虑最后一个线程拷贝的可能不足一块
                everySize = fromSize-srcpoint;
                copySize = everySize > blockSize ? blockSize : everySize;
                while(everySize > 0) {
                    if (Q_UNLIKELY(!stateCheck())) {
                        break;
                    }
                    memcpy(toPoint+destpoint + lastSize,fromPoint+srcpoint + lastSize,static_cast<size_t>(copySize));
                    everySize -= copySize;
                    lastSize += copySize;
                    countrefinesize(copySize);
                    copySize = everySize > blockSize ? blockSize : everySize;
                }
            }
        });
    }
    for ( auto futrue : futrueArray) {
        futrue.waitForFinished();
    }
    munmap(fromPoint,static_cast<size_t>(fromInfo->size()));
    munmap(toPoint,static_cast<size_t>(fromInfo->size()));
    close(fromFd);
    close(toFd);
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

    handler->setPermissions(toInfo->fileUrl(), /*source_info->permissions()*/permissions);


    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    q_ptr->disconnect(q_ptr,&DFileCopyMoveJob::stopAllGioDervic,q_ptr,nullptr);

    if (fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking)) {
        return true;
    }
//    qCDebug(fileJob(), "adler value: 0x%lx", source_checksum);

    return true;
}

bool DFileCopyMoveJobPrivate::doCopyFileSmall(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize)
{
    QSharedPointer<DFileDevice> fromDevice(DFileService::instance()->createFileDevice(nullptr, fromInfo->fileUrl()));
    if (!fromDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");
        q_ptr->stop();
        return false;
    }
    QSharedPointer<DFileDevice> toDevice(DFileService::instance()->createFileDevice(nullptr, toInfo->fileUrl()));
    while (m_fileRefineFd > 600) {
        usleep(10);
    }
    if (!toDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");
        q_ptr->stop();
        return false;
    }
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

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

        if (fromDevice->open(QIODevice::ReadOnly)) {
            action = DFileCopyMoveJob::NoAction;
            m_fileRefineFd++;
        } else {
            qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();
            DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::PermissionError;
            QString errorstr;
            if (fromInfo->isReadable()) {
                errorstr = qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(fromDevice->errorString());
                errortype = DFileCopyMoveJob::OpenError;
            }

            action = setAndhandleError(errortype, fromInfo, DAbstractFileInfoPointer(nullptr), errorstr);
            //防止卡死
            if (action == DFileCopyMoveJob::RetryAction) {
                QThread::msleep(THREAD_SLEEP_TIME);
            }
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());  // bug: 26333, while set the stop status shoule break the process!

    if (action == DFileCopyMoveJob::SkipAction) {
        return true;
    } else if (action != DFileCopyMoveJob::NoAction) {
        //出错就停止
        q_ptr->stop();
        return false;
    }
#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (fromDevice->handle() > 0) {
        posix_fadvise(fromDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif
    qint64 size_block = fromInfo->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : fromInfo->size();
    size_block = blockSize;

    FileCopyInfoPointer copyinfo(new FileCopyInfo());
    copyinfo->handler = handler;
    copyinfo->frominfo = fromInfo;
    copyinfo->toinfo = toInfo;
    while (true) {
        qint64 current_pos = fromDevice->pos();
        copyinfo->currentpos = current_pos;
        char *buffer = new char[size_block + 1];

        if (Q_UNLIKELY(!stateCheck())) {
            delete[]  buffer;
            fromDevice->close();
            toDevice->close();
            return false;
        }
        qint64 size_read = fromDevice->read(buffer, size_block);
        if (Q_UNLIKELY(!stateCheck())) {
            delete[]  buffer;
            fromDevice->close();
            toDevice->close();
            return false;
        }

        if (Q_UNLIKELY(size_read <= 0)) {
            if (size_read == 0 && fromDevice->atEnd()) {
                copyinfo->buffer = buffer;
                copyinfo->size = size_read;
                break;
            }

            const_cast<DAbstractFileInfo *>(copyinfo->frominfo.data())->refresh();
            DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::NonexistenceError;
            QString errorstr;
            if (copyinfo->frominfo->exists()) {
                errortype = DFileCopyMoveJob::ReadError;
                errorstr = qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: %1").arg(fromDevice->errorString());
            }
            switch (setAndhandleError(errortype, copyinfo->frominfo, copyinfo->toinfo, errorstr)) {
            case DFileCopyMoveJob::RetryAction: {
                if (!fromDevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, fromDevice->errorString());
                    fromDevice->close();
                    q_ptr->stop();
                    return false;
                }
                break;
            }
            case DFileCopyMoveJob::SkipAction:
                m_skipFileQueueMutex.lock();
                m_skipFileQueue.push_back(toDevice);
                m_skipFileQueueMutex.unlock();
                return true;
            default:
                fromDevice->close();
                q_ptr->stop();
                return false;
            }
        } else {
            FileCopyInfoPointer tmpinfo(new FileCopyInfo);
            tmpinfo->closeflag = false;
            tmpinfo->frominfo = copyinfo->frominfo;
            tmpinfo->toinfo = copyinfo->toinfo;
            tmpinfo->todevice = toDevice;
            tmpinfo->currentpos = current_pos;
            tmpinfo->buffer = buffer;
            tmpinfo->size = size_read;

            writeQueueEnqueue(tmpinfo);

            if (!m_isWriteThreadStart.load()) {
                m_isWriteThreadStart.store(true);
                m_writeResult = QtConcurrent::run([this]() {
                        qDebug() << "write thread start >>>>>>>>>>>>>>";
                        writeRefineThread();
                    });
            }
        }
    }
    copyinfo->todevice = toDevice;

    writeQueueEnqueue(copyinfo);

    fromDevice->close();
    m_fileRefineFd--;

    return true;
}

bool DFileCopyMoveJobPrivate::doCopyFileU(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize)
{
    while (m_fileRefineFd > 200) {
        QThread::msleep(5);
    }
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
    int fromfd = -1;
    int tofd = -1;

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
        fromfd = open(fromInfo->fileUrl().toLocalFile().toUtf8().toStdString().data(),O_RDONLY);
        if (-1 != fromfd) {
            action = DFileCopyMoveJob::NoAction;
            m_fileRefineFd++;
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
        tofd = open(toInfo->fileUrl().toLocalFile().toUtf8().toStdString().data(),O_CREAT | O_WRONLY, 0x666);
        if (-1 != tofd) {
            action = DFileCopyMoveJob::NoAction;
            m_fileRefineFd++;
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
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());  // bug: 26333, while set the stop status shoule break the process!

    if (action == DFileCopyMoveJob::SkipAction) {
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
    if (tofd > 0) {
        posix_fadvise(tofd, 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif
    qint64 size_block = fromInfo->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : fromInfo->size();
    size_block = blockSize;

    FileCopyInfoPointer copyinfo(new FileCopyInfo());
    copyinfo->handler = handler;
    copyinfo->frominfo = fromInfo;
    copyinfo->toinfo = toInfo;
    copyinfo->tofd = tofd;
    lseek(fromfd,0,SEEK_SET);
    qint64 current_pos = 0;
    while (true) {
        copyinfo->currentpos = current_pos;
        char *buffer = new char[size_block + 1];

        if (Q_UNLIKELY(!stateCheck())) {
            delete[]  buffer;
            close(fromfd);
            close(tofd);
            return false;
        }
        qint64 size_read = read(fromfd, buffer, static_cast<size_t>(size_block));

        if (Q_UNLIKELY(!stateCheck())) {
            delete[]  buffer;
            close(fromfd);
            close(tofd);
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
                if (!lseek(fromfd,current_pos,SEEK_SET)) {
                    setError(DFileCopyMoveJob::UnknowError, "");
                    close(fromfd);
                    q_ptr->stop();
                    return false;
                }
                break;
            }
            case DFileCopyMoveJob::SkipAction:
                m_skipFileQueueMutex.lock();
//                m_skipFileQueue.push_back(toDevice);
                m_skipFileQueueMutex.unlock();
                return true;
            default:
                close(fromfd);
                q_ptr->stop();
                return false;
            }
        } else {
            FileCopyInfoPointer tmpinfo(new FileCopyInfo);
            tmpinfo->closeflag = false;
            tmpinfo->frominfo = copyinfo->frominfo;
            tmpinfo->toinfo = copyinfo->toinfo;
//            tmpinfo->todevice = toDevice;
            tmpinfo->currentpos = current_pos;
            tmpinfo->buffer = buffer;
            tmpinfo->size = size_read;
            tmpinfo->tofd = tofd;
            current_pos += size_read;

            writeQueueEnqueue(tmpinfo);
            if (!m_isWriteThreadStart.load()) {
                m_isWriteThreadStart.store(true);
                m_writeResult = QtConcurrent::run([this]() {
                        qDebug() << "write thread start >>>>>>>>>>>>>>";
                        writeRefineThread();
                    });
            }
        }
    }
//    copyinfo->todevice = toDevice;

    writeQueueEnqueue(copyinfo);

    close(fromfd);
    m_fileRefineFd--;

    return true;
}

bool DFileCopyMoveJobPrivate::doRemoveFile(const QSharedPointer<DFileHandler> &handler, const DAbstractFileInfoPointer fileInfo)
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

            if (fileInfo->canRename()) {
                errortype = DFileCopyMoveJob::RemoveError;
                errorstr = qApp->translate("DFileCopyMoveJob", "Failed to delete the file, cause: %1").arg(handler->errorString());
            }
        }

        action = setAndhandleError(errortype, fileInfo, DAbstractFileInfoPointer(nullptr), errorstr);
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
                    return false;
                }

                // 删除旧的链接文件
                if (!doRemoveFile(handler, oldInfo)) {
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

    if (lastErrorHandleAction == DFileCopyMoveJob::SkipAction) {
        // 说明复制文件过程被跳过
        return true;
    }

    handler->setFileTime(newInfo->fileUrl(), oldInfo->lastRead(), oldInfo->lastModified());

    if (!doRemoveFile(handler, oldInfo)) {
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

bool DFileCopyMoveJobPrivate::process(const DUrl from, const DAbstractFileInfoPointer target_info)
{
    const DAbstractFileInfoPointer &source_info = DFileService::instance()->createFileInfo(nullptr, from);

    return process(from, source_info, target_info);
}

bool DFileCopyMoveJobPrivate::process(const DUrl from, const DAbstractFileInfoPointer source_info, const DAbstractFileInfoPointer target_info, const bool isNew)
{
    // reset error and action
    unsetError();
    lastErrorHandleAction = DFileCopyMoveJob::NoAction;

    beginJob(JobInfo::Preprocess, from, (source_info && target_info) ? target_info->getUrlByChildFileName(source_info->fileName()) : DUrl());
    bool ok = doProcess(from, source_info, target_info, isNew);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::copyFile(const DAbstractFileInfoPointer fromInfo, const DAbstractFileInfoPointer toInfo, const QSharedPointer<DFileHandler> &handler, int blockSize)
{
//    Q_Q(DFileCopyMoveJob);
//    qint64 elapsed = 0;
    if (fileJob().isDebugEnabled()) {
//        elapsed = updateSpeedElapsedTimer->elapsed();
        updateSpeedElapsedTimer->elapsed();
    }
    beginJob(JobInfo::Copy, fromInfo->fileUrl(), toInfo->fileUrl());
    if (m_refineStat != DFileCopyMoveJob::Refine
            //! 为避免卡顿，保险箱用之前的拷贝方式
            || VaultController::isVaultFile(fromInfo->toLocalFile())
            || VaultController::isVaultFile(toInfo->toLocalFile())) {
        return doCopyFile(fromInfo, toInfo, handler, blockSize);
    }
    //判读目标目录和本地目录是不是同盘，并且是大文件
    if (m_bDestLocal && isFromLocalUrls && fromInfo->size() > 500 * 1024 * 1024) {
        bool ok = doCopyFileBig(fromInfo, toInfo, handler, blockSize);
        m_count.store(m_count.load() + 1);
        return ok;
    }
    //1.判断源文件是本地，目标文件也是本地执行读写线程分离处理
    //2.判断源文件是本地，目标文件是（除光盘外的）块设备，
    else if (m_bDestLocal && isFromLocalUrls ) {
        QtConcurrent::run(&m_pool, this, static_cast<bool(DFileCopyMoveJobPrivate::*)
                          (const DAbstractFileInfoPointer , const DAbstractFileInfoPointer , const QSharedPointer<DFileHandler> &, int )>
                          (&DFileCopyMoveJobPrivate::doCopyFile), fromInfo, toInfo, handler, blockSize);
        return true;
    }
    else if ((isFromLocalUrls && !m_bDestLocal && m_isTagFromBlockDevice.load())) {
        return doCopyFileU(fromInfo, toInfo, handler, blockSize);
    }
    else{
        return doCopyFile(fromInfo, toInfo, handler, blockSize);
    }

//    qCDebug(fileJob(), "Time spent of copy the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed);

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

void DFileCopyMoveJobPrivate::beginJob(JobInfo::Type type, const DUrl from, const DUrl target, const bool isNew)
{
//    qCDebug(fileJob(), "job begin, Type: %d, from: %s, to: %s", type, qPrintable(from.toString()), qPrintable(target.toString()));
    if (!isNew) {
        jobStack.push({type, QPair<DUrl, DUrl>(from, target)});
        currentJobDataSizeInfo = qMakePair(-1, 0);
        currentJobFileHandle = -1;
    }
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

    completedProgressDataSize += dataSize <= 0 ? 4096 : 0;

    ++completedFilesCount;

    Q_EMIT q_ptr->completedFilesCountChanged(completedFilesCount);

    if (lastErrorHandleAction == DFileCopyMoveJob::SkipAction) {
        return;
    }

    completedFileList << qMakePair(from, target);
}

void DFileCopyMoveJobPrivate::joinToCompletedDirectoryList(const DUrl from, const DUrl target, qint64 dataSize)
{
    Q_UNUSED(dataSize)
//    qCDebug(fileJob(), "directory. from: %s, target: %s, data size: %lld", qPrintable(from.toString()), qPrintable(target.toString()), dataSize);

//    completedDataSize += dataSize;
    completedProgressDataSize += 4096;
    ++completedFilesCount;

    countrefinesize(4096);

    Q_EMIT q_ptr->completedFilesCountChanged(completedFilesCount);

    if (lastErrorHandleAction == DFileCopyMoveJob::SkipAction) {
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
    bool fromLocal = (isFromLocalUrls && targetUrl.isValid());
    const qint64 totalSize = fromLocal ? totalsize : fileStatistics->totalProgressSize();
    //通过getCompletedDataSize取出的已传输的数据大小后期会远超实际数据大小，这种情况下直接使用completedDataSize
    qint64 dataSize(getCompletedDataSize());
    // completedDataSize 可能一直为 0
    if (dataSize > completedDataSize && completedDataSize > 0) {
        dataSize = completedDataSize;
    }

    dataSize += completedProgressDataSize;

    //优化
    dataSize = m_bDestLocal ? m_refineCopySize : dataSize;

    dataSize += skipFileSize;

    if (totalSize == 0)
        return;

//    if (fileStatistics->isFinished()) {
    if ((fromLocal && iscountsizeover) || fileStatistics->isFinished()) {
        qreal realProgress = qreal(dataSize) / totalSize;
        if (realProgress > lastProgress)
            lastProgress = realProgress;
        qCDebug(fileJob(), "completed data size: %lld, total data size: %lld", dataSize, totalSize);
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
    m_isEveryReadAndWritesSnc = FileUtils::isGvfsMountFile(targetUrl.path());
    qDebug() << targetUrl.toLocalFile();
    DStorageInfo targetStorageInfo(targetUrl.toLocalFile());
    if (!m_isEveryReadAndWritesSnc && targetStorageInfo.isValid()) {
        const QString &fs_type = targetStorageInfo.fileSystemType();
        m_isVfat = fs_type.contains("vfat");

        m_isEveryReadAndWritesSnc = (fs_type == "cifs");
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

bool DFileCopyMoveJobPrivate::writeRefineThread()
{
    bool ok = true;
    while (checkRefineCopyProccessSate(DFileCopyMoveJob::ReadFileProccessOver)) {
        ok = (isFromLocalUrls && !m_bDestLocal && m_isTagFromBlockDevice.load()) ? writeRefineEx():writeRefineEx();
        if (!ok) {
            break;
        }
    }
    qDebug() << "write thread over      stat change   ========== " << QDateTime::currentMSecsSinceEpoch() - m_sart;
    if (ok) {
        ok = (isFromLocalUrls && !m_bDestLocal && m_isTagFromBlockDevice.load()) ? writeRefineEx(): writeRefineEx();
    }
    else {
        q_ptr->stop();
    }
    cancelReadFileDealWriteThread();
    qWarning() << "write thread over         ========== " << QDateTime::currentMSecsSinceEpoch() - m_sart;
    return true;
}

//为考虑线程之间的同步问题（错误同步，假如，读线程出错了，
//1.重试时写线程是不是要全部停住(已暂停)
//2.重试失败，跳过当前文件应该怎么处理（加入一个跳过写入队列，写线程检查当前的写队列，移除都所有的要移除的选项）
//3.重试失败，直接退出拷贝怎么处理（处理所有的写入队列，清理掉）。
//写线程出错时，1.弹出提示框时，是否要暂停读线程2.点击跳过应该怎么处理，清理掉有相同的文件描述符的写队列、关闭文件读文件是否要暂停3.关闭拷贝时，要怎么处理）
bool DFileCopyMoveJobPrivate::writeRefine()
{
    while (!checkWritQueueEmpty()) {
        auto info = writeQueueDequeue();
        if (Q_UNLIKELY(!stateCheck())) {
            return false;
        }
        //对文件夹加权
        if (info->isdir) {
            info->handler->setPermissions(info->toinfo->fileUrl(),info->permission);\
            continue;
        }
        if (skipReadFileDealWriteThread(info)) {
            continue;
        }

        if (!info->todevice->isOpen()) {
            DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
            do {
                qDebug() << info->todevice->fileUrl();
                if (info->todevice->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    action = DFileCopyMoveJob::NoAction;
                    m_fileRefineFd++;
                } else {
                    qCDebug(fileJob()) << "open error:" << info->toinfo->fileUrl();
                    DFileCopyMoveJob::Error errortype = DFileCopyMoveJob::PermissionError;
                    QString errorstr;
                    if (!info->toinfo->exists() || info->toinfo->isWritable()) {
                        errortype = DFileCopyMoveJob::OpenError;
                        errorstr = qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(info->todevice->errorString());
                    }
                    action = setAndhandleError(errortype, info->toinfo, DAbstractFileInfoPointer(nullptr), errorstr);

                    //防止卡死
                    if (action == DFileCopyMoveJob::RetryAction) {
                        QThread::msleep(THREAD_SLEEP_TIME);
                    }
                }
            } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

            if (action == DFileCopyMoveJob::SkipAction) {
                delete[] info->buffer;
                m_skipFileQueueMutex.lock();
                m_skipFileQueue.push_back(info->todevice);
                m_skipFileQueueMutex.unlock();
                return true;
            } else if (action != DFileCopyMoveJob::NoAction) {
                delete[] info->buffer;
                return false;
            }

            if (fileHints.testFlag(DFileCopyMoveJob::ResizeDestinationFile)) {
                do {
                    if (info->todevice->resize(info->frominfo->size())) {
                        action = DFileCopyMoveJob::NoAction;
                    } else {
                        action = setAndhandleError(DFileCopyMoveJob::ResizeError, info->toinfo, DAbstractFileInfoPointer(nullptr),
                                                   info->todevice->errorString());
                    }
                } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

                if (action == DFileCopyMoveJob::SkipAction) {
                    info->todevice->close();
                    m_fileRefineFd--;
                    delete[] info->buffer;
                    m_skipFileQueueMutex.lock();
                    m_skipFileQueue.push_back(info->todevice);
                    m_skipFileQueueMutex.unlock();
                    return true;
                } else if (action != DFileCopyMoveJob::NoAction) {
                    info->todevice->close();
                    m_fileRefineFd--;
                    delete[] info->buffer;
                    return false;
                }
            }
#ifdef Q_OS_LINUX
            // 开启读取优化，告诉内核，我们将顺序读取此文件
            if (info->todevice && info->todevice->handle() > 0) {
                posix_fadvise(info->todevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
            }
#endif
        }
    write_data: {
//            qDebug() << " write start   ===== " << curt - m_sart;
            qint64 size_write = info->todevice->write(info->buffer, info->size);
//            qDebug() << "write runing  " << m_write;
            if (Q_UNLIKELY(!stateCheck())) {
                return false;
            }
            //如果写失败了，直接推出
            if (size_write < 0) {
                if (!stateCheck()) {
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    if (!m_bDestLocal || m_isTagGvfsFile) {
                        info->todevice->closeWriteReadFailed(true);
                    } else {
                        info->todevice->close();
                    }
                    m_fileRefineFd--;
                    delete[] info->buffer;
                    return false;
                }
                switch (setAndhandleError(DFileCopyMoveJob::WriteError, info->frominfo, info->toinfo,
                                          qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").
                                          arg(info->todevice->errorString()))) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!info->todevice->seek(info->currentpos)) {
                        setError(DFileCopyMoveJob::UnknowError, info->todevice->errorString());
                        //临时处理 fix
                        //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                        //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                        if (FileUtils::isGvfsMountFile(info->toinfo->path())) {
                            info->todevice->closeWriteReadFailed(true);
                        } else {
                            info->todevice->close();
                        }
                        m_fileRefineFd--;

                        m_skipFileQueueMutex.lock();
                        m_skipFileQueue.push_back(info->todevice);
                        m_skipFileQueueMutex.unlock();

                        delete[] info->buffer;
                        return false;
                    }
                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    if (m_isTagGvfsFile) {
                        info->todevice->closeWriteReadFailed(true);
                    } else {
                        info->todevice->close();
                    }
                    m_fileRefineFd--;
                    delete[] info->buffer;

                    m_skipFileQueueMutex.lock();
                    m_skipFileQueue.push_back(info->todevice);
                    m_skipFileQueueMutex.unlock();
                    return true;
                default:
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    if (FileUtils::isGvfsMountFile(info->toinfo->path())) {
                        info->todevice->closeWriteReadFailed(true);
                    } else {
                        info->todevice->close();
                    }
                    m_fileRefineFd--;
                    delete[] info->buffer;

                    m_skipFileQueueMutex.lock();
                    m_skipFileQueue.push_back(info->todevice);
                    m_skipFileQueueMutex.unlock();

                    return false;
                }
            }


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
                            //        writtenDataSize += size_write;

                            surplus_data += size_write;
                            surplus_size -= size_write;

                            size_write = info->todevice->write(surplus_data, surplus_size);
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
                        errorstr = qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(info->todevice->errorString());
                    }

                    switch (setAndhandleError(errortype, info->frominfo, info->toinfo, errorstr)) {
                    case DFileCopyMoveJob::RetryAction: {
                        if (!info->todevice->seek(info->currentpos)) {
                            setError(DFileCopyMoveJob::UnknowError, info->todevice->errorString());
                            delete[] info->buffer;
                            if (FileUtils::isGvfsMountFile(info->toinfo->path())) {
                                info->todevice->closeWriteReadFailed(true);
                            } else {
                                info->todevice->close();
                            }
                            m_fileRefineFd--;

                            m_skipFileQueueMutex.lock();
                            m_skipFileQueue.push_back(info->todevice);
                            m_skipFileQueueMutex.unlock();

                            return false;
                        }

                        goto write_data;
                    }
                    case DFileCopyMoveJob::SkipAction:
                        if (!m_bDestLocal || FileUtils::isGvfsMountFile(info->toinfo->path())) {
                            info->todevice->closeWriteReadFailed(true);
                        } else {
                            info->todevice->close();
                        }
                        m_fileRefineFd--;
                        delete[] info->buffer;

                        m_skipFileQueueMutex.lock();
                        m_skipFileQueue.push_back(info->todevice);
                        m_skipFileQueueMutex.unlock();

                        return true;
                    default:
                        if (m_isTagGvfsFile) {
                            info->todevice->closeWriteReadFailed(true);
                        } else {
                            info->todevice->close();
                        }
                        m_fileRefineFd--;
                        m_errorUrlList.push_back(info->todevice->fileUrl());
                        delete[] info->buffer;
                        return false;
                    }
                } while (false);
            }
            currentJobDataSizeInfo.second += size_write;
            completedDataSize += size_write;
            completedDataSizeOnBlockDevice += size_write;

            countrefinesize(size_write);
            delete[] info->buffer;
        }
        //关闭文件并加权
        if (info->closeflag) {
            info->todevice->close();
            m_fileRefineFd--;
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

            handler->setPermissions(info->toinfo->fileUrl(), /*source_info->permissions()*/permissions);
        }
    }
    return true;
}

bool DFileCopyMoveJobPrivate::writeRefineEx()
{
    while (!checkWritQueueEmpty()) {
        auto info = writeQueueDequeue();
        if (Q_UNLIKELY(!stateCheck())) {
            close(info->tofd);
            return false;
        }
        //对文件夹加权
        if (info->isdir) {
            info->handler->setPermissions(info->toinfo->fileUrl(),info->permission);\
            continue;
        }
        if (skipReadFileDealWriteThread(info)) {
            continue;
        }
        if (info->tofd <= 0) {
            return false;
        }
    write_data: {
//            qDebug() << " write start   ===== " << m_sart;
            qint64 size_write = write(info->tofd, info->buffer, static_cast<size_t>(info->size));
//            qDebug() << "write runing  " << m_write;
            if (Q_UNLIKELY(!stateCheck())) {
                close(info->tofd);
                return false;
            }
            //如果写失败了，直接推出
            if (size_write < 0) {
                if (!stateCheck()) {
                    close(info->tofd);
                    m_fileRefineFd--;
                    delete[] info->buffer;
                    return false;
                }
                switch (setAndhandleError(DFileCopyMoveJob::WriteError, info->frominfo, info->toinfo,
                                          qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause:")/*.
                                          arg(info->todevice->errorString())*/)) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!lseek(info->tofd,info->currentpos,SEEK_SET)) {
                        setError(DFileCopyMoveJob::UnknowError, "");
                        close(info->tofd);
                        m_fileRefineFd--;

                        m_skipFileQueueMutex.lock();
                        m_skipFileQueue.push_back(info->todevice);
                        m_skipFileQueueMutex.unlock();

                        delete[] info->buffer;
                        return false;
                    }
                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    close(info->tofd);
                    m_fileRefineFd--;
                    delete[] info->buffer;

                    m_skipFileQueueMutex.lock();
                    m_skipFileQueue.push_back(info->todevice);
                    m_skipFileQueueMutex.unlock();
                    return true;
                default:
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    close(info->tofd);
                    m_fileRefineFd--;
                    delete[] info->buffer;

                    m_skipFileQueueMutex.lock();
                    m_skipFileQueue.push_back(info->todevice);
                    m_skipFileQueueMutex.unlock();

                    return false;
                }
            }


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
                            //        writtenDataSize += size_write;

                            surplus_data += size_write;
                            surplus_size -= size_write;

                            size_write = write(info->tofd, surplus_data, static_cast<size_t>(surplus_size));
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
                        errorstr = qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause:")/*.arg(info->todevice->errorString())*/;
                    }

                    switch (setAndhandleError(errortype, info->frominfo, info->toinfo, errorstr)) {
                    case DFileCopyMoveJob::RetryAction: {
                        if (!lseek(info->tofd,info->currentpos,SEEK_SET)) {
                            setError(DFileCopyMoveJob::UnknowError, "");
                            delete[] info->buffer;
                            close(info->tofd);
                            m_fileRefineFd--;

                            m_skipFileQueueMutex.lock();
                            m_skipFileQueue.push_back(info->todevice);
                            m_skipFileQueueMutex.unlock();

                            return false;
                        }

                        goto write_data;
                    }
                    case DFileCopyMoveJob::SkipAction:
                        close(info->tofd);
                        m_fileRefineFd--;
                        delete[] info->buffer;

                        m_skipFileQueueMutex.lock();
                        m_skipFileQueue.push_back(info->todevice);
                        m_skipFileQueueMutex.unlock();

                        return true;
                    default:
                        close(info->tofd);
                        m_fileRefineFd--;
                        m_errorUrlList.push_back(info->todevice->fileUrl());
                        delete[] info->buffer;
                        return false;
                    }
                } while (false);
            }
            currentJobDataSizeInfo.second += size_write;
            completedDataSize += size_write;
            completedDataSizeOnBlockDevice += size_write;

            countrefinesize(size_write);
            delete[] info->buffer;
        }
        //关闭文件并加权
        if (info->closeflag) {
            close(info->tofd);
            m_fileRefineFd--;
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

            handler->setPermissions(info->toinfo->fileUrl(), /*source_info->permissions()*/permissions);
        }
    }
    return true;
}

bool DFileCopyMoveJobPrivate::skipReadFileDealWriteThread(const QSharedPointer<DFileCopyMoveJobPrivate::FileCopyInfo> copyinfo)
{
    if (!copyinfo || !copyinfo->toinfo) {
        return false;
    }
    QMutexLocker lk(&m_skipFileQueueMutex);
    return m_skipFileQueue.contains(copyinfo->todevice);
}


void DFileCopyMoveJobPrivate::cancelReadFileDealWriteThread()
{
    QMutexLocker lk(&m_copyInfoQueueMutex);
    while (!m_writeFileQueue.isEmpty()) {
        auto info = m_writeFileQueue.dequeue();
        if (info->todevice) {
            info->todevice->close();
        }
        if (info->tofd > 0) {
            close(info->tofd);
        }
        delete[] info->buffer;
    }
}
void DFileCopyMoveJobPrivate::countAllCopyFile()
{
//    if (mode ==  DFileCopyMoveJob::CopyMode) {
    qint64 times = QDateTime::currentMSecsSinceEpoch();
    for (auto url : sourceUrlList) {
        char *paths[2] = {nullptr, nullptr};
        paths[0] = strdup(url.path().toUtf8().toStdString().data());
        FTS *fts = fts_open(paths, 0, nullptr);
        if (nullptr == fts) {
            perror("fts_open");
            continue;
        }
        while (1) {
            FTSENT *ent = fts_read(fts);
            if (ent == nullptr) {
                printf("walk end\n");
                break;
            }
            unsigned short flag = ent->fts_info;
            if (flag != FTS_DP) {
                totalsize += ent->fts_statp->st_size <= 0 ? 4096 : ent->fts_statp->st_size;
            }
            if (flag == FTS_F) {
                totalfilecount++;
            }
        }
        fts_close(fts);
    }

    iscountsizeover = true;

    emit q_ptr->fileStatisticsFinished();

    qDebug() << " dir time " << QDateTime::currentMSecsSinceEpoch() - times << totalsize;
//    }

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



DFileCopyMoveJob::DFileCopyMoveJob(QObject *parent)
    : DFileCopyMoveJob(*new DFileCopyMoveJobPrivate(this), parent)
{
}

DFileCopyMoveJob::~DFileCopyMoveJob()
{
    qDebug() << "release  DFileCopyMoveJob" << this;
    stop();
    // ###(zccrs): wait() ?
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

    if (d->isFromLocalUrls) {
        if (!d->iscountsizeover) {
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
    return d->m_isShowProgress;
}

bool DFileCopyMoveJob::getSysncState()
{
    Q_D(DFileCopyMoveJob);

    return d->m_bSyncState;
}

bool DFileCopyMoveJob::getSysncQuitState()
{
    Q_D(DFileCopyMoveJob);

    return d->m_bSyncQuitState;
}

void DFileCopyMoveJob::setSysncState(const bool &state)
{
    Q_D(DFileCopyMoveJob);

    d->m_bSyncState = state;
}

void DFileCopyMoveJob::setSysncQuitState(const bool &quitstate)
{
    Q_D(DFileCopyMoveJob);

    d->m_bSyncQuitState = quitstate;
}

bool DFileCopyMoveJob::isLocalFile(const QString &rootpath)
{
    bool isLocal = true;
    GFile *dest_dir_file = g_file_new_for_path(rootpath.toUtf8().constData());
    GMount *dest_dir_mount = g_file_find_enclosing_mount(dest_dir_file, nullptr, nullptr);
    if (dest_dir_mount) {
        isLocal = !g_mount_can_unmount(dest_dir_mount);
        g_object_unref(dest_dir_mount);
    }
    g_object_unref(dest_dir_file);
    return isLocal;
}

bool DFileCopyMoveJob::isFromLocalFile(const DUrlList &urls)
{
    if (urls.isEmpty())
        return true;

    // 一个 job 的原 urllist 目前未发现有同时来自本地和网络的情况
    // 因此只取首个 url 就可判断是否为本地文件, 不必遍历所有文件
    const DUrl &url(urls.at(0));
    return !FileUtils::isGvfsMountFile(url.path());
}

void DFileCopyMoveJob::setRefine(const RefineState &refinestat)
{
    Q_D(DFileCopyMoveJob);

    d->m_refineStat = refinestat;
}

void DFileCopyMoveJob::waitSysncEnd()
{
    Q_D(DFileCopyMoveJob);

    if (d->m_refineStat == NoRefine) {
        return;
    }
    while (d->m_pool.activeThreadCount() > 0 || getSysncState()) {
        if (StoppedState == d->state) {
            d->m_pool.clear();
            d->m_syncResult.cancel();
            break;
        }
        QThread::msleep(50);
    }
}

void DFileCopyMoveJob::waitRefineThreadFinish()
{
    Q_D(DFileCopyMoveJob);
    while (d->m_pool.activeThreadCount() > 0 || !d->m_writeResult.isStarted() || !d->m_writeResult.isFinished()) {
        QThread::msleep(50);
    }
}

//! 用于保存回收站剪切出去的文件在回收站的原始路径
void DFileCopyMoveJob::setCurTrashData(QVariant fileNameList)
{
    Q_D(DFileCopyMoveJob);
    d->setCutTrashData(fileNameList);
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
    case IntegrityCheckingError:
        return SkipAction | RetryAction | CancelAction;
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

    d->sourceUrlList = sourceUrls;
    d->targetUrl = targetUrl;
    d->isFromLocalUrls = isFromLocalFile(d->sourceUrlList);
    if (!d->isFromLocalUrls) {
        if (d->fileStatistics->isRunning()) {
            d->fileStatistics->stop();
            d->fileStatistics->wait();
        }
        d->fileStatistics->start(sourceUrls);
    }

    // DFileStatisticsJob 统计数量很慢，自行统计
    QtConcurrent::run([sourceUrls, d]() {
        if (d->mode == MoveMode || d->mode == CutMode) {
            d->countStatisticsFinished = false;
            for (const auto &url : sourceUrls) {
                QStringList list;
                FileUtils::recurseFolder(url.toLocalFile(), "", &list);
                d->totalMoveFilesCount += (list.size() + 1); // +1 的目的是当前选中的目录要统计到
            }
            d->countStatisticsFinished = true;
        }
    });

    QThread::start();
}

void DFileCopyMoveJob::stop()
{
    Q_D(DFileCopyMoveJob);

    if (d->state == StoppedState) {
        return;
    }

    d->fileStatistics->stop();

    d->setState(StoppedState);
    d->waitCondition.wakeAll();

    emit stopAllGioDervic();
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

DFileCopyMoveJob::DFileCopyMoveJob(DFileCopyMoveJobPrivate &dd, QObject *parent)
    : QThread(parent)
    , d_d_ptr(&dd)
{
    dd.fileStatistics = new DFileStatisticsJob();
    dd.updateSpeedTimer = new QTimer(this);

    connect(dd.fileStatistics, &DFileStatisticsJob::finished, this, &DFileCopyMoveJob::fileStatisticsFinished, Qt::DirectConnection);
    connect(dd.updateSpeedTimer, SIGNAL(timeout()), this, SLOT(_q_updateProgress()), Qt::DirectConnection);
}

void DFileCopyMoveJob::run()
{
    Q_D(DFileCopyMoveJob);


    qCDebug(fileJob()) << "start job, mode:" << d->mode << "file url list:" << d->sourceUrlList << ", target url:" << d->targetUrl;
    qint64 timesec = QDateTime::currentMSecsSinceEpoch();
    d->m_sart = timesec;

    // 本地文件使用 countAllCopyFile 统计大小非常快, 因此不必开辟线程去统计大小. 同步等待文件大小统计完成
    // 网络文件使用以下方式反而会更慢, 因此使用线程统计类
    if (d->targetUrl.isValid() && d->isFromLocalUrls) {
        d->countAllCopyFile();
    }

    d->unsetError();
    d->setState(RunningState);
    d->completedDirectoryList.clear();
    d->completedFileList.clear();
    d->targetUrlList.clear();
    d->completedDataSize = 0;
    d->completedDataSizeOnBlockDevice = 0;
    d->completedFilesCount = 0;
    d->tid = qt_gettid();

    //检查是否需要每次读写都去同步
    d->checkTagetNeedSync();

    DAbstractFileInfoPointer target_info;
    bool mayExecSync = false;
    QPointer<DFileCopyMoveJob> me = this;
//    QPointer<DFileCopyMoveJobPrivate> dp = d;

    if (d->targetUrl.isValid()) {
        target_info = DFileService::instance()->createFileInfo(nullptr, d->targetUrl);

        if (!target_info) {
            d->setError(UnknowUrlError);
            goto end;
        }

        if (!target_info->exists()) {
            d->setError(NonexistenceError, "The target directory non-exists or not permission");
            goto end;
        }

        if (!target_info->isDir()) {
            d->setError(UnknowError, "The target url is not directory");
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
        //检查目标目录是否是块设备
        d->checkTagetIsFromBlockDevice();

        QScopedPointer<DStorageInfo> targetStorageInfo(DFileService::instance()->createStorageInfo(nullptr, d->targetUrl));

        if (targetStorageInfo) {
            d->targetRootPath = targetStorageInfo->rootPath();
            QString rootpath = d->targetRootPath;
            d->m_bDestLocal = isLocalFile(rootpath);
            if (!d->m_bDestLocal) {

                //优化等待1秒后启动异步“同文件”
                QTimer::singleShot(100, this, [me, d, &rootpath]() {
                    if (!me || !d) {
                        return;
                    }
                    d->m_syncResult = QtConcurrent::run([me, d, &rootpath]() {
                        if (!me) {
                            return;
                        }
                        me->setSysncState(true);
                        while (me && d && !me->getSysncQuitState()) {
                            QProcess::execute("sync", {"-f", rootpath});
                        }
                        if (!me) {
                            return;
                        }
                        me->setSysncState(false);
                    });
                });
            }

            qCDebug(fileJob(), "Target block device: \"%s\", Root Path: \"%s\"",
                    targetStorageInfo->device().constData(), qPrintable(d->targetRootPath));

            if (targetStorageInfo->isLocalDevice()) {
                d->canUseWriteBytes = targetStorageInfo->fileSystemType().startsWith("ext");

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
    } else if (d->mode == CopyMode) {
        d->setError(UnknowError, "Invalid target url");
        goto end;
    } else {
        // remove mode
        qCDebug(fileJob(), "remove mode");
    }
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

        const DAbstractFileInfoPointer &source_info = DFileService::instance()->createFileInfo(nullptr, source);
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
            goto end;
        }

        // 只要有一个不是skip或者cancel就执行一下sync
        if (!mayExecSync && d->lastErrorHandleAction != SkipAction &&
                d->lastErrorHandleAction != CancelAction) {
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

    d->setError(NoError);

end:
    //设置同步线程结束
    qDebug() << "mian copy preceess over ===" << QDateTime::currentMSecsSinceEpoch() - timesec;
    if (DFileCopyMoveJob::StoppedState == d->state) {
        d->m_syncResult.cancel();
    }
    //设置优化拷贝线程结束
    d->setRefineCopyProccessSate(ReadFileProccessOver);
    //等待线程池结束,等待异步tongbu线程结束
    qDebug() << "mian copy preceess over ===" << QDateTime::currentMSecsSinceEpoch() - timesec;
    waitRefineThreadFinish();
    setSysncQuitState(true);
    qDebug() << "all copy refine preceess over ===" << QDateTime::currentMSecsSinceEpoch() - timesec;
    waitSysncEnd();
    qDebug() << "sync preceess over ===" << QDateTime::currentMSecsSinceEpoch() - timesec;

    if (!d->m_bDestLocal && d->targetIsRemovable && mayExecSync &&
            d->state != DFileCopyMoveJob::StoppedState) { //主动取消时state已经被设置为stop了
        // 任务完成后执行 sync 同步数据到硬盘, 同时将状态改为 SleepState，用于定时器更新进度和速度信息
        d->setState(IOWaitState);
        int syncRet = 0;
        d->m_syncResult = QtConcurrent::run([me, &d, &syncRet]() {
            qDebug() << "sync >>>>>>>>>>>>>>";
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

    d->fileStatistics->stop();
    d->setState(StoppedState);

    if (d->error == NoError) {
        Q_EMIT progressChanged(1, d->completedDataSize);
    }

    qCDebug(fileJob()) << "job finished, error:" << error() << ", message:" << errorString() << QDateTime::currentMSecsSinceEpoch() - timesec;
}

QString DFileCopyMoveJob::Handle::getNewFileName(DFileCopyMoveJob *job, const DAbstractFileInfoPointer sourceInfo)
{
    Q_UNUSED(job)

    return job->d_func()->formatFileName(sourceInfo->fileName());
}

QString DFileCopyMoveJob::Handle::getNonExistsFileName(const DAbstractFileInfoPointer sourceInfo, const DAbstractFileInfoPointer targetDirectory)
{
    return DFileCopyMoveJobPrivate::getNewFileName(sourceInfo, targetDirectory);
}

DFM_END_NAMESPACE

#include "moc_dfilecopymovejob.cpp"
