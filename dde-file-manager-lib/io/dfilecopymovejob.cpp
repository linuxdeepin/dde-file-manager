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
}

DFileCopyMoveJobPrivate::~DFileCopyMoveJobPrivate()
{
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

            qCDebug(fileJob(), "Did Write size on block device: %lld", size);

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
    } else if (s != DFileCopyMoveJob::IOWaitState) {
        updateSpeedElapsedTimer->togglePause();

        QMetaObject::invokeMethod(updateSpeedTimer, "stop");
    }

    Q_EMIT q->stateChanged(s);

    qCDebug(fileJob()) << "state changed, new state:" << s;
}

void DFileCopyMoveJobPrivate::setError(DFileCopyMoveJob::Error e, const QString &es)
{
    if (error == e) {
        return;
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
            unsetError();
            break;
        case DFileCopyMoveJob::FileExistsError:
        case DFileCopyMoveJob::DirectoryExistsError:
            lastErrorHandleAction = DFileCopyMoveJob::CoexistAction;
            unsetError();
            break;
        default:
            lastErrorHandleAction = DFileCopyMoveJob::CancelAction;
            setError(DFileCopyMoveJob::CancelError);
            break;
        }

        qCDebug(fileJob()) << "no handle," << "default action:" << lastErrorHandleAction
                           << "source url:" << sourceInfo->fileUrl()
                           << "target url:" << (targetInfo ? targetInfo->fileUrl() : DUrl());

        return lastErrorHandleAction;
    }

    setState(DFileCopyMoveJob::SleepState);

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
        emit q->stateChanged(DFileCopyMoveJob::PausedState);
        if (!jobWait()) {
            setError(DFileCopyMoveJob::CancelError);
            qCDebug(fileJob()) << "Will be abort";

            return false;
        }
    } else if (state == DFileCopyMoveJob::StoppedState) {
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

QString DFileCopyMoveJobPrivate::getNewFileName(const DAbstractFileInfo *sourceFileInfo, const DAbstractFileInfo *targetDirectory)
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

bool DFileCopyMoveJobPrivate::doProcess(const DUrl &from, DAbstractFileInfoPointer source_info, const DAbstractFileInfo *target_info)
{
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
            ok = removeFile(handler, source_info.constData());
            if (ok) {
                joinToCompletedFileList(from, DUrl(), size);
            }
        } else {
            // 删除文件夹时先设置其权限
            if (fileHints.testFlag(DFileCopyMoveJob::ForceDeleteFile)) {
                handler->setPermissions(source_info->fileUrl(), QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser);
            }

            ok = mergeDirectory(handler, source_info.constData(), nullptr);
            if (ok) {
                joinToCompletedDirectoryList(from, DUrl(), size);
            }
        }

        return ok;
    }

    QString file_name = handle ? handle->getNewFileName(q_ptr, source_info.constData()) : source_info->fileName();

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
        if ((mode == DFileCopyMoveJob::MoveMode || mode == DFileCopyMoveJob::CutMode)
                && new_file_info->fileUrl() == from) {
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
            file_name = handle ? handle->getNonExistsFileName(q_ptr, source_info.constData(), target_info)
                        : getNewFileName(source_info.constData(), target_info);
            goto create_new_file_info;
        default:
            return false;
        }
    }

    if (source_info->isSymLink()) {
        bool ok = false;

        if (mode == DFileCopyMoveJob::CopyMode) {
            if (fileHints.testFlag(DFileCopyMoveJob::FollowSymlink)) {
                do {
                    const DAbstractFileInfoPointer &symlink_target = DFileService::instance()->createFileInfo(nullptr, source_info->symLinkTarget());

                    if (!symlink_target->exists()) {
                        break;
                    }

                    source_info = symlink_target;
                } while (source_info->isSymLink());

                if (source_info->exists()) {
                    goto process_file;
                }
            }

            ok = linkFile(handler, new_file_info.constData(), source_info->symlinkTargetPath());
        } else {
            ok = renameFile(handler, source_info.constData(), new_file_info.constData());
        }

        if (ok) {
            joinToCompletedFileList(from, new_file_info->fileUrl(), 0);
        }

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
                if (!removeFile(handler, new_file_info.constData())) {
                    return false;
                }
            } else if (new_file_info->exists()) {
                // 复制文件时，如果需要覆盖，必须添加可写入权限
                handler->setPermissions(new_file_info->fileUrl(), QFileDevice::WriteUser | QFileDevice::ReadUser);
            }

            ok = copyFile(source_info.constData(), new_file_info.constData());

            if (ok) {
                handler->setFileTime(new_file_info->fileUrl(), source_info->lastRead(), source_info->lastModified());

                QFileDevice::Permissions permissions = source_info->permissions();
                //! use stat function to read vault file permission.
                QString path = source_info->fileUrl().path();
                if (VaultController::isVaultFile(path)) {
                    permissions = VaultController::getPermissions(path);
                }

                handler->setPermissions(new_file_info->fileUrl(), /*source_info->permissions()*/permissions);
            }
        } else {
            ok = renameFile(handler, source_info.constData(), new_file_info.constData());
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
            ok = mergeDirectory(handler, source_info.constData(), new_file_info.constData());
        } else if (!handler->rename(source_info->fileUrl(), new_file_info->fileUrl())) { // 尝试直接rename操作
            qCDebug(fileJob(), "Failed on rename, Well be copy and delete the directory");
            ok = mergeDirectory(handler, source_info.constData(), new_file_info.constData());
        }

        if (ok) {
            handler->setFileTime(new_file_info->fileUrl(), si_last_read, si_last_modified);
            joinToCompletedDirectoryList(from, new_file_info->fileUrl(), size);
        }

        return ok;
    }

    return false;
}

bool DFileCopyMoveJobPrivate::mergeDirectory(DFileHandler *handler, const DAbstractFileInfo *fromInfo, const DAbstractFileInfo *toInfo)
{
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
                    action = handleError(fromInfo, toInfo);
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

                action = handleError(fromInfo, toInfo);
            }

        } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

        if (action != DFileCopyMoveJob::NoAction) {
            return action == DFileCopyMoveJob::SkipAction;
        }
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

        if (!process(url, info, toInfo)) {
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

bool DFileCopyMoveJobPrivate::doCopyFile(const DAbstractFileInfo *fromInfo, const DAbstractFileInfo *toInfo, int blockSize)
{
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
                    action = handleError(fromInfo, nullptr);
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

                action = handleError(fromInfo, nullptr);
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
                qCDebug(fileJob()) << "open error:" << toInfo->fileUrl();

                if (!toInfo->exists() || toInfo->isWritable()) {
                    setError(DFileCopyMoveJob::OpenError, qApp->translate("DFileCopyMoveJob", "Failed to open the file, cause: %1").arg(toDevice->errorString()));
                } else {
                    setError(DFileCopyMoveJob::PermissionError);
                }

                action = handleError(toInfo, nullptr);
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
                    action = handleError(toInfo, nullptr);
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

//    int writtenDataSize = 0;
    uLong source_checksum = adler32(0L, nullptr, 0);

    Q_FOREVER {
        qint64 current_pos = fromDevice->pos();
read_data:
        if (Q_UNLIKELY(!stateCheck())) {
            return false;
        }

        char data[blockSize + 1];
        qint64 size_read = fromDevice->read(data, blockSize);

        if (Q_UNLIKELY(size_read <= 0)) {
            if (fromDevice->atEnd()) {
                break;
            }

            const_cast<DAbstractFileInfo *>(fromInfo)->refresh();

            if (fromInfo->exists()) {
                setError(DFileCopyMoveJob::ReadError, qApp->translate("DFileCopyMoveJob", "Failed to read the file, cause: %1").arg(fromDevice->errorString()));
            } else {
                setError(DFileCopyMoveJob::NonexistenceError);
            }

            switch (handleError(fromInfo, toInfo)) {
            case DFileCopyMoveJob::RetryAction: {
                if (!fromDevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, fromDevice->errorString());

                    return false;
                }

                goto read_data;
            }
            case DFileCopyMoveJob::SkipAction:
                return true;
            default:
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
                return false;
            }
            setError(DFileCopyMoveJob::WriteError, qApp->translate("DFileCopyMoveJob", "Failed to write the file, cause: %1").arg(toDevice->errorString()));
            switch (handleError(fromInfo, toInfo)) {
            case DFileCopyMoveJob::RetryAction: {
                if (!toDevice->seek(current_pos)) {
                    setError(DFileCopyMoveJob::UnknowError, toDevice->errorString());
                    //临时处理 fix
                    //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                    //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                    if (FileUtils::isGvfsMountFile(toInfo->path())) {
                        toDevice->closeWriteReadFailed(true);
                    }
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
                return true;
            default:
                //临时处理 fix
                //判断是否是网络文件，是，就去调用closeWriteReadFailed，不去调用g_output_stream_close(d->output_stream, nullptr, nullptr);
                //在失去网络，网络文件调用gio 的 g_output_stream_close 关闭 output_stream，会卡很久
                if (FileUtils::isGvfsMountFile(toInfo->path())) {
                    toDevice->closeWriteReadFailed(true);
                }
                return false;
            }
        }
        //fix 修复vfat格式u盘卡死问题，写入数据后立刻同步
        const DStorageInfo &targetStorageInfo = directoryStack.top().targetStorageInfo;
        if (targetStorageInfo.isValid()) {
            const QString &fs_type = targetStorageInfo.fileSystemType();
            if (fs_type == "vfat") {
                toDevice->inherits("");
                if (size_write > 0)
                    toDevice->syncToDisk();
            }
        }

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

                switch (handleError(fromInfo, toInfo)) {
                case DFileCopyMoveJob::RetryAction: {
                    if (!toDevice->seek(current_pos)) {
                        setError(DFileCopyMoveJob::UnknowError, toDevice->errorString());

                        return false;
                    }

                    goto write_data;
                }
                case DFileCopyMoveJob::SkipAction:
                    return true;
                default:
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
    // 关闭文件时可能会需要很长时间，因为内核可能要把内存里的脏数据回写到硬盘
    setState(DFileCopyMoveJob::IOWaitState);
    fromDevice->close();
    toDevice->close();

    if (state == DFileCopyMoveJob::IOWaitState) {
        setState(DFileCopyMoveJob::RunningState);
    } else if (Q_UNLIKELY(!stateCheck())) {
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
            action = handleError(toInfo, nullptr);
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

    char data[blockSize + 1];
    ulong target_checksum = adler32(0L, nullptr, 0);

    qint64 elapsed_time_checksum = 0;

    if (fileJob().isDebugEnabled()) {
        elapsed_time_checksum = updateSpeedElapsedTimer->elapsed();
    }

    Q_FOREVER {
        qint64 size = toDevice->read(data, blockSize);

        if (Q_UNLIKELY(size <= 0)) {
            if (toDevice->atEnd()) {
                break;
            }

            setError(DFileCopyMoveJob::IntegrityCheckingError, qApp->translate("DFileCopyMoveJob", "File integrity was damaged, cause: %1").arg(toDevice->errorString()));

            switch (handleError(fromInfo, toInfo)) {
            case DFileCopyMoveJob::RetryAction: {
                continue;
            }
            case DFileCopyMoveJob::SkipAction:
                return true;
            default:
                return false;
            }
        }

        target_checksum = adler32(target_checksum, reinterpret_cast<Bytef *>(data), static_cast<uInt>(size));

        if (Q_UNLIKELY(!stateCheck())) {
            return false;
        }
    }

    qCDebug(fileJob(), "Time spent of integrity check of the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed_time_checksum);

    if (source_checksum != target_checksum) {
        qCWarning(fileJob(), "Failed on file integrity checking, source file: 0x%lx, target file: 0x%lx", source_checksum, target_checksum);

        setError(DFileCopyMoveJob::IntegrityCheckingError);
        DFileCopyMoveJob::Action action = handleError(fromInfo, toInfo);

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

bool DFileCopyMoveJobPrivate::doRemoveFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo)
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

        action = handleError(fileInfo, nullptr);
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning() );

    return action == DFileCopyMoveJob::SkipAction;
}

bool DFileCopyMoveJobPrivate::doRenameFile(DFileHandler *handler, const DAbstractFileInfo *oldInfo, const DAbstractFileInfo *newInfo)
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
    if (!doCopyFile(oldInfo, newInfo)) {
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

bool DFileCopyMoveJobPrivate::doLinkFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo, const QString &linkPath)
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
        action = handleError(fileInfo, nullptr);
        q->msleep(500); // fix bug#30091 文件操作失败的时候，点击对话框的“不再提示+重试”，会导致不停失败不停发送信号通知主线程更新ui，这里加个延时控制响应频率
    } while (action == DFileCopyMoveJob::RetryAction && this->isRunning());

    return action == DFileCopyMoveJob::SkipAction;
}

bool DFileCopyMoveJobPrivate::process(const DUrl &from, const DAbstractFileInfo *target_info)
{
    const DAbstractFileInfoPointer &source_info = DFileService::instance()->createFileInfo(nullptr, from);

    return process(from, source_info, target_info);
}

bool DFileCopyMoveJobPrivate::process(const DUrl &from, const DAbstractFileInfoPointer &source_info, const DAbstractFileInfo *target_info)
{
    // reset error and action
    unsetError();
    lastErrorHandleAction = DFileCopyMoveJob::NoAction;

    beginJob(JobInfo::Preprocess, from, (source_info && target_info) ? target_info->getUrlByChildFileName(source_info->fileName()) : DUrl());
    bool ok = doProcess(from, source_info, target_info);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::copyFile(const DAbstractFileInfo *fromInfo, const DAbstractFileInfo *toInfo, int blockSize)
{
    qint64 elapsed = 0;

    if (fileJob().isDebugEnabled()) {
        elapsed = updateSpeedElapsedTimer->elapsed();
    }

    beginJob(JobInfo::Copy, fromInfo->fileUrl(), toInfo->fileUrl());
    bool ok = doCopyFile(fromInfo, toInfo, blockSize);
    endJob();

    qCDebug(fileJob(), "Time spent of copy the file: %lld", updateSpeedElapsedTimer->elapsed() - elapsed);

    return ok;
}

bool DFileCopyMoveJobPrivate::removeFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo)
{
    beginJob(JobInfo::Remove, fileInfo->fileUrl(), DUrl());
    bool ok = doRemoveFile(handler, fileInfo);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::renameFile(DFileHandler *handler, const DAbstractFileInfo *oldInfo, const DAbstractFileInfo *newInfo)
{
    beginJob(JobInfo::Move, oldInfo->fileUrl(), newInfo->fileUrl());
    bool ok = doRenameFile(handler, oldInfo, newInfo);
    endJob();

    return ok;
}

bool DFileCopyMoveJobPrivate::linkFile(DFileHandler *handler, const DAbstractFileInfo *fileInfo, const QString &linkPath)
{
    beginJob(JobInfo::Link, DUrl(linkPath), fileInfo->fileUrl());
    bool ok = doLinkFile(handler, fileInfo, linkPath);
    endJob();

    return ok;
}

void DFileCopyMoveJobPrivate::beginJob(JobInfo::Type type, const DUrl &from, const DUrl &target)
{
//    qCDebug(fileJob(), "job begin, Type: %d, from: %s, to: %s", type, qPrintable(from.toString()), qPrintable(target.toString()));

    jobStack.push({type, QPair<DUrl, DUrl>(from, target)});
    currentJobDataSizeInfo = qMakePair(-1, 0);
    currentJobFileHandle = -1;

    Q_EMIT q_ptr->currentJobChanged(from, target);
}

void DFileCopyMoveJobPrivate::endJob()
{
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
    qCDebug(fileJob(), "file. from: %s, target: %s, data size: %lld", qPrintable(from.toString()), qPrintable(target.toString()), dataSize);

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
//    qCDebug(fileJob(), "directory. from: %s, target: %s, data size: %lld", qPrintable(from.toString()), qPrintable(target.toString()), dataSize);

//    completedDataSize += dataSize;
    completedProgressDataSize += targetIsRemovable <= 0 ? 4096 : 0;
    ++completedFilesCount;

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
    const qint64 totalSize = fileStatistics->totalProgressSize();
    //通过getCompletedDataSize取出的已传输的数据大小后期会远超实际数据大小，这种情况下直接使用completedDataSize
    qint64 dataSize(getCompletedDataSize());
    // completedDataSize 可能一直为 0
    if (dataSize > completedDataSize && completedDataSize > 0) {
        dataSize = completedDataSize;
    }

    dataSize = targetIsRemovable <= 0 ? completedDataSizeOnBlockDevice : dataSize;

    dataSize += completedProgressDataSize;

    if (totalSize == 0)
        return;

    if (fileStatistics->isFinished()) {
        qreal realProgress = qreal(dataSize) / totalSize;
        if (realProgress > lastProgress)
            lastProgress = realProgress;
        qCDebug(fileJob(), "completed data size: %lld, total data size: %lld", dataSize, fileStatistics->totalSize());
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
    const qint64 total_size = getCompletedDataSize();
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

    // 因为sleep状态时可能会导致进度信息长时间无法得到更新，故在此处直接更新进度信息
    if (state == DFileCopyMoveJob::IOWaitState) {
        updateProgress();
    } else {
        needUpdateProgress = true;
    }
}

DFileCopyMoveJob::DFileCopyMoveJob(QObject *parent)
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

    if (d->fileStatistics->isRunning()) {
        d->fileStatistics->stop();
        d->fileStatistics->wait();
    }

    d->fileStatistics->start(sourceUrls);

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

        if (!d->process(source, source_info, target_info.constData())) {
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
    if (d->targetIsRemovable && mayExecSync &&
            d->state != DFileCopyMoveJob::StoppedState) { //主动取消时state已经被设置为stop了
        qCDebug(fileJob()) << "sync file, lastErrorHandleAction" << d->lastErrorHandleAction
                           << ", state:" << d->state;
        // 任务完成后执行 sync 同步数据到硬盘, 同时将状态改为 SleepState，用于定时器更新进度和速度信息
        d->setState(IOWaitState);
        int syncRet = 0;
        auto result = QtConcurrent::run([&d, &syncRet]() {
            syncRet = QProcess::execute("sync", {"-f", d->targetRootPath});
        });
        // 检测同步时是否被停止，若停止则立即跳出
        while (!result.isFinished()) {
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

    qDebug() << mayExecSync ;

    d->fileStatistics->stop();
    d->setState(StoppedState);

    if (d->error == NoError) {
        Q_EMIT progressChanged(1, d->completedDataSize);
    }

    qCDebug(fileJob()) << "job finished, error:" << error() << ", message:" << errorString();
}

QString DFileCopyMoveJob::Handle::getNewFileName(DFileCopyMoveJob *job, const DAbstractFileInfo *sourceInfo)
{
    Q_UNUSED(job)

    return job->d_func()->formatFileName(sourceInfo->fileName());
}

QString DFileCopyMoveJob::Handle::getNonExistsFileName(DFileCopyMoveJob *job, const DAbstractFileInfo *sourceInfo, const DAbstractFileInfo *targetDirectory)
{
    Q_UNUSED(job)

    return DFileCopyMoveJobPrivate::getNewFileName(sourceInfo, targetDirectory);
}

DFM_END_NAMESPACE

#include "moc_dfilecopymovejob.cpp"
