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
#include "interfaces/dfmstandardpaths.h"
#include "shutil/fileutils.h"

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
    m_pool.setMaxThreadCount(24);
}

DFileCopyMoveJobPrivate::~DFileCopyMoveJobPrivate()
{
    qDebug() << "DFileCopyMoveJobPrivate " << QDateTime::currentMSecsSinceEpoch() - m_sart;
    while (!closefromresult.isFinished() || !addper.isFinished() || !closedevice.isFinished()
           || !openfrom.isFinished() || !copyresult.isFinished() || !writeresult.isFinished()) {
        qDebug() << "DFileCopyMoveJobPrivate all thread over ooo" << QDateTime::currentMSecsSinceEpoch() - m_sart;
        qApp->processEvents();
    }
    delete updateSpeedElapsedTimer;
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

    Q_EMIT q->stateChanged(s);

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

    qCDebug(fileJob()) << "state changed, new state:" << s;
}

void DFileCopyMoveJobPrivate::setError(DFileCopyMoveJob::Error e, const QString &es)
{
    QMutexLocker lk(&m_errormutex);
    if (DFileCopyMoveJob::CancelError <= error && error == e) {
        return;
    }

    if (DFileCopyMoveJob::CancelError < error && !cansetnoerror && DFileCopyMoveJob::StoppedState != state) {
       setState(DFileCopyMoveJob::PausedState);
       if (!Q_LIKELY(stateCheck())) {
           qDebug() << "cuowuchuangtai ";
           return;
       }
    }
    if (DFileCopyMoveJob::CancelError < e) {
        cansetnoerror = false;
    }
    else {
        cansetnoerror = true;
    }

    error = e;
    errorString = es.isEmpty() ? errorToString(e) : es;

    Q_Q(DFileCopyMoveJob);

    Q_EMIT q->errorChanged(e);

    qCDebug(fileJob()) << "new error, type=" << e << ", message=" << es;
}

void DFileCopyMoveJobPrivate::unsetError()
{
    setError(DFileCopyMoveJob::NoError);
}

DFileCopyMoveJob::Action DFileCopyMoveJobPrivate::handleError(const DAbstractFileInfo *sourceInfo,
                                                              const DAbstractFileInfo *targetInfo)
{
    //当任务对话框结束返回cancel
    if (btaskdailogclose) {
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

        return lastErrorHandleAction;
    }

    if (!handle) {
        switch (error) {
        case DFileCopyMoveJob::PermissionError:
        case DFileCopyMoveJob::UnknowUrlError:
        case DFileCopyMoveJob::TargetIsSelfError:
            lastErrorHandleAction = DFileCopyMoveJob::SkipAction;
            cansetnoerror = true;
            unsetError();
            break;
        case DFileCopyMoveJob::FileExistsError:
        case DFileCopyMoveJob::DirectoryExistsError:
            lastErrorHandleAction = DFileCopyMoveJob::CoexistAction;
            cansetnoerror = true;
            unsetError();
            break;
        default:
            lastErrorHandleAction = DFileCopyMoveJob::CancelAction;
            cansetnoerror = true;
            setError(DFileCopyMoveJob::CancelError);
            break;
        }

        qCDebug(fileJob()) << "no handle," << "default action:" << lastErrorHandleAction
                           << "source url:" << sourceInfo->fileUrl()
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
    cansetnoerror = true;

    if (state == DFileCopyMoveJob::SleepState) {
        setState(DFileCopyMoveJob::RunningState);
    }

    if (lastErrorHandleAction == DFileCopyMoveJob::NoAction) {
        lastErrorHandleAction = DFileCopyMoveJob::CancelAction;
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
    if (directoryStack.isEmpty()){
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

QString DFileCopyMoveJobPrivate::getNewFileName(const DAbstractFileInfoPointer &sourceFileInfo, const DAbstractFileInfoPointer &targetDirectory)
{
    const QString &copy_text = QCoreApplication::translate("DFileCopyMoveJob", "copy",
                                                           "Extra name added to new file name when used for file name.");

    DAbstractFileInfoPointer target_file_info;
    QString file_base_name = sourceFileInfo->baseName();
    QString suffix = sourceFileInfo->suffix();
    QString filename = sourceFileInfo->fileName();
    //在7z分卷压缩后的名称特殊处理7z.003
//    if(filename.contains(QRegularExpression("\.7z\.[0-9]{3,10}$")))
//    {
//        file_base_name = filename.left(filename.indexOf(QRegularExpression("\.7z\.[0-9]{3,10}$")));
//        suffix = filename.mid(filename.indexOf(QRegularExpression("\.7z\.[0-9]{3,10}$"))+1);
//    }
    //'\'没有转义为了避免警告加了转义
    if (filename.contains(QRegularExpression("\\.7z\\.[0-9]{3,10}$"))) {
        file_base_name = filename.left(filename.indexOf(QRegularExpression("\\.7z\\.[0-9]{3,10}$")));
        suffix = filename.mid(filename.indexOf(QRegularExpression("\\.7z\\.[0-9]{3,10}$")) + 1);
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

bool DFileCopyMoveJobPrivate::doProcess(const DUrl &from, const DAbstractFileInfoPointer &source_info, const DAbstractFileInfoPointer &target_info, const bool isNew)
{
//    Q_Q(DFileCopyMoveJob);

    if (!source_info) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed create file info");

        return handleError(source_info.constData(), nullptr) == DFileCopyMoveJob::SkipAction;
    }

    if (!source_info->exists()) {
        setError(DFileCopyMoveJob::NonexistenceError);

        return handleError(source_info.constData(), nullptr) == DFileCopyMoveJob::SkipAction;
    }

    switch (source_info->fileType()) {
    case DAbstractFileInfo::CharDevice:
    case DAbstractFileInfo::BlockDevice:
    case DAbstractFileInfo::FIFOFile:
    case DAbstractFileInfo::SocketFile: {
        setError(DFileCopyMoveJob::SpecialFileError);

        return handleError(source_info.constData(), nullptr) == DFileCopyMoveJob::SkipAction;
    }
    default:
        break;
    }

    DFileHandler *handler = DFileService::instance()->createFileHandler(nullptr, from);

    if (!handler) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed create file handler");

        return handleError(source_info.constData(), nullptr) == DFileCopyMoveJob::SkipAction;
    }

    // only remove
    if (!target_info) {
        bool ok = false;

        qint64 size = source_info->isSymLink() ? 0 : source_info->size();

        if (source_info->isFile() || source_info->isSymLink()) {
            qDebug() << "removeFile  ";
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
    file_name = isNew ? source_info->fileName() : (handle ? handle->getNewFileName(q_ptr, source_info) : source_info->fileName());

    // 回收站可能重名文件，因此回收站中的文件实际filename是经过处理的,这里需要取真正需要展示的filename
    if (source_info->filePath().startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))) {
        QExplicitlySharedDataPointer<TrashFileInfo> info(new TrashFileInfo(DUrl::fromTrashFile("/" + source_info->fileName())));
        file_name = info->fileDisplayName();
    }
create_new_file_info:
    const DAbstractFileInfoPointer &new_file_info = DFileService::instance()->createFileInfo(nullptr, target_info->getUrlByChildFileName(file_name));

    if (!new_file_info) {
        qWarning() << "fileinfo create failed!" << target_info->getUrlByChildFileName(file_name);
        return false;
    }

    if (new_file_info->exists()) {
        if ((mode == DFileCopyMoveJob::MoveMode || mode == DFileCopyMoveJob::CutMode) &&
                (new_file_info->fileUrl() == from || DStorageInfo::isSameFile(from.path(), new_file_info->fileUrl().path()))) {
                // 不用再进行后面的操作
                return true;
        }

        // 禁止目录复制/移动到自己里面
        if (new_file_info->isAncestorsUrl(source_info->fileUrl())) {
            setError(DFileCopyMoveJob::TargetIsSelfError);

            DFileCopyMoveJob::Action action = handleError(source_info.constData(), new_file_info.constData());

            if (action == DFileCopyMoveJob::SkipAction) {
                return true;
            }

            if (action != DFileCopyMoveJob::EnforceAction) {
                return false;
            }
        }

        bool source_is_file = source_info->isFile() || source_info->isSymLink();
        bool target_is_file = new_file_info->isFile() || new_file_info->isSymLink();

        if (target_is_file) {
            setError(DFileCopyMoveJob::FileExistsError);
        } else {
            setError(DFileCopyMoveJob::DirectoryExistsError);
        }

        switch (handleError(source_info.constData(), new_file_info.constData())) {
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
            return true;
        case DFileCopyMoveJob::CoexistAction:
            file_name = handle ? handle->getNonExistsFileName(q_ptr, source_info, target_info)
                        : getNewFileName(source_info, target_info);
            goto create_new_file_info;
        default:
            return false;
        }
    }

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
            setError(DFileCopyMoveJob::NotEnoughSpaceError);
            DFileCopyMoveJob::Action action = handleError(source_info.constData(), new_file_info.constData());

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
            setError(DFileCopyMoveJob::FileSizeTooBigError);
            DFileCopyMoveJob::Action action = handleError(source_info.constData(), new_file_info.constData());

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
            ok = renameFile(handler, source_info, new_file_info);
        }

        if (ok) {
            joinToCompletedFileList(from, new_file_info->fileUrl(), size);
        }

        return ok;
    } else if (source_info->isDir()) {
        static qint64 alldir = 0;
        qint64 cer = QDateTime::currentMSecsSinceEpoch();
        // 禁止目录复制/移动到自己里面
        if (new_file_info->isAncestorsUrl(source_info->fileUrl())) {
            setError(DFileCopyMoveJob::TargetIsSelfError);

            DFileCopyMoveJob::Action action = handleError(source_info.constData(), new_file_info.constData());

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
        alldir = alldir + (QDateTime::currentMSecsSinceEpoch() - cer);

        return ok;
    }

    return false;
}

bool DFileCopyMoveJobPrivate::mergeDirectory(DFileHandler *handler, const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo)
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
                    setError(DFileCopyMoveJob::MkdirError, qApp->translate("DFileCopyMoveJob", "Failed to open the dir, cause: File name too long"));
                    action = handleError(fromInfo.data(), toInfo.data());
                    break;
                }
            }

            if (!handler->mkdir(toInfo->fileUrl())) {
                const DAbstractFileInfoPointer &parent_info = DFileService::instance()->createFileInfo(nullptr, toInfo->parentUrl());

                if (!parent_info->exists() || parent_info->isWritable()) {
                    setError(DFileCopyMoveJob::MkdirError, qApp->translate("DFileCopyMoveJob", "Failed to create the directory, cause: %1").arg(handler->errorString()));
                } else {
                    setError(DFileCopyMoveJob::PermissionError);
                }

                action = handleError(fromInfo.data(), toInfo.data());
            }
            isNew = true;
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

        if (action != DFileCopyMoveJob::NoAction) {
            return action == DFileCopyMoveJob::SkipAction;
        }
    }

    if (fromInfo->filesCount() <= 0 && mode == DFileCopyMoveJob::CopyMode) {
        QFileDevice::Permissions permissions;
        QString filePath = fromInfo->fileUrl().toLocalFile();
        if (VaultController::ins()->isVaultFile(filePath)) {
            permissions = VaultController::ins()->getPermissions(filePath);
        } else {
            permissions = fromInfo->permissions();
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
        QFileDevice::Permissions permissions;
        QString filePath = fromInfo->fileUrl().toLocalFile();
        if (VaultController::ins()->isVaultFile(filePath)) {
            permissions = VaultController::ins()->getPermissions(filePath);
        } else {
            permissions = fromInfo->permissions();
        }

        handler->setPermissions(toInfo->fileUrl(), permissions);
    }

    if (mode == DFileCopyMoveJob::CopyMode) {
        return true;
    }

    if (existsSkipFile) {
        return true;
    }

    // 完成操作后删除原目录
    return removeFile(handler, fromInfo);
}

bool DFileCopyMoveJobPrivate::doCopyFile(const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo, DFileHandler *handler, int blockSize)
{
    //预先读取
    {
        int fromfd = open(fromInfo->fileUrl().path().toUtf8().toStdString().data(),O_RDONLY);
        if (-1 != fromfd) {
            readahead(fromfd,0,static_cast<size_t>(fromInfo->size()));
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
                    setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: File name too long"));
                    action = handleError(fromInfo.constData(), nullptr);
                    break;
                }
            }

            if (fromDevice->open(QIODevice::ReadOnly)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << fromInfo->fileUrl();

                if (fromInfo->isReadable()) {
                    setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(fromDevice->errorString()));
                } else {
                    setError(DFileCopyMoveJob::PermissionError);
                }

                action = handleError(fromInfo.constData(), nullptr);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() ); // bug: 26333, while set the stop status shoule break the process!

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        do {
            if (toDevice->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Append)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << toInfo->fileUrl();

                if (!toInfo->exists() || toInfo->isWritable()) {
                    setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(toDevice->errorString()));
                } else {
                    setError(DFileCopyMoveJob::PermissionError);
                }

                action = handleError(toInfo.data(), nullptr);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        if (fileHints.testFlag(DFileCopyMoveJob::ResizeDestinationFile)) {
            do {
                if (toDevice->resize(fromDevice->size())) {
                    action = DFileCopyMoveJob::NoAction;
                } else {
                    setError(DFileCopyMoveJob::ResizeError, toDevice->errorString());
                    action = handleError(toInfo.data(), nullptr);
                }
            } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

            if (action == DFileCopyMoveJob::SkipAction) {
                return true;
            } else if (action != DFileCopyMoveJob::NoAction) {
                return false;
            }
        }
    }

#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (fromDevice->handle() > 0) {
        posix_fadvise (fromDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }

    if (toDevice->handle() > 0) {
        posix_fadvise (toDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif

    currentJobDataSizeInfo.first = fromDevice->size();
    currentJobFileHandle = toDevice->handle();
    bool teestqq = false;
    uLong source_checksum = adler32(0L, nullptr, 0);

    qint64 block_Size = fromInfo->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : fromInfo->size();

    char *data = new char[block_Size +1];

    Q_FOREVER {
        qint64 current_pos = fromDevice->pos();
read_data:
        if (Q_UNLIKELY(!stateCheck())) {
            delete[] data;
            return false;
        }

//        char data[blockSize + 1];
        qint64 size_read = fromDevice->read(data, block_Size);

        if (Q_UNLIKELY(size_read <= 0)) {
            if (size_read == 0 && fromDevice->atEnd()) {
                teestqq = true;
                break;
            }

            const_cast<DAbstractFileInfo *>(fromInfo.data())->refresh();

            if (fromInfo->exists()) {
                setError(DFileCopyMoveJob::ReadError, qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: %1").arg(fromDevice->errorString()));
            } else {
                setError(DFileCopyMoveJob::NonexistenceError);
            }

            switch (handleError(fromInfo.data(), toInfo.data())) {
            case DFileCopyMoveJob::RetryAction: {
                if (!fromDevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, fromDevice->errorString());
                    delete[] data;
                    return false;
                }

                goto read_data;
            }
            case DFileCopyMoveJob::SkipAction:
                delete[] data;
                return true;
            default:
                delete[] data;
                return false;
            }
        }

        current_pos = toDevice->pos();
write_data:
        if (Q_UNLIKELY(!stateCheck())) {
            return false;
        }
        qint64 size_write = toDevice->write(data, size_read);
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
                return false;
            }
           setError(DFileCopyMoveJob::WriteError, qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString()));
           switch (handleError(fromInfo.data(), toInfo.data())) {
           case DFileCopyMoveJob::RetryAction: {
               if (!toDevice->seek(current_pos)) {
                   setError(DFileCopyMoveJob::UnknowError, toDevice->errorString());
                   //临时处理 fix
                   //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                   //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                   if (FileUtils::isGvfsMountFile(toInfo->path())) {
                       toDevice->closeWriteReadFailed(true);
                   }
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
                delete[] data;
                return true;
            default:
                //临时处理 fix
                //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                if (FileUtils::isGvfsMountFile(toInfo->path())) {
                    toDevice->closeWriteReadFailed(true);
                }
                delete[] data;
                return false;
            }
        }
        //fix 修复vfat格式u盘卡死问题，写入数据后立刻同步
//        const DStorageInfo &targetStorageInfo = directoryStack.top().targetStorageInfo;
//        if (targetStorageInfo.isValid()) {
//            const QString &fs_type = targetStorageInfo.fileSystemType();
//            if (fs_type == "vfat") {
//                toDevice->inherits("");
//                if (size_write > 0)
//                    toDevice->syncToDisk();
//            }
//        }
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
                    } while (size_write > 0 && size_write != surplus_size);

                    // 表示全部数据写入完成
                    if (size_write > 0) {
                        break;
                    }
                }

                if (checkFreeSpace(currentJobDataSizeInfo.first - currentJobDataSizeInfo.second)) {
                    setError(DFileCopyMoveJob::WriteError, qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString()));
                } else {
                    setError(DFileCopyMoveJob::NotEnoughSpaceError);
                }

                switch (handleError(fromInfo.data(), toInfo.data())) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!toDevice->seek(current_pos)) {
                        setError(DFileCopyMoveJob::UnknowError, toDevice->errorString());
                        delete[] data;
                        return false;
                    }

                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    delete[] data;
                    return true;
                default:
                    delete[] data;
                    return false;
                }
            } while (false);
        }

        currentJobDataSizeInfo.second += size_write;
        completedDataSize += size_write;
        completedDataSizeOnBlockDevice += size_write;
//        writtenDataSize += size_write;

        if (Q_LIKELY(!fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking))) {
//            source_checksum = adler32(source_checksum, reinterpret_cast<Bytef *>(data), size_read);
            source_checksum = adler32(source_checksum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(size_read));
        }

//        if (Q_UNLIKELY(writtenDataSize > 20000000)) {
//            writtenDataSize = 0;
//            toDevice->syncToDisk();
//        }
    }
// end forever
//    qDebug() << "cope satart end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << fromInfo->fileUrl();
    // 关闭文件时可能会需要很长时间，因为内核可能要把内存里的脏数据回写到硬盘
//    setState(DFileCopyMoveJob::IOWaitState);
    delete[] data;
    fromDevice->close();
    toDevice->close();
    countrefinesize(fromInfo->size() <= 0 ? 4096 : 0);
    /*if (state == DFileCopyMoveJob::IOWaitState) {
        setState(DFileCopyMoveJob::RunningState);
    } else */

    //对文件加权
    handler->setFileTime(toInfo->fileUrl(), fromInfo->lastRead(), fromInfo->lastModified());

    QFileDevice::Permissions permissions = fromInfo->permissions();
    //! use stat function to read vault file permission.
    QString path = fromInfo->fileUrl().path();
    if (VaultController::isVaultFile(path)) {
        permissions = VaultController::getPermissions(path);
    }

    handler->setPermissions(toInfo->fileUrl(), /*source_info->permissions()*/permissions);


    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }
//    qDebug() << "cope satart end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << fromInfo->fileUrl();

    if (fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking)) {
        return true;
    }

    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

    do {
        if (toDevice->open(QIODevice::ReadOnly)) {
            break;
        } else {
            setError(DFileCopyMoveJob::OpenError, "Unable to open file for integrity check, , cause: " + toDevice->errorString());
            action = handleError(toInfo.data(), nullptr);
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
            if (toDevice->atEnd()) {
                break;
            }

            setError(DFileCopyMoveJob::IntegrityCheckingError, qApp->translate("DFileCopyMoveJob", "File integrity was damaged, cause: %1").arg(toDevice->errorString()));

            switch (handleError(fromInfo.constData(), toInfo.constData())) {
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

//    qCDebug(fileJob(), "Time spent of integrity check of the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed_time_checksum);

    if (source_checksum != target_checksum) {
        qCWarning(fileJob(), "Failed on file integrity checking, source file: 0x%lx, target file: 0x%lx", source_checksum, target_checksum);

        setError(DFileCopyMoveJob::IntegrityCheckingError);
        DFileCopyMoveJob::Action action = handleError(fromInfo.constData(), toInfo.constData());

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        }

        if (action == DFileCopyMoveJob::RetryAction) {
            goto open_file;
        }

        return false;
    }

//    qDebug() << "yi ci yige +++++++++++++++++ " << fromInfo->fileUrl();
//    qCDebug(fileJob(), "adler value: 0x%lx", source_checksum);

    return true;
}

bool DFileCopyMoveJobPrivate::doRemoveFile(DFileHandler *handler, const DAbstractFileInfoPointer &fileInfo)
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
        if (VaultController::isVaultFile(absolutePath)) {
            VaultController::FileBaseInfo fbi = VaultController::ins()->getFileInfo(VaultController::localToVault(absolutePath));
            if (!fbi.isWritable) {
                canRemove = false;
                setError(DFileCopyMoveJob::PermissionError);
            }
        }

        if(canRemove) {
            if (is_file ? handler->remove(fileInfo->fileUrl()) : handler->rmdir(fileInfo->fileUrl())) {
                return true;
            }
            if (fileInfo->canRename()) {
                setError(DFileCopyMoveJob::RemoveError, qApp->translate("DFileCopyMoveJob", "Failed to delete the file, cause: %1").arg(handler->errorString()));
            } else {
                setError(DFileCopyMoveJob::PermissionError);
            }
        }

        action = handleError(fileInfo.constData(), nullptr);
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

    return action == DFileCopyMoveJob::SkipAction;
}

bool DFileCopyMoveJobPrivate::doRenameFile(DFileHandler *handler, const DAbstractFileInfoPointer &oldInfo, const DAbstractFileInfoPointer &newInfo)
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
                if (VaultController::isVaultFile(oldInfo->path()) || VaultController::isVaultFile(newInfo->path())) { // 如果是保险箱任务

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

    //! use stat function to read vault file permission.
    QFileDevice::Permissions permissions = oldInfo->permissions();
    QString path = oldInfo->fileUrl().path();
    if (VaultController::isVaultFile(path)) {
        permissions = VaultController::getPermissions(path);
    }

    handler->setPermissions(newInfo->fileUrl(), /*oldInfo->permissions()*/permissions);

    if (!doRemoveFile(handler, oldInfo)) {
        return false;
    }

    return true;
}

bool DFileCopyMoveJobPrivate::doLinkFile(DFileHandler *handler, const DAbstractFileInfoPointer &fileInfo, const QString &linkPath)
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

        setError(DFileCopyMoveJob::SymlinkError, qApp->translate("DFileCopyMoveJob", "Fail to create symlink, cause: %1").arg(handler->errorString()));
        action = handleError(fileInfo.constData(), nullptr);
        q->msleep(500); // fix bug#30091 文件操作失败的时候，点击对话框的“不再提示+重试”，会导致不停失败不停发送信号通知主线程更新ui，这里加个延时控制响应频率
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

    return action == DFileCopyMoveJob::SkipAction;
}

bool DFileCopyMoveJobPrivate::process(const DUrl &from, const DAbstractFileInfoPointer &target_info)
{
    const DAbstractFileInfoPointer &source_info = DFileService::instance()->createFileInfo(nullptr, from);

    return process(from, source_info, target_info);
}

bool DFileCopyMoveJobPrivate::process(const DUrl &from, const DAbstractFileInfoPointer &source_info, const DAbstractFileInfoPointer &target_info, const bool isNew)
{
    // reset error and action
    if (DFileCopyMoveJob::MoreThreadAndMainRefine <= refinestat) {
        return processRefine(from,source_info,target_info,isNew);
    }
    unsetError();
    lastErrorHandleAction = DFileCopyMoveJob::NoAction;

    beginJob(JobInfo::Preprocess, from, (source_info && target_info) ? target_info->getUrlByChildFileName(source_info->fileName()) : DUrl());
    bool ok = doProcess(from, source_info, target_info,isNew);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::copyFile(const DAbstractFileInfoPointer &fromInfo, const DAbstractFileInfoPointer &toInfo, DFileHandler *handler, int blockSize)
{
//    Q_Q(DFileCopyMoveJob);
    qint64 elapsed = 0;
    if (fileJob().isDebugEnabled()) {
        elapsed = updateSpeedElapsedTimer->elapsed();
    }
    beginJob(JobInfo::Copy, fromInfo->fileUrl(), toInfo->fileUrl());
    bool ok = true;
    //判断目标目录是u盘就只走但线程
    if (!bdestLocal || DFileCopyMoveJob::MoreThreadRefine > refinestat) {
        ok = doCopyFile(fromInfo, toInfo, handler, blockSize);
    }
    else {
        while(m_pool.activeThreadCount() >= 24)
        {
            if (Q_UNLIKELY(!stateCheck())) {
                return false;
            }
        }
        QtConcurrent::run(&m_pool, this, static_cast<bool(DFileCopyMoveJobPrivate::*)
                         (const DAbstractFileInfoPointer &, const DAbstractFileInfoPointer &, DFileHandler *, int)>
                         (&DFileCopyMoveJobPrivate::doCopyFile), fromInfo, toInfo, handler, blockSize);
    }
    endJob();

//    qCDebug(fileJob(), "Time spent of copy the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed);
    return ok;
}

bool DFileCopyMoveJobPrivate::removeFile(DFileHandler *handler, const DAbstractFileInfoPointer &fileInfo)
{
    beginJob(JobInfo::Remove, fileInfo->fileUrl(), DUrl());
    bool ok = doRemoveFile(handler, fileInfo);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::renameFile(DFileHandler *handler, const DAbstractFileInfoPointer &oldInfo, const DAbstractFileInfoPointer &newInfo)
{
    Q_UNUSED(handler);

    beginJob(JobInfo::Move, oldInfo->fileUrl(), newInfo->fileUrl());
    bool ok = doRenameFile(handler, oldInfo, newInfo);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::linkFile(DFileHandler *handler, const DAbstractFileInfoPointer &fileInfo, const QString &linkPath)
{
    beginJob(JobInfo::Link, DUrl(linkPath), fileInfo->fileUrl());
    bool ok = doLinkFile(handler, fileInfo, linkPath);
    endJob();

    return ok;
}

void DFileCopyMoveJobPrivate::beginJob(JobInfo::Type type, const DUrl &from, const DUrl &target, const bool isNew)
{
//    qCDebug(fileJob(), "job begin, Type: %d, from: %s, to: %s", type, qPrintable(from.toString()), qPrintable(target.toString()));
    if (!isNew) {
        jobStack.push({type, QPair<DUrl, DUrl>(from, target)});
        currentJobDataSizeInfo = qMakePair(-1, 0);
        currentJobFileHandle = -1;
    }

    Q_EMIT q_ptr->currentJobChanged(from, target,false);
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

void DFileCopyMoveJobPrivate::enterDirectory(const DUrl &from, const DUrl &to)
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

void DFileCopyMoveJobPrivate::joinToCompletedFileList(const DUrl &from, const DUrl &target, qint64 dataSize)
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

void DFileCopyMoveJobPrivate::joinToCompletedDirectoryList(const DUrl &from, const DUrl &target, qint64 dataSize)
{
    Q_UNUSED(dataSize)
//    qCDebug(fileJob(), "directory. from: %s, target: %s, data size: %lld", qPrintable(from.toString()), qPrintable(target.toString()), dataSize);

//    completedDataSize += dataSize;
    completedProgressDataSize += targetIsRemovable <= 0 ? 4096 : 0;
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
    // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
//    const qint64 totalSize = fileStatistics->totalProgressSize();
    const qint64 totalSize = totalsize;
    //通过getCompletedDataSize取出的已传输的数据大小后期会远超实际数据大小，这种情况下直接使用completedDataSize
    qint64 dataSize(getCompletedDataSize());
    // completedDataSize 可能一直为 0
    if (dataSize > completedDataSize && completedDataSize > 0) {
        dataSize = completedDataSize;
    }

    dataSize = targetIsRemovable <= 0 ? completedDataSizeOnBlockDevice : dataSize;

    dataSize += completedProgressDataSize;

    //优化
    dataSize = (refinestat >= DFileCopyMoveJob::MoreThreadRefine && bdestLocal) ? refinecpsize : dataSize;

    if (totalSize == 0)
        return;

//    if (fileStatistics->isFinished()) {
    if (iscountsizeover) {
        qreal realProgress = qreal(dataSize) / totalSize;
        if (realProgress > lastProgress)
            lastProgress = realProgress;
//        qCDebug(fileJob(), "completed data size: %lld, total data size: %lld", dataSize, fileStatistics->totalProgressSize());
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
    const qint64 total_size = refinestat >= DFileCopyMoveJob::MoreThreadRefine ? refinecpsize : getCompletedDataSize();
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
    QMutexLocker lock(&m_refinemutex);
    refinecpsize += size;
}

bool DFileCopyMoveJobPrivate::mergeDirectoryRefine(DFileHandler *handler, const DAbstractFileInfoPointer &fromInfo,
                                                  const DAbstractFileInfoPointer &toInfo)
{
    //copyrefineflag为0,遍历线程才会继续去遍历
    if (copyrefineflag >= 1) {
        return true;
    }
    bool nextischeck = false;
    if (toInfo && !toInfo->exists()) {
        DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
        do {
            // 当为保险箱路径时，判断目录名的长度，如果长度大于85，则不让其创建成功，并报错“文件名过长”
            QString strPath = toInfo->fileUrl().toString();
            if (VaultController::isVaultFile(strPath)) {
                // 获得目录名
                QString strDirName = strPath.section("/", -1, -1);
                if (strDirName.toUtf8().length() > 255) {
                    setError(DFileCopyMoveJob::MkdirError, qApp->translate("DFileCopyMoveJob", "Failed to open the dir, cause: File name too long"));
                    action = handleError(fromInfo.constData(), toInfo.constData());
                    break;
                }
            }
            if (!handler->mkdir(toInfo->fileUrl())) {
                const DAbstractFileInfoPointer &parent_info = DFileService::instance()->createFileInfo(nullptr, toInfo->parentUrl());

                if (!parent_info->exists() || parent_info->isWritable()) {
                    setError(DFileCopyMoveJob::MkdirError, qApp->translate("DFileCopyMoveJob", "Failed to create the directory, cause: %1").arg(handler->errorString()));
                } else {
                    setError(DFileCopyMoveJob::PermissionError);
                }

                action = handleError(fromInfo.constData(), toInfo.constData());
            }
            nextischeck = true;

        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

        if (action != DFileCopyMoveJob::NoAction) {
            return action == DFileCopyMoveJob::SkipAction;
        }
    }
    if (fromInfo->filesCount() <= 0 && mode == DFileCopyMoveJob::CopyMode) {
        FileCopyInfoPointer copyinfo(new FileCopyInfo);
        copyinfo->isdir = true;
        copyinfo->frominfo = fromInfo;
        copyinfo->toinfo = toInfo;
        if (refinestat == DFileCopyMoveJob::MoreThreadAndMainAndOpenRefine) {
            openfromfilequeue.enqueue(copyinfo);
        }
        //直接加权
        QFileDevice::Permissions permissions;
        QString filePath = fromInfo->fileUrl().toLocalFile();
        if (VaultController::ins()->isVaultFile(filePath)) {
            permissions = VaultController::ins()->getPermissions(filePath);
        } else {
            permissions = fromInfo->permissions();
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

        if (!processRefine(url, info, toInfo, nextischeck)) {
            return false;
        }

        if (lastErrorHandleAction == DFileCopyMoveJob::SkipAction) {
            existsSkipFile = true;
        }
    }

    if (enter_dir) {
        leaveDirectory();
    }
    if (mode == DFileCopyMoveJob::CopyMode) {
        FileCopyInfoPointer copyinfo(new FileCopyInfo);
        copyinfo->isdir = true;
        copyinfo->frominfo = fromInfo;
        copyinfo->toinfo = toInfo;
        copyinfo->handler = handler;
        if (refinestat == DFileCopyMoveJob::MoreThreadAndMainAndOpenRefine) {
            openfromfilequeue.enqueue(copyinfo);
        }
        else {
            readfileinfoqueue.enqueue(copyinfo);
        }
        return true;
    }
    //处理文件权限问题
    if (toInfo) {

        // vault file fetch permissons separately.
        QFileDevice::Permissions permissions;
        QString filePath = fromInfo->fileUrl().toLocalFile();
        if (VaultController::ins()->isVaultFile(filePath)) {
            permissions = VaultController::ins()->getPermissions(filePath);
        } else {
            permissions = fromInfo->permissions();
        }

        handler->setPermissions(toInfo->fileUrl(), permissions);
    }

    if (existsSkipFile) {
        return true;
    }

    // 完成操作后删除原目录
    return removeFile(handler, fromInfo);
}

bool DFileCopyMoveJobPrivate::processRefine(const DUrl &from, const DAbstractFileInfoPointer &source_info,
                                            const DAbstractFileInfoPointer &target_info, const bool ischeck)
{
    // reset error and action
    unsetError();
    lastErrorHandleAction = DFileCopyMoveJob::NoAction;

    beginJob(JobInfo::Preprocess, from, (source_info && target_info) ? target_info->getUrlByChildFileName(source_info->fileName()) : DUrl(),true);
    bool ok = doProcessRefine(from, source_info, target_info,ischeck);
    endJob(true);

    return ok;
}

bool DFileCopyMoveJobPrivate::doProcessRefine(const DUrl &from, const DAbstractFileInfoPointer &source_info,
                                               const DAbstractFileInfoPointer &target_info, const bool ischeck)
{
//    Q_Q(DFileCopyMoveJob);
    qint64 curt = QDateTime::currentMSecsSinceEpoch();
    if (!source_info) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed create file info");

        return handleError(source_info.constData(), nullptr) == DFileCopyMoveJob::SkipAction;
    }

    if (!source_info->exists()) {
        //如果是从root目录拷贝到普通用户的目录，需要提示权限错误
        if (source_info->path().startsWith("/root/") && !target_info->path().startsWith("/root/")) {
            setError(DFileCopyMoveJob::PermissionError);
        } else {
            setError(DFileCopyMoveJob::NonexistenceError);
        }

        return handleError(source_info.constData(), nullptr) == DFileCopyMoveJob::SkipAction;
    }

    switch (source_info->fileType()) {
    case DAbstractFileInfo::CharDevice:
    case DAbstractFileInfo::BlockDevice:
    case DAbstractFileInfo::FIFOFile:
    case DAbstractFileInfo::SocketFile: {
        setError(DFileCopyMoveJob::SpecialFileError);

        return handleError(source_info.constData(), nullptr) == DFileCopyMoveJob::SkipAction;
    }
    default:
        break;
    }

    DFileHandler *handler = DFileService::instance()->createFileHandler(nullptr, from);

    if (!handler) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed create file handler");

        return handleError(source_info.constData(), nullptr) == DFileCopyMoveJob::SkipAction;
    }

    // only remove
    if (!target_info) {
        bool ok = false;

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
    if (!ischeck) {
        file_name = handle ? handle->getNewFileName(q_ptr, source_info) : source_info->fileName();
    }
    else {
        file_name = source_info->fileName();
    }

    // 回收站可能重名文件，因此回收站中的文件实际filename是经过处理的,这里需要取真正需要展示的filename
    if (source_info->filePath().startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))) {
        QExplicitlySharedDataPointer<TrashFileInfo> info(new TrashFileInfo(DUrl::fromTrashFile("/" + source_info->fileName())));
        file_name = info->fileDisplayName();
    }
//    m_check += QDateTime::currentMSecsSinceEpoch() - curt;
    curt = QDateTime::currentMSecsSinceEpoch();
create_new_file_info:
    const DAbstractFileInfoPointer &new_file_info = DFileService::instance()->createFileInfo(nullptr, target_info->getUrlByChildFileName(file_name));

    if (!new_file_info) {
        qWarning() << "fileinfo create failed!" << target_info->getUrlByChildFileName(file_name);
        return false;
    }

    if (new_file_info->exists()) {
        if ((mode == DFileCopyMoveJob::MoveMode || mode == DFileCopyMoveJob::CutMode) &&
                (new_file_info->fileUrl() == from || DStorageInfo::isSameFile(from.path(), new_file_info->fileUrl().path()))) {
                // 不用再进行后面的操作
                return true;
        }

        // 禁止目录复制/移动到自己里面
        if (new_file_info->isAncestorsUrl(source_info->fileUrl())) {
            setError(DFileCopyMoveJob::TargetIsSelfError);

            DFileCopyMoveJob::Action action = handleError(source_info.constData(), new_file_info.constData());

            if (action == DFileCopyMoveJob::SkipAction) {
                return true;
            }

            if (action != DFileCopyMoveJob::EnforceAction) {
                return false;
            }
        }

        bool source_is_file = source_info->isFile() || source_info->isSymLink();
        bool target_is_file = new_file_info->isFile() || new_file_info->isSymLink();

        if (target_is_file) {
            setError(DFileCopyMoveJob::FileExistsError);
        } else {
            setError(DFileCopyMoveJob::DirectoryExistsError);
        }

        switch (handleError(source_info.constData(), new_file_info.constData())) {
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
            return true;
        case DFileCopyMoveJob::CoexistAction:
            file_name = handle ? handle->getNonExistsFileName(q_ptr, source_info, target_info)
                        : getNewFileName(source_info, target_info);
            goto create_new_file_info;
        default:
            return false;
        }
    }

    if (source_info->isSymLink()) {
        bool ok = false;

        if (mode == DFileCopyMoveJob::CopyMode) {
            DAbstractFileInfoPointer new_source_info;
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
            if (new_source_info) {
                ok = linkFile(handler, new_file_info, new_source_info->symlinkTargetPath());
            }
            else {
                ok = linkFile(handler, new_file_info, source_info->symlinkTargetPath());
            }
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
            setError(DFileCopyMoveJob::NotEnoughSpaceError);
            DFileCopyMoveJob::Action action = handleError(source_info.constData(), new_file_info.constData());

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
            setError(DFileCopyMoveJob::FileSizeTooBigError);
            DFileCopyMoveJob::Action action = handleError(source_info.constData(), new_file_info.constData());

            if (action == DFileCopyMoveJob::SkipAction) {
                return true;
            }

            if (action != DFileCopyMoveJob::EnforceAction) {
                return false;
            }
        }

        if (mode == DFileCopyMoveJob::CopyMode) {
            //copyrefineflag为0,遍历线程才会继续去遍历
            if (copyrefineflag >= 1) {
                return false;
            }
            if (new_file_info->isSymLink() || fileHints.testFlag(DFileCopyMoveJob::RemoveDestination)) {
                if (!removeFile(handler, new_file_info)) {
                    return false;
                }
            } else if (new_file_info->exists()) {
                // 复制文件时，如果需要覆盖，必须添加可写入权限
                handler->setPermissions(new_file_info->fileUrl(), QFileDevice::WriteUser | QFileDevice::ReadUser);
            }
            FileCopyInfoPointer info(new FileCopyInfo);
            info->frominfo = source_info;
            info->toinfo = new_file_info;
            info->handler = handler;
            if (refinestat >= DFileCopyMoveJob::MoreThreadAndMainAndOpenRefine) {
                openfromfilequeue.enqueue(info);
            }
            else {
                readfileinfoqueue.enqueue(info);
            }
            ok = true;
        } else {
            ok = renameFile(handler, source_info, new_file_info);
        }

        if (ok) {
            joinToCompletedFileList(from, new_file_info->fileUrl(), size);
        }

        return ok;
    } else if (source_info->isDir()) {
        // 禁止目录复制/移动到自己里面
        if (new_file_info->isAncestorsUrl(source_info->fileUrl())) {
            setError(DFileCopyMoveJob::TargetIsSelfError);

            DFileCopyMoveJob::Action action = handleError(source_info.constData(), new_file_info.constData());

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
            ok = mergeDirectoryRefine(handler, source_info, new_file_info);
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

bool DFileCopyMoveJobPrivate::copyFileRefine(const DFileCopyMoveJobPrivate::FileCopyInfoPointer &copyinfo)
{
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch();
//    qDebug() << "read and open copyFileRefine  " << copyinfo->frominfo->fileUrl();
    beginJob(JobInfo::Copy, copyinfo->frominfo->fileUrl(), copyinfo->toinfo->fileUrl(),true);
    //预先读取
    {
        int fromfd = open(copyinfo->frominfo->fileUrl().path().toUtf8().toStdString().data(),O_RDONLY);
        if (-1 != fromfd) {
            readahead(fromfd,0,static_cast<size_t>(copyinfo->frominfo->size()));
            close(fromfd);
        }
    }
    bool ok = doCopyFileRefine(copyinfo);
    endJob(true);
    m_tatol += QDateTime::currentMSecsSinceEpoch() - elapsed;

    return ok;
}

bool DFileCopyMoveJobPrivate::doCopyFileRefine(const FileCopyInfoPointer &copyinfo)
{
    qint64 curt = QDateTime::currentMSecsSinceEpoch();
    QSharedPointer<DFileDevice> fromDevice(DFileService::instance()->createFileDevice(nullptr, copyinfo->frominfo->fileUrl()));
    if (!fromDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");

        return false;
    }
    QSharedPointer<DFileDevice> toDevice(DFileService::instance()->createFileDevice(nullptr, copyinfo->toinfo->fileUrl()));

    if (!toDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");

        return false;
    }
    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

    do {
        // 如果打开文件在保险箱内
        QString strPath = copyinfo->toinfo->fileUrl().toString();
        if (VaultController::isVaultFile(strPath)) {
            QString strFileName = strPath.section("/", -1, -1);
            if (strFileName.toUtf8().length() > 255) {
                qCDebug(fileJob()) << "open error:" << copyinfo->frominfo->fileUrl();
                setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: File name too long"));
                action = handleError(copyinfo->frominfo.constData(), nullptr);
                break;
            }
        }

        if (fromDevice->open(QIODevice::ReadOnly)) {
            action = DFileCopyMoveJob::NoAction;
            filerefinefd++;
        } else {
            qCDebug(fileJob()) << "open error:" << copyinfo->frominfo->fileUrl();

            if (copyinfo->frominfo->isReadable()) {
                setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(fromDevice->errorString()));
            } else {
                setError(DFileCopyMoveJob::PermissionError);
            }

            action = handleError(copyinfo->frominfo.constData(), nullptr);
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() ); // bug: 26333, while set the stop status shoule break the process!

    if (action == DFileCopyMoveJob::SkipAction) {
        return true;
    } else if (action != DFileCopyMoveJob::NoAction) {
        return false;
    }
#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (fromDevice->handle() > 0) {
        posix_fadvise (fromDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif
    qint64 size_block = fromDevice->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : fromDevice->size();
    while (true) {
        qint64 current_pos = fromDevice->pos();
        copyinfo->currentpos = current_pos;
        char *buffer = new char[size_block + 1];

        if (Q_UNLIKELY(!stateCheck())) {
//            qDebug() << "read and open state change  " << copyinfo->frominfo->fileUrl();
            delete[]  buffer;
            closefromdevicequeue.enqueue(fromDevice);
            return false;
        }
        curt = QDateTime::currentMSecsSinceEpoch();
        qint64 size_read = fromDevice->read(buffer, size_block);
        m_readtime += QDateTime::currentMSecsSinceEpoch() - curt;
//        qDebug() << "read and open time " << m_readtime;

        if (Q_UNLIKELY(size_read <= 0)) {
            if (size_read == 0 && fromDevice->atEnd()) {
                break;
            }

            const_cast<DAbstractFileInfo *>(copyinfo->frominfo.data())->refresh();

            if (copyinfo->frominfo->exists()) {
                setError(DFileCopyMoveJob::ReadError, qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: %1").arg(fromDevice->errorString()));
            } else {
                setError(DFileCopyMoveJob::NonexistenceError);
            }

            switch (handleError(copyinfo->frominfo.data(), copyinfo->toinfo.data())) {
            case DFileCopyMoveJob::RetryAction: {
                if (!fromDevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, fromDevice->errorString());
                    closefromdevicequeue.enqueue(fromDevice);
                    return false;
                }
                break;
            }
            case DFileCopyMoveJob::SkipAction:
                closefromdevicequeue.enqueue(fromDevice);
                return true;
            default:
                closefromdevicequeue.enqueue(fromDevice);
                return false;
            }
        }
        else {
            if (size_read >= MAX_BUFFER_LEN && size_read < fromDevice->size()) {
                FileCopyInfoPointer tmpinfo(new FileCopyInfo);
                tmpinfo->closeflag = false;
                tmpinfo->frominfo = copyinfo->frominfo;
                tmpinfo->toinfo = copyinfo->toinfo;
                tmpinfo->todevice = toDevice;
                tmpinfo->currentpos = current_pos;
                tmpinfo->buffer = buffer;
                tmpinfo->size = size_read;
                writefilequeue.enqueue(tmpinfo);
//                qDebug() << "insert write queue";
            }
            else {
                copyinfo->buffer = buffer;
                copyinfo->size = size_read;
                if (size_read >= fromDevice->size()) {
                    break;
                }
            }
        }
    }
    copyinfo->todevice = toDevice;
    writefilequeue.enqueue(copyinfo);
    closefromdevicequeue.enqueue(fromDevice);
    return true;
}

bool DFileCopyMoveJobPrivate::doCopyFileRefineReadAndWrite(const DFileCopyMoveJobPrivate::FileCopyInfoPointer &copyinfo)
{
//    // fix:bug42483 从文管中(数据盘)的桌面目录拖动回收站、桌面等图标至桌面，选择替换后图标变为文件状态无法使用
//    if (DStorageInfo::isSameFile(copyinfo->frominfo->fileUrl().path(), copyinfo->toinfo->fileUrl().path())) {
//        qDebug()<<"Info:"<<copyinfo->frominfo->fileUrl().path()<<" and "<<copyinfo->toinfo->fileUrl().path()<<" are the same file.";
//        return true;
//    }

    qint64 curt = QDateTime::currentMSecsSinceEpoch();
    QScopedPointer<DFileDevice> fromDevice(DFileService::instance()->createFileDevice(nullptr, copyinfo->frominfo->fileUrl()));
    if (!fromDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");

        return false;
    }
    QScopedPointer<DFileDevice> toDevice(DFileService::instance()->createFileDevice(nullptr, copyinfo->toinfo->fileUrl()));

    if (!toDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");

        return false;
    }
open_file: {
        DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

        do {
            // 如果打开文件在保险箱内
            QString strPath = copyinfo->toinfo->fileUrl().toString();
            if (VaultController::isVaultFile(strPath)) {
                QString strFileName = strPath.section("/", -1, -1);
                if (strFileName.toUtf8().length() > 255) {
                    qCDebug(fileJob()) << "open error:" << copyinfo->frominfo->fileUrl();
                    setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: File name too long"));
                    action = handleError(copyinfo->frominfo.constData(), nullptr);
                    break;
                }
            }

            if (fromDevice->open(QIODevice::ReadOnly)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << copyinfo->frominfo->fileUrl();

                if (copyinfo->frominfo->isReadable()) {
                    setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(fromDevice->errorString()));
                } else {
                    setError(DFileCopyMoveJob::PermissionError);
                }

                action = handleError(copyinfo->frominfo.constData(), nullptr);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() ); // bug: 26333, while set the stop status shoule break the process!

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        do {
            if (toDevice->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                action = DFileCopyMoveJob::NoAction;
            } else {
                qCDebug(fileJob()) << "open error:" << copyinfo->toinfo->fileUrl();

                if (!copyinfo->toinfo->exists() || copyinfo->toinfo->isWritable()) {
                    setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(toDevice->errorString()));
                } else {
                    setError(DFileCopyMoveJob::PermissionError);
                }

                action = handleError(copyinfo->toinfo.data(), nullptr);
            }
        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        } else if (action != DFileCopyMoveJob::NoAction) {
            return false;
        }

        if (fileHints.testFlag(DFileCopyMoveJob::ResizeDestinationFile)) {
            do {
                if (toDevice->resize(fromDevice->size())) {
                    action = DFileCopyMoveJob::NoAction;
                } else {
                    setError(DFileCopyMoveJob::ResizeError, toDevice->errorString());
                    action = handleError(copyinfo->toinfo.data(), nullptr);
                }
            } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

            if (action == DFileCopyMoveJob::SkipAction) {
                return true;
            } else if (action != DFileCopyMoveJob::NoAction) {
                return false;
            }
        }
    }

#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (fromDevice->handle() > 0) {
        posix_fadvise (fromDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }

    if (toDevice->handle() > 0) {
        posix_fadvise (toDevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif

    currentJobDataSizeInfo.first = fromDevice->size();
    currentJobFileHandle = toDevice->handle();
    uLong source_checksum = adler32(0L, nullptr, 0);
    qint64 size_block = copyinfo->frominfo->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN: copyinfo->frominfo->size();
    char *buffer = new char[size_block + 1];

    Q_FOREVER {
        qint64 current_pos = fromDevice->pos();
read_data:
        if (Q_UNLIKELY(!stateCheck())) {
            delete[] buffer;
            return false;
        }
        curt = QDateTime::currentMSecsSinceEpoch();
        qint64 size_read = fromDevice->read(buffer, size_block);
        m_readtime += QDateTime::currentMSecsSinceEpoch() - curt;

        if (Q_UNLIKELY(size_read <= 0)) {
            if (size_read == 0 && fromDevice->atEnd()) {
                break;
            }

            const_cast<DAbstractFileInfo *>(copyinfo->frominfo.data())->refresh();

            if (copyinfo->frominfo->exists()) {
                setError(DFileCopyMoveJob::ReadError, qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: %1").arg(fromDevice->errorString()));
            } else {
                setError(DFileCopyMoveJob::NonexistenceError);
            }

            switch (handleError(copyinfo->frominfo.data(), copyinfo->toinfo.data())) {
            case DFileCopyMoveJob::RetryAction: {
                if (!fromDevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, fromDevice->errorString());
                    delete[] buffer;
                    return false;
                }

                goto read_data;
            }
            case DFileCopyMoveJob::SkipAction:
                delete[] buffer;
                return true;
            default:
                delete[] buffer;
                return false;
            }
        }

        current_pos = toDevice->pos();
write_data:
        if (Q_UNLIKELY(!stateCheck())) {
            delete[] buffer;
            return false;
        }
//        qint64 size_write = toDevice->write(data, size_read);
        curt = QDateTime::currentMSecsSinceEpoch();
        qint64 size_write = toDevice->write(buffer, size_read);
        m_write += QDateTime::currentMSecsSinceEpoch() - curt;
        //如果写失败了，直接推出
        if (size_write < 0) {
            if (!stateCheck()) {
                //临时处理 fix
                //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                if (FileUtils::isGvfsMountFile(copyinfo->toinfo->path())) {
                    toDevice->closeWriteReadFailed(true);
                }
                delete[] buffer;
                return false;
            }
           setError(DFileCopyMoveJob::WriteError, qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString()));
           switch (handleError(copyinfo->frominfo.data(), copyinfo->toinfo.data())) {
           case DFileCopyMoveJob::RetryAction: {
               if (!toDevice->seek(current_pos)) {
                   setError(DFileCopyMoveJob::UnknowError, toDevice->errorString());
                   //临时处理 fix
                   //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                   //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                   if (FileUtils::isGvfsMountFile(copyinfo->toinfo->path())) {
                       toDevice->closeWriteReadFailed(true);
                   }
                   delete[] buffer;
                   return false;
               }

                goto write_data;
            }
            case DFileCopyMoveJob::SkipAction:
                //临时处理 fix
                //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                if (FileUtils::isGvfsMountFile(copyinfo->toinfo->path())) {
                    toDevice->closeWriteReadFailed(true);
                }
                delete[] buffer;
                return true;
            default:
                //临时处理 fix
                //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                if (FileUtils::isGvfsMountFile(copyinfo->toinfo->path())) {
                    toDevice->closeWriteReadFailed(true);
                }
                delete[] buffer;
                return false;
            }
        }

        if (Q_UNLIKELY(size_write != size_read)) {
            do {
                // 在某些情况下（往sftp挂载目录写入），可能一次未能写入那么多数据
                // 但不代表写入失败，应该继续尝试，直到所有数据全部写入
                if (size_write > 0) {
                    const char *surplus_data = buffer;
                    qint64 surplus_size = size_read;

                    do {
                        currentJobDataSizeInfo.second += size_write;
                        completedDataSize += size_write;
                        //        writtenDataSize += size_write;

                        surplus_data += size_write;
                        surplus_size -= size_write;

                        size_write = toDevice->write(surplus_data, surplus_size);
                    } while (size_write > 0 && size_write != surplus_size);

                    // 表示全部数据写入完成
                    if (size_write > 0) {
                        break;
                    }
                }

                if (checkFreeSpace(currentJobDataSizeInfo.first - currentJobDataSizeInfo.second)) {
                    setError(DFileCopyMoveJob::WriteError, qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString()));
                } else {
                    setError(DFileCopyMoveJob::NotEnoughSpaceError);
                }

                switch (handleError(copyinfo->frominfo.data(), copyinfo->toinfo.data())) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!toDevice->seek(current_pos)) {
                        setError(DFileCopyMoveJob::UnknowError, toDevice->errorString());
                        delete[] buffer;
                        return false;
                    }

                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    delete[] buffer;
                    return true;
                default:
                    delete[] buffer;
                    return false;
                }
            } while (false);

        }

        currentJobDataSizeInfo.second += size_write;
        completedDataSize += size_write;
        completedDataSizeOnBlockDevice += size_write;
        countrefinesize(size_write);

        if (Q_LIKELY(!fileHints.testFlag(DFileCopyMoveJob::DontIntegrityChecking))) {
            source_checksum = adler32(source_checksum, reinterpret_cast<Bytef *>(buffer), static_cast<uInt>(size_read));
        }
    }
    delete[] buffer;
    DFileHandler *handler = copyinfo->handler ? copyinfo->handler :
                                            DFileService::instance()->createFileHandler(nullptr, copyinfo->frominfo->fileUrl());
    handler->setFileTime(copyinfo->frominfo->fileUrl(), copyinfo->frominfo->lastRead(), copyinfo->frominfo->lastModified());

    QFileDevice::Permissions permissions = copyinfo->frominfo->permissions();
    //! use stat function to read vault file permission.
    QString path = copyinfo->frominfo->fileUrl().path();
    if (VaultController::isVaultFile(path)) {
        permissions = VaultController::getPermissions(path);
    }

    handler->setPermissions(copyinfo->toinfo->fileUrl(), /*source_info->permissions()*/permissions);

    // 关闭文件时可能会需要很长时间，因为内核可能要把内存里的脏数据回写到硬盘
    fromDevice->close();
    toDevice->close();
    countrefinesize(copyinfo->frominfo->size() <= 0 ? 4096 : 0);

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
            setError(DFileCopyMoveJob::OpenError, "Unable to open file for integrity check, , cause: " + toDevice->errorString());
            action = handleError(copyinfo->toinfo.data(), nullptr);
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
    size_block = 1024 * 1024;
    char *data = new char[size_block + 1];
    ulong target_checksum = adler32(0L, nullptr, 0);

    qint64 elapsed_time_checksum = 0;

    if (fileJob().isDebugEnabled()) {
        elapsed_time_checksum = updateSpeedElapsedTimer->elapsed();
    }

    Q_FOREVER {
        qint64 size = toDevice->read(data, size_block);

        if (Q_UNLIKELY(size <= 0)) {
            if (toDevice->atEnd()) {
                break;
            }

            setError(DFileCopyMoveJob::IntegrityCheckingError, qApp->translate("DFileCopyMoveJob", "File integrity was damaged, cause: %1").arg(toDevice->errorString()));

            switch (handleError(copyinfo->frominfo.constData(), copyinfo->toinfo.constData())) {
            case DFileCopyMoveJob::RetryAction: {
                continue;
            }
            case DFileCopyMoveJob::SkipAction:
                delete[] data;
                return true;
            default:
                delete[] data;
                return false;
            }
        }

        target_checksum = adler32(target_checksum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(size));

        if (Q_UNLIKELY(!stateCheck())) {
            delete[] data;
            return false;
        }
    }
    delete[] data;
//    qCDebug(fileJob(), "Time spent of integrity check of the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed_time_checksum);

    if (source_checksum != target_checksum) {
        qCWarning(fileJob(), "Failed on file integrity checking, source file: 0x%lx, target file: 0x%lx", source_checksum, target_checksum);

        setError(DFileCopyMoveJob::IntegrityCheckingError);
        DFileCopyMoveJob::Action action = handleError(copyinfo->frominfo.constData(), copyinfo->toinfo.constData());

        if (action == DFileCopyMoveJob::SkipAction) {
            return true;
        }

        if (action == DFileCopyMoveJob::RetryAction) {
            goto open_file;
        }

        return false;
    }

    return true;
}

bool DFileCopyMoveJobPrivate::openRefineThread()
{
    qint64 copyresulttime = QDateTime::currentMSecsSinceEpoch();
    bool ok = false;
    while (checkRefineCopyProccessSate(DFileCopyMoveJob::MainProccessOver)) {
        if(!openfromfilequeue.isEmpty()) {
            auto copyinfo = openfromfilequeue.dequeue();
            if (copyinfo->isdir) {
                readfileinfoqueue.enqueue(copyinfo);
            }
            else {
                while (filerefinefd > 600) {
                    usleep(10);
                }
                ok = openRefine(copyinfo);
                //打开文件失败，退出当前线程，清理所有的open队列（是文件夹的继续，到后面去加权），继续处理read、write和close队列，
                if (!ok) {
                    while(!openfromfilequeue.isEmpty()) {
                        auto copyinfo = openfromfilequeue.dequeue();
                        if (copyinfo->isdir) {
                            readfileinfoqueue.enqueue(copyinfo);
                        }
                    }
                    setRefineCopyProccessSate(DFileCopyMoveJob::OpenFromFileProccessOver);
                    return false;
                }
            }
        }
    }
    while(!openfromfilequeue.isEmpty()) {
        auto copyinfo = openfromfilequeue.dequeue();
        if (copyinfo->isdir) {
            readfileinfoqueue.enqueue(copyinfo);
        }
        else {
            while (filerefinefd > 600) {
                usleep(10);
            }
            ok = openRefine(copyinfo);
            //打开文件失败，退出当前线程，清理所有的open队列，继续处理read、write和close队列，
            if (!ok) {
                while(!openfromfilequeue.isEmpty()) {
                    auto copyinfo = openfromfilequeue.dequeue();
                    if (copyinfo->isdir) {
                        readfileinfoqueue.enqueue(copyinfo);
                    }
                }
                setRefineCopyProccessSate(DFileCopyMoveJob::OpenFromFileProccessOver);
                return false;
            }
        }
    }
    setRefineCopyProccessSate(DFileCopyMoveJob::OpenFromFileProccessOver);

    qDebug() << "open file over ==== " << copyresulttime - m_sart << QDateTime::currentMSecsSinceEpoch() - copyresulttime;
    return true;
}

bool DFileCopyMoveJobPrivate::openRefine(const DFileCopyMoveJobPrivate::FileCopyInfoPointer &copyinfo)
{
    beginJob(JobInfo::Copy, copyinfo->frominfo->fileUrl(), copyinfo->toinfo->fileUrl(),true);

    QSharedPointer<DFileDevice> fromDevice(DFileService::instance()->createFileDevice(nullptr, copyinfo->frominfo->fileUrl()));
    if (!fromDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");

        return false;
    }
    QSharedPointer<DFileDevice> toDevice(DFileService::instance()->createFileDevice(nullptr, copyinfo->toinfo->fileUrl()));

    if (!toDevice) {
        setError(DFileCopyMoveJob::UnknowUrlError, "Failed on create file device");

        return false;
    }

    DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;

    do {
        // 如果打开文件在保险箱内
        QString strPath = copyinfo->toinfo->fileUrl().toString();
        if (VaultController::isVaultFile(strPath)) {
            QString strFileName = strPath.section("/", -1, -1);
            if (strFileName.toUtf8().length() > 255) {
                qCDebug(fileJob()) << "open error:" << copyinfo->frominfo->fileUrl();
                setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: File name too long"));
                action = handleError(copyinfo->frominfo.constData(), nullptr);
                break;
            }
        }

        if (fromDevice->open(QIODevice::ReadOnly)) {
            action = DFileCopyMoveJob::NoAction;
            filerefinefd++;
        } else {
            qCDebug(fileJob()) << "open error:" << copyinfo->frominfo->fileUrl();

            if (copyinfo->frominfo->isReadable()) {
                setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(fromDevice->errorString()));
            } else {
                setError(DFileCopyMoveJob::PermissionError);
            }

            action = handleError(copyinfo->frominfo.constData(), nullptr);
        }
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() ); // bug: 26333, while set the stop status shoule break the process!

    if (action == DFileCopyMoveJob::SkipAction) {
        return true;
    } else if (action != DFileCopyMoveJob::NoAction) {
        return false;
    }
    copyinfo->fromdevice = fromDevice;
    copyinfo->todevice = toDevice;
    endJob(true);
    readfileinfoqueue.enqueue(copyinfo);

    return true;
}

bool DFileCopyMoveJobPrivate::readRefineThread()
{

    while (checkRefineCopyProccessSate(DFileCopyMoveJob::OpenFromFileProccessOver)) {
        if (!readfileinfoqueue.isEmpty()) {
            auto copyinfo = readfileinfoqueue.dequeue();
            if (copyinfo->isdir) {
                if (refinestat == DFileCopyMoveJob::MoreThreadAndMainRefine) {
                    addfilepermissionsqueue.enqueue(copyinfo);
                    continue;
                }
                writefilequeue.enqueue(copyinfo);
            }
            else{
                bool ok = false;
                if (refinestat == DFileCopyMoveJob::MoreThreadAndMainAndOpenRefine) {
                    ok = readRefine(copyinfo);
                }
                else {
                    ok = copyFileRefine(copyinfo);
                }
                if (!ok) {
                    //打开文件失败，退出当前线程，清理所有的open队列，继续处理read、write和close队列，
                    while(!readfileinfoqueue.isEmpty()) {
                        auto copyinfo = readfileinfoqueue.dequeue();
                        if (copyinfo->isdir) {
                            writefilequeue.enqueue(copyinfo);
                        }
                    }
                    setRefineCopyProccessSate(DFileCopyMoveJob::ReadFileProccessOver);
                    return false;
                }
            }
        }
    }
    while (!readfileinfoqueue.isEmpty()) {
        auto copyinfo = readfileinfoqueue.dequeue();
        if (copyinfo->isdir) {
            if (refinestat == DFileCopyMoveJob::MoreThreadAndMainRefine) {
                addfilepermissionsqueue.enqueue(copyinfo);
                continue;
            }
            //对文件夹加权
            writefilequeue.enqueue(copyinfo);
        }
        else{
            bool ok = false;
            if (refinestat == DFileCopyMoveJob::MoreThreadAndMainAndOpenRefine) {
                ok = readRefine(copyinfo);
            }
            else {
                ok = copyFileRefine(copyinfo);
            }
            if (!ok) {
                //打开文件失败，退出当前线程，清理所有的open队列，继续处理read、write和close队列，
                while(!readfileinfoqueue.isEmpty()) {
                    auto copyinfo = readfileinfoqueue.dequeue();
                    if (copyinfo->isdir) {
                        writefilequeue.enqueue(copyinfo);
                    }
                }
                setRefineCopyProccessSate(DFileCopyMoveJob::ReadFileProccessOver);
                return false;
            }
        }
    }
    setRefineCopyProccessSate(DFileCopyMoveJob::ReadFileProccessOver);
    qDebug() << "read file thread over ==== " << QDateTime::currentMSecsSinceEpoch() - m_sart;
    return true;
}


bool DFileCopyMoveJobPrivate::readRefine(const DFileCopyMoveJobPrivate::FileCopyInfoPointer &copyinfo)
{
#ifdef Q_OS_LINUX
    // 开启读取优化，告诉内核，我们将顺序读取此文件

    if (copyinfo->fromdevice->handle() > 0) {
        posix_fadvise (copyinfo->fromdevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
    }
#endif
    qint64 size_block = copyinfo->fromdevice->size() > MAX_BUFFER_LEN ? MAX_BUFFER_LEN : copyinfo->fromdevice->size();

    while (true) {
        qint64 current_pos = copyinfo->fromdevice->pos();
        copyinfo->currentpos = current_pos;
        char *buffer = new char[size_block + 1];

        if (Q_UNLIKELY(!stateCheck())) {
            return false;
        }
        qint64 curt = QDateTime::currentMSecsSinceEpoch();
        qint64 size_read = copyinfo->fromdevice->read(buffer, size_block);
         m_readtime += QDateTime::currentMSecsSinceEpoch() - curt;

        if (Q_UNLIKELY(size_read <= 0)) {
            if (size_read == 0 && copyinfo->fromdevice->atEnd()) {
                break;
            }

            const_cast<DAbstractFileInfo *>(copyinfo->frominfo.data())->refresh();

            if (copyinfo->frominfo->exists()) {
                setError(DFileCopyMoveJob::ReadError, qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: %1").arg(copyinfo->fromdevice->errorString()));
            } else {
                setError(DFileCopyMoveJob::NonexistenceError);
            }

            switch (handleError(copyinfo->frominfo.data(), copyinfo->toinfo.data())) {
            case DFileCopyMoveJob::RetryAction: {
                if (!copyinfo->fromdevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, copyinfo->fromdevice->errorString());

                    return false;
                }
                break;
            }
            case DFileCopyMoveJob::SkipAction:
                return true;
            default:
                return false;
            }
        }
        else {
            if (size_read >= MAX_BUFFER_LEN && size_read < copyinfo->fromdevice->size()) {
                FileCopyInfoPointer tmpinfo(new FileCopyInfo(*copyinfo));
                tmpinfo->closeflag = false;
                tmpinfo->buffer = buffer;
                tmpinfo->size = size_read;
                tmpinfo->currentpos = current_pos;
                writefilequeue.enqueue(tmpinfo);
            }
            else {
                copyinfo->buffer = buffer;
                copyinfo->size = size_read;
                if (size_read >= copyinfo->fromdevice->size()) {
                    break;
                }
            }
        }
    }
    writefilequeue.enqueue(copyinfo);
    closefromdevicequeue.enqueue(copyinfo->fromdevice);

    return true;
}

bool DFileCopyMoveJobPrivate::copyReadAndWriteRefineThread()
{
    qint64 copyresulttime = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "start _ copyReadAndWriteRefineThread  =               == " << copyresulttime - m_sart;

    while (checkRefineCopyProccessSate(DFileCopyMoveJob::OpenFromFileProccessOver)) {
        if (!readfileinfoqueue.isEmpty()) {
            auto copyinfo = readfileinfoqueue.dequeue();
            if (copyinfo->isdir) {
                addfilepermissionsqueue.enqueue(copyinfo);
            }
            else
            {
                //判断目标目录是u盘就只走但线程
                if (Q_UNLIKELY(!stateCheck())) {
                    //打开文件失败，退出当前线程，清理所有的open队列，继续处理read、write和close队列，
                    while(!readfileinfoqueue.isEmpty()) {
                        auto copyinfo = readfileinfoqueue.dequeue();
                        if (copyinfo->isdir) {
                            addfilepermissionsqueue.enqueue(copyinfo);
                        }
                    }
                    setRefineCopyProccessSate(isreadwriteseparate ? DFileCopyMoveJob::ReadFileProccessOver :
                                                                    DFileCopyMoveJob::ReadAndWriteFileProccessOver);
                    return false;
                }
                if (!bdestLocal || isreadwriteseparate) {
                    copyReadAndWriteRefineRefine(copyinfo);
                    continue;
                }

                while(m_pool.activeThreadCount() >= 24)
                {

                }
                QtConcurrent::run(&m_pool, this, static_cast<bool(DFileCopyMoveJobPrivate::*)
                                 (const DFileCopyMoveJobPrivate::FileCopyInfoPointer &)>
                                 (&DFileCopyMoveJobPrivate::copyReadAndWriteRefineRefine), copyinfo);
            }
        }
    }
    qDebug() << "copyReadAndWriteRefineThread stat change  " << QDateTime::currentMSecsSinceEpoch() - copyresulttime;
    while (!readfileinfoqueue.isEmpty()) {
        auto copyinfo = readfileinfoqueue.dequeue();
        if (copyinfo->isdir) {
            addfilepermissionsqueue.enqueue(copyinfo);
        }
        else
        {
            //判断目标目录是u盘就只走但线程
            if (Q_UNLIKELY(!stateCheck())) {
                //打开文件失败，退出当前线程，清理所有的open队列，继续处理read、write和close队列，
                while(!readfileinfoqueue.isEmpty()) {
                    auto copyinfo = readfileinfoqueue.dequeue();
                    if (copyinfo->isdir) {
                        addfilepermissionsqueue.enqueue(copyinfo);
                    }
                }
                setRefineCopyProccessSate(isreadwriteseparate ? DFileCopyMoveJob::ReadFileProccessOver :
                                                                DFileCopyMoveJob::ReadAndWriteFileProccessOver);
                return false;
            }
            if (!bdestLocal || isreadwriteseparate) {
                copyReadAndWriteRefineRefine(copyinfo);
                continue;
            }
            while(m_pool.activeThreadCount() >= 24)
            {

            }
            QtConcurrent::run(&m_pool, this, static_cast<bool(DFileCopyMoveJobPrivate::*)
                             (const DFileCopyMoveJobPrivate::FileCopyInfoPointer &)>
                             (&DFileCopyMoveJobPrivate::copyReadAndWriteRefineRefine), copyinfo);
        }
    }
    setRefineCopyProccessSate(isreadwriteseparate ? DFileCopyMoveJob::ReadFileProccessOver :
                                                    DFileCopyMoveJob::ReadAndWriteFileProccessOver);
    qDebug() << "copyReadAndWriteRefineThread over ==== " << QDateTime::currentMSecsSinceEpoch() - m_sart;
    return true;
}

bool DFileCopyMoveJobPrivate::copyReadAndWriteRefineRefine(const DFileCopyMoveJobPrivate::FileCopyInfoPointer &copyinfo)
{
    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }
    FileCopyInfoPointer copytemp = copyinfo;
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch();
    beginJob(JobInfo::Copy, copyinfo->frominfo->fileUrl(), copyinfo->toinfo->fileUrl(),true);
    //大于1G并且在盘内只有一个拷贝的和大于1G拷贝到u盘，执行

    bool ok = false;
    //预先读取
    {
        int fromfd = open(copyinfo->frominfo->fileUrl().path().toUtf8().toStdString().data(),O_RDONLY);
        if (-1 != fromfd) {
            readahead(fromfd,0,static_cast<size_t>(copyinfo->frominfo->size()));
            close(fromfd);
        }
    }
    if ((copyinfo->frominfo->size() >= BIG_FILE_SIZE) && (1 == totalfilecount || !bdestLocal)) {
        ok = doCopyFileRefine(copytemp);
    }
    else {
        ok = doCopyFileRefineReadAndWrite(copytemp);
    }
    endJob(true);
    m_tatol += QDateTime::currentMSecsSinceEpoch() - elapsed;

    return ok;
}

bool DFileCopyMoveJobPrivate::writeRefineThread()
{
    while (checkRefineCopyProccessSate(DFileCopyMoveJob::ReadFileProccessOver)) {
        writeRefine();
    }
    qDebug() << "write thread over      stat change   ========== " << QDateTime::currentMSecsSinceEpoch() - m_sart;
    writeRefine();
    setRefineCopyProccessSate(DFileCopyMoveJob::ReadAndWriteFileProccessOver);
    qDebug() << "write thread over         ========== " << QDateTime::currentMSecsSinceEpoch() - m_sart;
    return true;
}

bool DFileCopyMoveJobPrivate::writeRefine()
{
    while (!writefilequeue.isEmpty()) {

        while (filerefinefd > 600) {
            usleep(10);
        }
        auto info = writefilequeue.dequeue();
        if (Q_UNLIKELY(!stateCheck())) {
            qDebug() << "write stat change ";
            closetodevicesqueue.enqueue(info->todevice);
            while (!writefilequeue.isEmpty()) {
                auto info = writefilequeue.dequeue();
                delete[] info->buffer;
            }
            return false;
        }
        if (!info->todevice && info->isdir) {
//            qDebug() << "write stat change ";
            addfilepermissionsqueue.enqueue(info);
            continue;
        }
        if (!info->todevice){
//            qDebug() << "write stat change ";
            continue;
        }
#ifdef Q_OS_LINUX
        // 开启读取优化，告诉内核，我们将顺序读取此文件
        if (info->todevice && info->todevice->handle() > 0) {
            posix_fadvise (info->todevice->handle(), 0, 0, POSIX_FADV_SEQUENTIAL);
        }
#endif
        if (!info->todevice->isOpen()) {
            DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
            do {
                qDebug() << info->todevice->fileUrl();
                if (info->todevice->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    action = DFileCopyMoveJob::NoAction;
                    filerefinefd++;
                } else {
                    qCDebug(fileJob()) << "open error:" << info->toinfo->fileUrl();

                    if (!info->toinfo->exists() || info->toinfo->isWritable()) {
                        setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(info->todevice->errorString()));
                    } else {
                        setError(DFileCopyMoveJob::PermissionError);
                    }

                    action = handleError(info->toinfo.data(), nullptr);
                }
            } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

            if (action == DFileCopyMoveJob::SkipAction) {
                return true;
            } else if (action != DFileCopyMoveJob::NoAction) {
                return false;
            }

            if (fileHints.testFlag(DFileCopyMoveJob::ResizeDestinationFile)) {
                do {
                    if (info->todevice->resize(info->fromdevice->size())) {
                        action = DFileCopyMoveJob::NoAction;
                    } else {
                        setError(DFileCopyMoveJob::ResizeError, info->todevice->errorString());
                        action = handleError(info->toinfo.data(), nullptr);
                    }
                } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

                if (action == DFileCopyMoveJob::SkipAction) {
                    return true;
                } else if (action != DFileCopyMoveJob::NoAction) {
                    return false;
                }
            }
        }

        qint64 curt = QDateTime::currentMSecsSinceEpoch();
write_data:
        {
//            qDebug() << " write start   ===== " << curt - m_sart;
            qint64 size_write = info->todevice->write(info->buffer, info->size);
            m_write += QDateTime::currentMSecsSinceEpoch() - curt;
//            qDebug() << "write runing  " << m_write;
            //如果写失败了，直接推出
            if (size_write < 0) {
                if (!stateCheck()) {
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    if (FileUtils::isGvfsMountFile(info->toinfo->path())) {
                        info->todevice->closeWriteReadFailed(true);
                    }
                    delete[] info->buffer;
                    return false;
                }
               setError(DFileCopyMoveJob::WriteError, qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").
                        arg(info->todevice->errorString()));
               switch (handleError(info->frominfo.data(), info->toinfo.data())) {
               case DFileCopyMoveJob::RetryAction: {
                   if (!info->todevice->seek(info->currentpos)) {
                       setError(DFileCopyMoveJob::UnknowError, info->todevice->errorString());
                       //临时处理 fix
                       //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                       //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                       if (FileUtils::isGvfsMountFile(info->toinfo->path())) {
                           info->todevice->closeWriteReadFailed(true);
                       }
                       delete[] info->buffer;
                       return false;
                   }

                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    if (FileUtils::isGvfsMountFile(info->toinfo->path())) {
                        info->todevice->closeWriteReadFailed(true);
                    }
                    delete[] info->buffer;
                    return true;
                default:
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    if (FileUtils::isGvfsMountFile(info->toinfo->path())) {
                        info->todevice->closeWriteReadFailed(true);
                    }
                    delete[] info->buffer;
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

                    if (checkFreeSpace(currentJobDataSizeInfo.first - currentJobDataSizeInfo.second)) {
                        setError(DFileCopyMoveJob::WriteError, qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(info->todevice->errorString()));
                    } else {
                        setError(DFileCopyMoveJob::NotEnoughSpaceError);
                    }

                    switch (handleError(info->frominfo.data(), info->toinfo.data())) {
                    case DFileCopyMoveJob::RetryAction: {
                        if (!info->todevice->seek(info->currentpos)) {
                            setError(DFileCopyMoveJob::UnknowError, info->todevice->errorString());
                            delete[] info->buffer;
                            if (info->closeflag) {
                                addfilepermissionsqueue.enqueue(info);
                                closetodevicesqueue.enqueue(info->todevice);
                            }
                            return false;
                        }

                        goto write_data;
                    }
                    case DFileCopyMoveJob::SkipAction:
                        delete[] info->buffer;
                        if (info->closeflag) {
                            addfilepermissionsqueue.enqueue(info);
                            closetodevicesqueue.enqueue(info->todevice);
                        }
                        return true;
                    default:
                        delete[] info->buffer;
                        if (info->closeflag) {
                            addfilepermissionsqueue.enqueue(info);
                            closetodevicesqueue.enqueue(info->todevice);
                        }
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
        if (info->closeflag) {
            if (isreadwriteseparate) {
                //对文件加权
//                qDebug() << "isreadwriteseparate write addpermissions  ";
                DFileHandler *handler = info->handler ? info->handler :
                                                        DFileService::instance()->createFileHandler(nullptr, info->frominfo->fileUrl());
                handler->setFileTime(info->frominfo->fileUrl(), info->frominfo->lastRead(), info->frominfo->lastModified());
                QFileDevice::Permissions permissions = info->frominfo->permissions();
                //! use stat function to read vault file permission.
                QString path = info->frominfo->fileUrl().path();
                if (VaultController::isVaultFile(path)) {
                    permissions = VaultController::getPermissions(path);
                }
                handler->setPermissions(info->toinfo->fileUrl(), /*source_info->permissions()*/permissions);
            }
            else {
//                qDebug() << "write insert addfilepermissionsqueue  ";
                addfilepermissionsqueue.enqueue(info);
            }
            closetodevicesqueue.enqueue(info->todevice);
        }
//        qDebug() << "once write over ";
    }
    return true;
}

void DFileCopyMoveJobPrivate::addRefinePermissions()
{
    while (!addfilepermissionsqueue.isEmpty()) {
        auto info = addfilepermissionsqueue.dequeue();
        DFileHandler *handler = info->handler ? info->handler :
                                                DFileService::instance()->createFileHandler(nullptr, info->frominfo->fileUrl());
        if (info->isdir) {
            //对文件夹加权
            //处理文件权限问题
            QFileDevice::Permissions permissions;
            QString filePath = info->frominfo->fileUrl().toLocalFile();
            if (VaultController::ins()->isVaultFile(filePath)) {
                permissions = VaultController::ins()->getPermissions(filePath);
            } else {
                permissions = info->frominfo->permissions();
            }
            handler->setPermissions(info->toinfo->fileUrl(), permissions);
        }
        else {
            //对文件加权
            handler->setFileTime(info->frominfo->fileUrl(), info->frominfo->lastRead(), info->frominfo->lastModified());
            QFileDevice::Permissions permissions = info->frominfo->permissions();
            //! use stat function to read vault file permission.
            QString path = info->frominfo->fileUrl().path();
            if (VaultController::isVaultFile(path)) {
                permissions = VaultController::getPermissions(path);
            }
            handler->setPermissions(info->toinfo->fileUrl(), /*source_info->permissions()*/permissions);
        }
    }
}

void DFileCopyMoveJobPrivate::addRefinePermissionsThread()
{
    while (checkRefineCopyProccessSate(DFileCopyMoveJob::ReadAndWriteFileProccessOver)) {
        addRefinePermissions();
    }
    addRefinePermissions();
    qDebug() << " add file permission thread over " << QDateTime::currentMSecsSinceEpoch() - m_sart;
}

void DFileCopyMoveJobPrivate::closeRefineFromDeviceThread()
{
    while (checkRefineCopyProccessSate(DFileCopyMoveJob::ReadAndWriteFileProccessOver)) {
        if (!closefromdevicequeue.isEmpty()) {
            QSharedPointer<DFileDevice> device = closefromdevicequeue.dequeue();
            device->close();
            filerefinefd--;
        }
    }
    while (!closefromdevicequeue.isEmpty()) {
        QSharedPointer<DFileDevice> device = closefromdevicequeue.dequeue();
        device->close();
        filerefinefd--;
    }
    qDebug() << " close from file thread over = " << QDateTime::currentMSecsSinceEpoch() - m_sart;
}

void DFileCopyMoveJobPrivate::closeRefineToDeviceThread()
{
    while (checkRefineCopyProccessSate(DFileCopyMoveJob::ReadAndWriteFileProccessOver)) {
        if (!closetodevicesqueue.isEmpty()) {
            QSharedPointer<DFileDevice> device = closetodevicesqueue.dequeue();
            device->close();
            filerefinefd--;
        }
    }
    if (!closetodevicesqueue.isEmpty()) {
        QSharedPointer<DFileDevice> device = closetodevicesqueue.dequeue();
        device->close();
        filerefinefd--;
    }
    qDebug() << " close to file thread over = " << QDateTime::currentMSecsSinceEpoch() - m_sart;
}

void DFileCopyMoveJobPrivate::countAllCopyFile()
{
//    if (mode ==  DFileCopyMoveJob::CopyMode) {
        qint64 times = QDateTime::currentMSecsSinceEpoch();
        for (auto url : sourceUrlList) {
            char *paths[2] = {nullptr,nullptr};
            paths[0] = strdup(url.path().toUtf8().toStdString().data());
            FTS* fts = fts_open(paths, 0, nullptr);
            if (nullptr == fts) {
                perror("fts_open");
                continue;
            }
            while(1) {
                FTSENT* ent = fts_read(fts);
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
                if (BIG_FILE_SIZE <= ent->fts_statp->st_size) {
                    isbigfile = true;
                }
            }
            fts_close(fts);
//        }

        iscountsizeover = true;

        qDebug() << " dir time " << QDateTime::currentMSecsSinceEpoch() - times << totalsize;
    }

}

void DFileCopyMoveJobPrivate::runRefineThread()
{
    switch (refinestat) {
        case DFileCopyMoveJob::MoreThreadAndMainRefine:
        {
            //启动拷贝线程
            copyresult = QtConcurrent::run([this](){
                this->copyReadAndWriteRefineThread();
            });

            break;
        }
        case DFileCopyMoveJob::MoreThreadAndMainAndReadRefine:
        {
            //启用线程去读取文件
            copyresult = QtConcurrent::run([this](){
                this->readRefineThread();
            });
            //启动写线程
            runRefineWriteAndCloseThread();
            //加权线程
            this->addper = QtConcurrent::run([this](){
                this->addRefinePermissionsThread();
            });
            break;
        }
        case DFileCopyMoveJob::MoreThreadAndMainAndOpenRefine:
        {
            openfrom = QtConcurrent::run([this](){
                openRefineThread();
            });

            //启用线程去读取文件
            copyresult = QtConcurrent::run([this](){
                this->readRefineThread();
            });
            runRefineWriteAndCloseThread();
            //加权线程
            this->addper = QtConcurrent::run([this](){
                this->addRefinePermissionsThread();
            });
            break;
        }
    }
}

void DFileCopyMoveJobPrivate::runRefineWriteAndCloseThread()
{
    //启动写线程
    writeresult = QtConcurrent::run([this](){
        this->writeRefineThread();
    });
    this->closedevice = QtConcurrent::run([this](){
        this->closeRefineFromDeviceThread();
    });
    this->closefromresult = QtConcurrent::run([this](){
        this->closeRefineToDeviceThread();
    });
}

void DFileCopyMoveJobPrivate::setRefineCopyProccessSate(const DFileCopyMoveJob::RefineCopyProccessSate &stat)
{
    copyrefineflag = stat;
}

bool DFileCopyMoveJobPrivate::checkRefineCopyProccessSate(const DFileCopyMoveJob::RefineCopyProccessSate &stat)
{
    return copyrefineflag < stat;
}



DFileCopyMoveJob::DFileCopyMoveJob( QObject *parent)
    : DFileCopyMoveJob(*new DFileCopyMoveJobPrivate(this), parent)
{
}

DFileCopyMoveJob::~DFileCopyMoveJob()
{
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
    if (!d->iscountsizeover) {
        return -1;
    }
    return d->totalsize;

    if (d->mode == CopyMode) {
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

bool DFileCopyMoveJob::getSysncState()
{
    Q_D(DFileCopyMoveJob);

    return d->bsysncstate;
}

bool DFileCopyMoveJob::getSysncQuitState()
{
    Q_D(DFileCopyMoveJob);

    return d->bsysncquitstate;
}

void DFileCopyMoveJob::setSysncState(const bool &state)
{
    Q_D(DFileCopyMoveJob);

    d->bsysncstate = state;
}

void DFileCopyMoveJob::setSysncQuitState(const bool &quitstate)
{
    Q_D(DFileCopyMoveJob);

    d->bsysncquitstate = quitstate;
}

bool DFileCopyMoveJob::destIsLocal(const QString &rootpath)
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

void DFileCopyMoveJob::setRefine(const RefineState &refinestat)
{
    Q_D(DFileCopyMoveJob);

    d->refinestat = refinestat;
}

void DFileCopyMoveJob::waitSysncEnd()
{
    Q_D(DFileCopyMoveJob);

    if (d->refinestat == NoRefine) {
        return;
    }
    while (d->m_pool.activeThreadCount() || getSysncState())
    {
        if (StoppedState == d->state) {
            d->m_pool.clear();
            d->syncresult.cancel();
            break;
        }
        usleep(100);
    }
}

void DFileCopyMoveJob::waitRefineThreadOver()
{
    Q_D(DFileCopyMoveJob);
    while (d->m_pool.activeThreadCount() > 0 || !d->openfrom.isFinished()
           || !d->copyresult.isFinished() || !d->writeresult.isFinished()) {
        usleep(100);
    }
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
//    if (d->mode != CopyMode) {
//        if (d->fileStatistics->isRunning()) {
//            d->fileStatistics->stop();
//            d->fileStatistics->wait();
//        }
//        d->fileStatistics->start(sourceUrls);
//    }

    // DFileStatisticsJob 统计数量很慢，自行统计
    QtConcurrent::run([sourceUrls, d] () {
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
}

void DFileCopyMoveJob::togglePause()
{
    Q_D(DFileCopyMoveJob);

    if (d->state == StoppedState) {
        return;
    }

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

    d->btaskdailogclose = true;
}

DFileCopyMoveJob::DFileCopyMoveJob(DFileCopyMoveJobPrivate &dd, QObject *parent)
    : QThread(parent)
    , d_d_ptr(&dd)
{
    dd.fileStatistics = new DFileStatisticsJob(this);
    dd.updateSpeedTimer = new QTimer(this);

    connect(dd.fileStatistics, &DFileStatisticsJob::finished, this, &DFileCopyMoveJob::fileStatisticsFinished, Qt::DirectConnection);
    connect(dd.updateSpeedTimer, SIGNAL(timeout()), this, SLOT(_q_updateProgress()), Qt::DirectConnection);
}

void DFileCopyMoveJob::run()
{
    Q_D(DFileCopyMoveJob);

//    if (d->mode == CopyMode) {
//        if (d->fileHints.testFlag(CreateParents)) {

//        }
//    }

//    qCDebug(fileJob()) << "start job, mode:" << d->mode << "file url list:" << d->sourceUrlList << ", target url:" << d->targetUrl;
    qint64 timesec = QDateTime::currentMSecsSinceEpoch();
    d->m_sart = timesec;

    //启动遍历线程统计文件大小
    d->countAllCopyFile();

    //启动优化线程
    d->runRefineThread();

    d->unsetError();
    d->setState(RunningState);
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

        QScopedPointer<DStorageInfo> targetStorageInfo(DFileService::instance()->createStorageInfo(nullptr, d->targetUrl));

        if (targetStorageInfo) {
            d->targetRootPath = targetStorageInfo->rootPath();
            QString rootpath = d->targetRootPath;
            d->bdestLocal = destIsLocal(rootpath);
            d->isreadwriteseparate = (d->isbigfile && (!d->bdestLocal || 1 == d->totalfilecount));
            if (d->isreadwriteseparate && d->refinestat == MoreThreadAndMainRefine) {
                d->runRefineWriteAndCloseThread();
            }
            if (d->refinestat >= MoreThreadRefine && !d->bdestLocal) {

                //优化等待1秒后启动异步“同文件”
                QTimer::singleShot(100,this,[me,d,&rootpath](){
                    if (!me || !d) {
                        return;
                    }
                    d->syncresult = QtConcurrent::run([me,d,&rootpath]() {
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

                                    qCWarning(fileJob(), );
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
    setSysncQuitState(true);
    if (DFileCopyMoveJob::StoppedState == d->state) {
        d->syncresult.cancel();
    }
    //设置优化拷贝线程结束
    d->setRefineCopyProccessSate(d->refinestat >= MoreThreadAndMainAndOpenRefine ? MainProccessOver : OpenFromFileProccessOver);
    //等待线程池结束,等待异步tongbu线程结束
    qDebug() << "mian copy preceess over ===" << QDateTime::currentMSecsSinceEpoch() - timesec;
    waitRefineThreadOver();
    if (MoreThreadAndMainRefine == d->refinestat && DFileCopyMoveJob::StoppedState != d->state) {
        //加权线程
        d->addper = QtConcurrent::run([d](){
            d->addRefinePermissionsThread();
        });
    }
    waitRefineThreadOver();
    qDebug() << "all copy refine preceess over ===" << QDateTime::currentMSecsSinceEpoch() - timesec;
    waitSysncEnd();
    qDebug() << "sync preceess over ===" << QDateTime::currentMSecsSinceEpoch() - timesec;

    if (!d->bdestLocal && d->targetIsRemovable && mayExecSync &&
            d->state != DFileCopyMoveJob::StoppedState) { //主动取消时state已经被设置为stop了
        // 任务完成后执行 sync 同步数据到硬盘, 同时将状态改为 SleepState，用于定时器更新进度和速度信息
        d->setState(IOWaitState);
        int syncRet = 0;
        d->syncresult = QtConcurrent::run([me,&d, &syncRet]() {
            qDebug() << "sync >>>>>>>>>>>>>>";
            syncRet = QProcess::execute("sync", {"-f", d->targetRootPath});
        });
        // 检测同步时是否被停止，若停止则立即跳出
        while (!d->syncresult.isFinished()) {
            if (d->state == DFileCopyMoveJob::StoppedState) {
                qDebug() << "stop sync";
                goto end;
            }
            QThread::msleep(10);
        }

        // 同步结果检查只针对拷贝
        if (d->mode == CopyMode && syncRet != 0) {
            d->setError(DFileCopyMoveJob::OpenError, "Failed to synchronize to disk u!");
            DFileCopyMoveJob::Action action = d->handleError(target_info.constData(), nullptr);

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

QString DFileCopyMoveJob::Handle::getNonExistsFileName(DFileCopyMoveJob *job,  const DAbstractFileInfoPointer &sourceInfo, const DAbstractFileInfoPointer &targetDirectory)
{
    Q_UNUSED(job)

    return DFileCopyMoveJobPrivate::getNewFileName(sourceInfo, targetDirectory);
}

DFM_END_NAMESPACE

#include "moc_dfilecopymovejob.cpp"
