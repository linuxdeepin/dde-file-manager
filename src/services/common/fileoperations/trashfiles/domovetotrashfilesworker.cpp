/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "domovetotrashfilesworker.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/core/diofactory.h>

#include <QUrl>
#include <QDebug>
#include <QWaitCondition>
#include <QtGlobal>
#include <QCryptographicHash>
#include <QStorageInfo>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

static const quint32 kMoveMaxBufferLength { 1024 * 1024 * 1 };

USING_IO_NAMESPACE
DSC_USE_NAMESPACE
DoMoveToTrashFilesWorker::DoMoveToTrashFilesWorker(QObject *parent)
    : AbstractWorker(parent)
{
}

DoMoveToTrashFilesWorker::~DoMoveToTrashFilesWorker()
{
}
/*!
 * \brief DoMoveToTrashFilesWorker::doWork the thread move to trash work function
 * \return move to trash is successed
 */
bool DoMoveToTrashFilesWorker::doWork()
{
    // The endcopy interface function has been called here
    if (!AbstractWorker::doWork())
        return false;
    // ToDo::执行移动到回收站的业务逻辑

    doMoveToTrash();

    endWork();

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::statisticsFilesSize init target arguments
 * \return
 */
bool DoMoveToTrashFilesWorker::statisticsFilesSize()
{
    sourceFilesCount = sources.size();
    target = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashFilesPath));
    trashLocalDir = QString("%1/.local/share/Trash").arg(QDir::homePath());
    targetStorageInfo.reset(new QStorageInfo(trashLocalDir));

    return true;
}

/*!
 * \brief DoCopyFilesWorker::doHandleErrorAndWait Blocking handles errors and returns
 * actions supported by the operation
 * \param from source information
 * \param to target information
 * \param error error type
 * \param needRetry is neef retry action
 * \param errorMsg error message
 * \return support action
 */
AbstractJobHandler::SupportAction DoMoveToTrashFilesWorker::doHandleErrorAndWait(const QUrl &from, const AbstractJobHandler::JobErrorType &error, const QString &errorMsg)
{
    setStat(AbstractJobHandler::JobState::kPauseState);
    emitErrorNotify(from, target, error, errorMsg);

    if (!handlingErrorQMutex)
        handlingErrorQMutex.reset(new QMutex);
    handlingErrorQMutex->lock();
    if (handlingErrorCondition.isNull())
        handlingErrorCondition.reset(new QWaitCondition);
    handlingErrorCondition->wait(handlingErrorQMutex.data());
    handlingErrorQMutex->unlock();

    return currentAction;
}
/*!
 * \brief DoMoveToTrashFilesWorker::doMoveToTrash do move to trash
 * \return move to trash success
 */
bool DoMoveToTrashFilesWorker::doMoveToTrash()
{
    if (!checkTrashDirIsReady())
        return false;

    bool result = false;
    // 总大小使用源文件个数
    for (const auto &url : sources) {
        if (!stateCheck())
            return false;

        if (url.path().startsWith(target.path())) {
            compeleteFilesCount++;
            continue;
        }

        // url是否可以删除 canrename
        if (!isCanMoveToTrash(url, result)) {
            if (result) {
                compeleteFilesCount++;
                continue;
            }

            return false;
        }

        const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!fileInfo) {
            // pause and emit error msg
            if (AbstractJobHandler::SupportAction::kSkipAction != doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kProrogramError)) {
                return false;
            } else {
                compeleteFilesCount++;
                continue;
            }
        }

        isInSameDisk(fileInfo);

        bool ok = false;

        if (fileInfo->isSymLink()) {
            ok = handleSymlinkFile(fileInfo);
        } else {
            ok = handleMoveToTrash(fileInfo);
        }

        if (!ok)
            return false;
        // 写入trash信息
        // 链接文件设置 1。写入trash信息，2。在回收站创建新的链接，3。删除原来的链接
        // 获取文件是否和回收站 同一个磁盘
        // 是，执rename,加入完成列表
        // 否，检查磁盘空间，1。是执行拷贝，拷贝成功删除源文件，2。否错误提示
    }
    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::checkTrashDirIsReady check trash dir is exit or ready
 * \return
 */
bool DoMoveToTrashFilesWorker::checkTrashDirIsReady()
{
    QDir trashDir;

    if (!trashDir.mkpath(StandardPaths::location(StandardPaths::kTrashFilesPath))) {
        doHandleErrorAndWait(sources.first(), AbstractJobHandler::JobErrorType::kMakeStandardTrashError);
        qWarning() << " mk " << StandardPaths::location(StandardPaths::kTrashInfosPath) << "failed!";

        return false;
    }

    if (!trashDir.mkpath(StandardPaths::location(StandardPaths::kTrashInfosPath))) {
        doHandleErrorAndWait(sources.first(), AbstractJobHandler::JobErrorType::kMakeStandardTrashError);
        qWarning() << " mk " << StandardPaths::location(StandardPaths::kTrashInfosPath) << "failed!";

        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::canMoveToTrash check the source file can move to trash
 * \param filePath the source file path
 * \return can move to trash
 */
bool DoMoveToTrashFilesWorker::canMoveToTrash(const QString &filePath)
{
    // 如果是root，则拥有权限
    if (getuid() == 0)
        return true;

    QFileInfo file_info(filePath);
    QString folderPath = file_info.dir().absolutePath();
    QFileInfo dir_info(folderPath);

    bool isFolderWritable = false;

    QFileInfo folderinfo(folderPath);   // 判断上层文件是否是只读，有可能上层是只读，而里面子文件或文件夾又是可以写

    isFolderWritable = folderinfo.isWritable();

    if (!isFolderWritable)
        return false;

#ifdef Q_OS_LINUX
    struct stat statBuffer;
    if (::lstat(dir_info.absoluteFilePath().toUtf8().constData(), &statBuffer) == 0) {
        // 如果父目录拥有t权限，则判断当前用户是不是文件的owner，不是则无法操作文件
        if ((statBuffer.st_mode & S_ISVTX) && file_info.ownerId() != getuid()) {
            return false;
        }
    }
#endif

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::isCanMoveToTrash loop to check the source file can move to trash
 * \param url the source file url
 * \param result Output parameters, is skip this file
 * \return can move to trash
 */
bool DoMoveToTrashFilesWorker::isCanMoveToTrash(const QUrl &url, bool &result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    do {
        if (!canMoveToTrash(url.path()))
            // pause and emit error msg
            action = doHandleErrorAndWait(url, AbstractJobHandler::JobErrorType::kPermissionDeniedError);

    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::handleSymlinkFile Process linked files, move to the recycle bin,
 * create new linked files, and delete source linked files
 * \param File information of link file
 * \return Process successed
 */
bool DoMoveToTrashFilesWorker::handleSymlinkFile(const AbstractFileInfoPointer &fileInfo)
{
    if (!stateCheck())
        return false;

    QFileInfo fromInfo(fileInfo->url().path());
    QString srcFileName = fromInfo.fileName();
    QString srcPath = fileInfo->path();
    QString targetPath;

    bool result = false;
    if (!WriteTrashInfo(fileInfo, targetPath, result)) {
        return result;
    }
    QFile targetFile(fileInfo->symLinkTarget());

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        if (!targetFile.link(targetPath))
            // pause and emit error msg
            action = doHandleErrorAndWait(fileInfo->url(), AbstractJobHandler::JobErrorType::kSymlinkError);

    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        return action == AbstractJobHandler::SupportAction::kSkipAction;
    }

    handler->deleteFile(fileInfo->url());

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::handleMoveToTrash Processing ordinary files and moving them to the recycle bin
 * \param fileInfo File information of source file
 * \return Is the process successful
 */
bool DoMoveToTrashFilesWorker::handleMoveToTrash(const AbstractFileInfoPointer &fileInfo)
{
    bool result = false;
    QString targetPath;
    if (!WriteTrashInfo(fileInfo, targetPath, result)) {
        return result;
    }

    emitCurrentTaskNotify(fileInfo->url(), QUrl::fromLocalFile(targetPath));

    // ToDo::判断是否同盘，是就直接rename
    if (isSameDisk == 1) {

        AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

        do {
            if (!handler->renameFile(fileInfo->url(), QUrl::fromLocalFile(targetPath))) {
                action = doHandleErrorAndWait(fileInfo->url(), AbstractJobHandler::JobErrorType::kRenameError, handler->errorString());
            }
        } while (isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

        compeleteFilesCount++;

        if (action == AbstractJobHandler::SupportAction::kSkipAction || action == AbstractJobHandler::SupportAction::kNoAction)
            return true;

        return false;
    }

    // ToDo:: 不是检查文件是否操过1G，再检查磁盘空间是否够用，再执行拷贝文件，再去删除文件
    if (!checkFileOutOfLimit(fileInfo)) {
        // ToDo::大于1G，执行侧底删除代码

        return deleteFile(fileInfo);
    }
    // 检查磁盘空间是否不足
    if (!checkDiskSpaceAvailable(fileInfo->url(), result))
        return result;
    // 拷贝并删除文件
    const auto &toInfo = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(targetPath));
    if (!toInfo) {
        // pause and emit error msg
        return AbstractJobHandler::SupportAction::kSkipAction == doHandleErrorAndWait(toInfo->url(), AbstractJobHandler::JobErrorType::kProrogramError);
    }

    return doCopyAndDelete(fileInfo, toInfo);
}
/*!
 * \brief DoMoveToTrashFilesWorker::checkFileOutOfLimit Check whether the file size exceeds the limit value
 * \param fileInfo File information of source file
 * \return Have you manipulated the limit value
 */
bool DoMoveToTrashFilesWorker::checkFileOutOfLimit(const AbstractFileInfoPointer &fileInfo)
{
    return FileOperationsUtils::isFilesSizeOutLimit(fileInfo->url(), 1024 * 1024 * 1024);
}

bool DoMoveToTrashFilesWorker::WriteTrashInfo(const AbstractFileInfoPointer &fileInfo, QString &targetPath, bool &result)
{
    if (!stateCheck())
        return false;

    QString path = target.path();
    QString baseName = getNotExistsTrashFileName(fileInfo->fileName());
    QString newName = path + baseName;
    QString delTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    qDebug() << " writeTrashInfo " << fileInfo->url();
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        if (!doWriteTrashInfo(baseName, fileInfo->path(), delTime))
            // pause and emit error msg
            action = doHandleErrorAndWait(fileInfo->url(), AbstractJobHandler::JobErrorType::kPermissionDeniedError);

    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    targetPath = newName;

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::getNotExistsTrashFileName Gets the name of a nonexistent recycle bin file
 * \param fileName the name of source file
 * \return the name of a nonexistent recycle bin file
 */
QString DoMoveToTrashFilesWorker::getNotExistsTrashFileName(const QString &fileName)
{
    QByteArray name = fileName.toUtf8();

    int index = name.lastIndexOf('/');

    if (index >= 0)
        name = name.mid(index + 1);

    index = name.lastIndexOf('.');
    QByteArray suffix;

    if (index >= 0)
        suffix = name.mid(index);

    if (suffix.size() > 200)
        suffix = suffix.left(200);

    name.chop(suffix.size());
    name = name.left(200 - suffix.size());

    QString trashpath = StandardPaths::location(StandardPaths::kTrashFilesPath) + "/";

    while (true) {
        QFileInfo info(trashpath + name + suffix);
        // QFile::exists ==> If the file is a symlink that points to a non-existing file, false is returned.
        if (!info.isSymLink() && !info.exists()) {
            break;
        }

        name = QCryptographicHash::hash(name, QCryptographicHash::Md5).toHex();
    }

    return QString::fromUtf8(name + suffix);
}
/*!
 * \brief DoMoveToTrashFilesWorker::doWriteTrashInfo Write information about files moved to the recycle bin
 * \param fileBaseName the base Name of the file moved to the recycle bin
 * \param path The path to the recycle bin file
 * \param time Time the file was moved to the recycle bin
 * \return Whether the write is complete
 */
bool DoMoveToTrashFilesWorker::doWriteTrashInfo(const QString &fileBaseName, const QString &path, const QString &time)
{
    QFile metadata(trashLocalDir + "/info/" + fileBaseName + ".trashinfo");

    if (!metadata.open(QIODevice::WriteOnly)) {
        qDebug() << metadata.fileName() << "file open error:" << metadata.errorString();

        return false;
    }

    QByteArray data;

    data.append("[Trash Info]\n");
    data.append("Path=").append(path.toUtf8().toPercentEncoding("/")).append("\n");
    data.append("DeletionDate=").append(time).append("\n");

    // save the file tag info
    const QStringList tag_name_list;

    qint64 size = metadata.write(data);

    metadata.close();

    if (size < 0) {
        qDebug() << "write file " << metadata.fileName() << "error:" << metadata.errorString();
    }

    return size > 0;
}
/*!
 * \brief DoMoveToTrashFilesWorker::getIsInSameDisk Check that the
 * source file and the recycle bin directory are on the same disk
 * \param fileInfo File information of source file
 */
void DoMoveToTrashFilesWorker::isInSameDisk(const AbstractFileInfoPointer &fileInfo)
{
    if (isSameDisk > -1)
        return;

    const QStorageInfo &sourceStorage = fileInfo->isSymLink() ? QStorageInfo(fileInfo->absolutePath())
                                                              : QStorageInfo(fileInfo->absoluteFilePath());

    isSameDisk = sourceStorage.rootPath() == targetStorageInfo->rootPath();
}
/*!
 * \brief DoMoveToTrashFilesWorker::checkDiskSpaceAvailable Check
 * whether the disk where the recycle bin directory is located
 * has space of the size of the source file
 * \param file URL of the source file
 * \param result Output parameter: whether skip
 * \return Is space available
 */
bool DoMoveToTrashFilesWorker::checkDiskSpaceAvailable(const QUrl &file, bool &result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;

    do {
        targetStorageInfo->refresh();
        qint64 freeBytes = targetStorageInfo->bytesFree();

        FileOperationsUtils::isFilesSizeOutLimit(file, freeBytes);
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::doCopyAndDelete Execute file copy and delete source files
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \return Copy and delete successfully
 */
bool DoMoveToTrashFilesWorker::doCopyAndDelete(const AbstractFileInfoPointer &fromInfo,
                                               const AbstractFileInfoPointer &toInfo)
{
    bool result = false;
    if (fromInfo->isFile()) {

        if (!copyFile(fromInfo, toInfo, result))
            return result;

    } else {
        if (!copyDir(fromInfo, toInfo, result))
            return result;
    }

    if (!deleteFile(fromInfo))
        return false;

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::readAheadSourceFile Pre read source file content
 * \param fileInfo File information of source file
 */
void DoMoveToTrashFilesWorker::readAheadSourceFile(const AbstractFileInfoPointer &fileInfo)
{
    if (fileInfo->size() <= 0)
        return;
    std::string stdStr = fileInfo->url().path().toUtf8().toStdString();
    int fromfd = open(stdStr.data(), O_RDONLY);
    if (-1 != fromfd) {
        readahead(fromfd, 0, static_cast<size_t>(fileInfo->size()));
        close(fromfd);
    }
}
/*!
 * \brief DoMoveToTrashFilesWorker::createFileDevices Device for creating source and directory files
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param fromeFile Output parameter: device of source file
 * \param toFile Output parameter: device of target file
 * \param result result Output parameter: whether skip
 * \return Whether the device of source file and target file is created successfully
 */
bool DoMoveToTrashFilesWorker::createFileDevices(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                                                 QSharedPointer<DFile> &fromeFile, QSharedPointer<DFile> &toFile, bool &result)
{
    if (!createFileDevice(fromInfo, fromeFile, result))
        return false;
    if (!createFileDevice(toInfo, toFile, result))
        return false;
    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::createFileDevice Device to create the file
 * \param needOpenInfo file information
 * \param file fromeFile Output parameter: file device
 * \param result result result Output parameter: whether skip
 * \return Is the device of the file created successfully
 */
bool DoMoveToTrashFilesWorker::createFileDevice(const AbstractFileInfoPointer &needOpenInfo, QSharedPointer<DFile> &file, bool &result)
{
    file.reset(nullptr);
    QUrl url = needOpenInfo->url();
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QSharedPointer<DIOFactory> factory { nullptr };
    do {
        factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
        if (!factory) {
            action = doHandleErrorAndWait(needOpenInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io factory failed!"));
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    do {
        file = factory->createFile();
        if (!file) {
            action = doHandleErrorAndWait(needOpenInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io dfile failed!"));
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::openFiles Open source and destination files
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param fromeFile device of source file
 * \param toFile device of target file
 * \param result result Output parameter: whether skip
 * \return Open source and target files successfully
 */
bool DoMoveToTrashFilesWorker::openFiles(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, const QSharedPointer<DFile> &fromeFile, const QSharedPointer<DFile> &toFile, bool &result)
{
    if (fromInfo->size() > 0 && !openFile(fromInfo, fromeFile, DFile::OpenFlag::ReadOnly, result)) {
        return false;
    }

    if (!openFile(toInfo, toFile, DFile::OpenFlag::Truncate, result)) {
        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::openFile
 * \param fileInfo file information
 * \param file file deivce
 * \param flags Flag for opening file
 * \param result result Output parameter: whether skip
 * \return wether open the file successfully
 */
bool DoMoveToTrashFilesWorker::openFile(const AbstractFileInfoPointer &fileInfo, const QSharedPointer<DFile> &file, const DFile::OpenFlag &flags, bool &result)
{
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!file->open(flags)) {
            action = doHandleErrorAndWait(fileInfo->url(), AbstractJobHandler::JobErrorType::kDfmIoError, QObject::tr("create dfm io dfile failed!"));
        }
    } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
    if (action != AbstractJobHandler::SupportAction::kNoAction) {
        result = action == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }
    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::doReadFile  Read file contents
 * \param fileInfo file information
 * \param fromDevice file device
 * \param data Data buffer
 * \param blockSize Data buffer size
 * \param readSize Read size
 * \param result result Output parameter: whether skip
 * \return Read successfully
 */
bool DoMoveToTrashFilesWorker::doReadFile(const AbstractFileInfoPointer &fileInfo, const QSharedPointer<DFile> &fromDevice, char *data, const qint64 &blockSize, qint64 &readSize, bool &result)
{
    readSize = 0;
    qint64 currentPos = fromDevice->pos();
    AbstractJobHandler::SupportAction actionForRead = AbstractJobHandler::SupportAction::kNoAction;

    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }
    do {
        readSize = fromDevice->read(data, blockSize);
        if (Q_UNLIKELY(!stateCheck())) {
            return false;
        }

        if (Q_UNLIKELY(readSize <= 0)) {
            if (readSize == 0 && fileInfo->size() == fromDevice->pos()) {
                return true;
            }
            fileInfo->refresh();
            AbstractJobHandler::JobErrorType errortype = fileInfo->exists() ? AbstractJobHandler::JobErrorType::kReadError : AbstractJobHandler::JobErrorType::kNonexistenceError;
            QString errorstr = fileInfo->exists() ? QString(QObject::tr("DFileCopyMoveJob", "Failed to read the file, cause: %1")).arg("to something!") : QString();

            actionForRead = doHandleErrorAndWait(fileInfo->url(), errortype, errorstr);

            if (actionForRead == AbstractJobHandler::SupportAction::kRetryAction) {
                if (!fromDevice->seek(currentPos)) {
                    AbstractJobHandler::SupportAction actionForReadSeek = doHandleErrorAndWait(fileInfo->url(),
                                                                                               AbstractJobHandler::JobErrorType::kSeekError);
                    result = actionForReadSeek == AbstractJobHandler::SupportAction::kSkipAction;
                    return false;
                }
            }
        }
    } while (!isStopped() && actionForRead == AbstractJobHandler::SupportAction::kRetryAction);

    if (actionForRead != AbstractJobHandler::SupportAction::kNoAction) {
        result = actionForRead == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::doWriteFile  Write file contents
 * \param fileInfo file information
 * \param fromDevice file device
 * \param data Data buffer
 * \param blockSize Data buffer size
 * \param readSize Write size
 * \param result result Output parameter: whether skip
 * \return Write successfully
 */
bool DoMoveToTrashFilesWorker::doWriteFile(const AbstractFileInfoPointer &fileInfo, const QSharedPointer<DFile> &toDevice, const char *data, const qint64 &readSize, bool &result)
{
    qint64 currentPos = toDevice->pos();
    AbstractJobHandler::SupportAction actionForWrite = AbstractJobHandler::SupportAction::kNoAction;
    qint64 sizeWrite = 0;
    qint64 surplusSize = readSize;
    do {
        const char *surplusData = data;
        do {
            surplusData += sizeWrite;
            surplusSize -= sizeWrite;
            sizeWrite = toDevice->write(surplusData, surplusSize);
            if (Q_UNLIKELY(!stateCheck()))
                return false;
        } while (sizeWrite > 0 && sizeWrite < surplusSize);

        // 表示全部数据写入完成
        if (sizeWrite > 0) {
            break;
        }
        QString errorstr = QString(QObject::tr("Failed to write the file, cause: %1")).arg("some thing to do!");

        actionForWrite = doHandleErrorAndWait(fileInfo->url(), AbstractJobHandler::JobErrorType::kWriteError, errorstr);
        if (actionForWrite == AbstractJobHandler::SupportAction::kRetryAction) {
            if (!toDevice->seek(currentPos)) {
                AbstractJobHandler::SupportAction actionForWritSeek = doHandleErrorAndWait(fileInfo->url(),
                                                                                           AbstractJobHandler::JobErrorType::kSeekError);
                result = actionForWritSeek == AbstractJobHandler::SupportAction::kSkipAction;

                return false;
            }
        }
    } while (!isStopped() && actionForWrite == AbstractJobHandler::SupportAction::kRetryAction);

    if (actionForWrite != AbstractJobHandler::SupportAction::kNoAction) {
        result = actionForWrite == AbstractJobHandler::SupportAction::kSkipAction;
        return false;
    }

    if (sizeWrite > 0) {
        toDevice->flush();
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::setTargetPermissions Set permissions on the target file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 */
void DoMoveToTrashFilesWorker::setTargetPermissions(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo)
{
    // 修改文件修改时间
    handler->setFileTime(toInfo->url(), fromInfo->lastRead(), fromInfo->lastModified());
    QFileDevice::Permissions permissions = fromInfo->permissions();
    QString path = fromInfo->url().path();
    //权限为0000时，源文件已经被删除，无需修改新建的文件的权限为0000
    if (permissions != 0000)
        handler->setPermissions(toInfo->url(), permissions);
}
/*!
 * \brief DoMoveToTrashFilesWorker::copyFile Copy file
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param reslut result Output parameter: whether skip
 * \return Whether the copied file is complete
 */
bool DoMoveToTrashFilesWorker::copyFile(const AbstractFileInfoPointer &fromInfo,
                                        const AbstractFileInfoPointer &toInfo, bool &reslut)
{
    //预先读取
    readAheadSourceFile(fromInfo);

    // 创建文件的divice
    QSharedPointer<DFile> fromDevice { nullptr }, toDevice { nullptr };
    if (!createFileDevices(fromInfo, toInfo, fromDevice, toDevice, reslut))
        return reslut;

    // 打开文件并创建
    if (!openFiles(fromInfo, toInfo, fromDevice, toDevice, reslut))
        return reslut;

    // 源文件大小如果为0
    if (fromInfo->size() <= 0) {
        // 对文件加权
        setTargetPermissions(fromInfo, toInfo);
        return true;
    }
    // 循环读取和写入文件，拷贝
    qint64 blockSize = fromInfo->size() > kMoveMaxBufferLength ? kMoveMaxBufferLength : fromInfo->size();
    char *data = new char[blockSize + 1];
    qint64 sizeRead = 0;
    do {
        if (!doReadFile(fromInfo, fromDevice, data, blockSize, sizeRead, reslut)) {
            delete[] data;
            data = nullptr;
            return reslut;
        }

        if (!doWriteFile(toInfo, toDevice, data, sizeRead, reslut)) {
            delete[] data;
            data = nullptr;
            return reslut;
        }
    } while (fromDevice->pos() == fromInfo->size());

    delete[] data;
    data = nullptr;

    // 对文件加权
    setTargetPermissions(fromInfo, toInfo);
    if (Q_UNLIKELY(!stateCheck())) {
        return false;
    }

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::copyDir Copy dir
 * \param fromInfo File information of source file
 * \param toInfo File information of target file
 * \param reslut result Output parameter: whether skip
 * \return Whether the copied dir is complete
 */
bool DoMoveToTrashFilesWorker::copyDir(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool &reslut)
{
    // 检查文件的一些合法性，源文件是否存在，创建新的目标目录名称，检查新创建目标目录名称是否存在
    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    QFileDevice::Permissions permissions = fromInfo->permissions();
    if (!toInfo->exists()) {
        do {
            if (handler->mkdir(toInfo->url())) {
                break;
            }
            action = doHandleErrorAndWait(fromInfo->url(), AbstractJobHandler::JobErrorType::kMkdirError, QString(QObject::tr("Fail to create symlink, cause: %1")).arg(handler->errorString()));
        } while (!isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);
        if (AbstractJobHandler::SupportAction::kNoAction != action) {
            // skip write size += all file size in sources dir
            reslut = AbstractJobHandler::SupportAction::kSkipAction == action;
            return false;
        }
    }

    if (fromInfo->countChildFile() <= 0) {
        handler->setPermissions(toInfo->url(), permissions);
        return true;
    }
    // 遍历源文件，执行一个一个的拷贝
    QString error;
    const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(fromInfo->url(), &error);
    if (iterator) {
        doHandleErrorAndWait(fromInfo->url(), AbstractJobHandler::JobErrorType::kProrogramError, QString(QObject::tr("create dir's iterator failed, case : %1")).arg(error));
        return false;
    }

    while (iterator->hasNext()) {
        if (!stateCheck()) {
            return false;
        }

        const QUrl &url = iterator->next();
        Q_UNUSED(url);
        const AbstractFileInfoPointer &info = iterator->fileInfo();
        if (!copyFile(info, toInfo, reslut)) {
            return false;
        }
    }

    handler->setPermissions(toInfo->url(), permissions);

    return true;
}
/*!
 * \brief DoMoveToTrashFilesWorker::deleteFile Delete file
 * \param fileInfo file information
 * \return Delete file successfully
 */
bool DoMoveToTrashFilesWorker::deleteFile(const AbstractFileInfoPointer &fileInfo)
{
    if (!stateCheck())
        return false;

    AbstractJobHandler::SupportAction action = AbstractJobHandler::SupportAction::kNoAction;
    do {
        if (!handler->deleteFile(fileInfo->url())) {
            action = doHandleErrorAndWait(fileInfo->url(), AbstractJobHandler::JobErrorType::kDeleteFileError, handler->errorString());
        }
    } while (isStopped() && action == AbstractJobHandler::SupportAction::kRetryAction);

    if (action == AbstractJobHandler::SupportAction::kSkipAction)
        return true;

    return action != AbstractJobHandler::SupportAction::kNoAction;
}
