#include "filejob.h"

#include "shutil/fileutils.h"
#include "interfaces/dfmstandardpaths.h"
#include "../deviceinfo/udiskdeviceinfo.h"
#include "../deviceinfo/udisklistener.h"
#include "../app/define.h"
#include "../widgets/singleton.h"
#include "../interfaces/dfmglobal.h"
#include "shutil/dmimedatabase.h"

#include <QFile>
#include <QThread>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>
#include <QDirIterator>
#include <QProcess>
#include <QCryptographicHash>
#include <QMetaEnum>
#include <QStorageInfo>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>

int FileJob::FileJobCount = 0;
qint64 FileJob::Msec_For_Display = 1000;
qint64 FileJob::Data_Block_Size = 65536;
qint64 FileJob::Data_Flush_Size = 16777216;


bool FileJob::setDirPermissions(const QString &scrPath, const QString& tarDirPath)
{
    struct stat buf;
    std::string stdSrcPath = scrPath.toStdString();
    stat(stdSrcPath.data(), &buf);
    std::string stdTarDirPath = tarDirPath.toStdString();
    bool success = ::chmod(stdTarDirPath.data(), buf.st_mode & 0777) == 0;
    return success;
}

FileJob::FileJob(JobType jobType, QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QMap<QString, QString>>();
    FileJobCount += 1;
    m_status = FileJob::Started;

    m_trashLoc = QString("%1/.local/share/Trash").arg(QDir::homePath());
    m_id = QString::number(FileJobCount);
    m_jobType = jobType;
    m_jobDetail.insert("jobId", m_id);
    QMetaEnum metaEnum = QMetaEnum::fromType<JobType>();
    QString type = metaEnum.valueToKey(m_jobType);
    m_jobDetail.insert("type", type.toLower());
    connect(this, &FileJob::finished, this, &FileJob::handleJobFinished);

#ifdef SPLICE_CP
    if(pipe(m_filedes) < 0) {
       qDebug() << "Create  pipe fail";
    }
    qDebug() << "Create  pipe successfully";
#endif
}

FileJob::~FileJob()
{
#ifdef SPLICE_CP
    close(m_filedes[0]);
    close(m_filedes[1]);
    qDebug() << "close pipe";
#endif
}

void FileJob::setStatus(FileJob::Status status)
{
    m_status = status;
}

void FileJob::setJobId(const QString &id)
{
    m_id = id;
}

QString FileJob::getJobId()
{
    return m_id;
}

QString FileJob::checkDuplicateName(const QString &name)
{
    QString destUrl = name;
    QFile file(destUrl);
    QFileInfo startInfo(destUrl);
    int num = 1;
    QString cpy = tr("copy");
    while (file.exists())
    {
        if(num == 1)
        {
            if(startInfo.isDir()){
                destUrl = QString("%1/%2(%3)").arg(startInfo.absolutePath(),
                                                   startInfo.fileName(),
                                                   cpy);
            }
            else{
                if (startInfo.completeSuffix().isEmpty()){
                    destUrl = QString("%1/%2(%3)").arg(startInfo.absolutePath(),
                                                          startInfo.baseName(),
                                                          cpy);
                }else{
                    destUrl = QString("%1/%2(%3).%4").arg(startInfo.absolutePath(),
                                                          startInfo.baseName(),
                                                          cpy,
                                                          startInfo.completeSuffix());
                }
            }
        }
        else
        {
            if(startInfo.isDir()){
                destUrl = QString("%1/%2(%3 %4)").arg(startInfo.absolutePath(),
                                                      startInfo.fileName(),
                                                      cpy,
                                                      QString::number(num));
            }
            else{
                if (startInfo.completeSuffix().isEmpty()){
                    destUrl = QString("%1/%2(%3 %4)").arg(startInfo.absolutePath(),
                                                             startInfo.baseName(),
                                                             cpy,
                                                             QString::number(num));
                }else{
                    destUrl = QString("%1/%2(%3 %4).%5").arg(startInfo.absolutePath(),
                                                             startInfo.baseName(),
                                                             cpy,
                                                             QString::number(num),
                                                             startInfo.completeSuffix());
                }
            }
        }
        num++;
        file.setFileName(destUrl);
    }
    return destUrl;
}

void FileJob::setApplyToAll(bool v)
{
    m_applyToAll = v;
}

void FileJob::setReplace(bool v)
{
    m_isReplaced = v;
}

int FileJob::getWindowId()
{
    return m_windowId;
}

QString FileJob::getTargetDir()
{
    return m_tarPath;
}

void FileJob::adjustSymlinkPath(QString &scrPath, QString &tarDirPath)
{
    QFileInfo srcFileInfo(scrPath);
    QFileInfo tarDirFileInfo(tarDirPath);
    if (!srcFileInfo.canonicalFilePath().isEmpty()){
        scrPath = srcFileInfo.canonicalFilePath();
    }
    if (!tarDirFileInfo.canonicalFilePath().isEmpty()){
        tarDirPath = tarDirFileInfo.canonicalFilePath();
    }
}

DUrlList FileJob::doCopy(const DUrlList &files, const DUrl &destination)
{
    return doMoveCopyJob(files, destination);
}

DUrlList FileJob::doMove(const DUrlList &files, const DUrl &destination)
{
    return doMoveCopyJob(files, destination);
}

DUrlList FileJob::doMoveCopyJob(const DUrlList &files, const DUrl &destination)
{
    qDebug() << "Do file operation is started" << m_jobDetail;

    jobPrepared();

    DUrlList list;
    QString tarDirPath = destination.toLocalFile();
    QDir tarDir(tarDirPath);
    QStorageInfo tarStorageInfo(tarDirPath);
    if (files.count() > 0 ){
        QStorageInfo srcStorageInfo(files.at(0).toLocalFile());
        if (srcStorageInfo.rootPath() != tarStorageInfo.rootPath()){
            m_isInSameDisk = false;
        }else if (DMimeDatabase::isGvfsFile(destination.toLocalFile())){
            UDiskDeviceInfoPointer pSrc = deviceListener->getDeviceByFilePath(files.at(0).toLocalFile());
            UDiskDeviceInfoPointer pDes = deviceListener->getDeviceByFilePath(destination.toLocalFile());
            if (pSrc && pDes){
                qDebug() << pSrc->getMountPointUrl() << pDes->getMountPointUrl();
                if (pSrc->getMountPointUrl() != pDes->getMountPointUrl()){
                    m_isInSameDisk = false;
                }
            }
        }
    }

    qDebug() << "m_isInSameDisk" << m_isInSameDisk;
    qDebug() << "mountpoint" << tarStorageInfo.rootPath();

    //No need to check dist usage for moving job in same disk
    if(!(m_jobType == Move && m_isInSameDisk)){
        const bool diskSpaceAvailable = checkDiskSpaceAvailable(files, destination);
        m_isCheckingDisk = false;
        if(!diskSpaceAvailable){
            emit requestNoEnoughSpaceDialogShowed();
            emit requestJobRemovedImmediately(m_jobDetail);
            return DUrlList();
        }
    } else{
        m_totalSize = FileUtils::totalSize(files);
    }

    qDebug() << "m_totalSize" << FileUtils::formatSize(m_totalSize);

    if(!tarDir.exists())
    {
        qDebug() << "Destination must be directory";
        return list;
    }

    for(int i = 0; i < files.size(); i++)
    {
        QUrl url = files.at(i);
        QString srcPath = url.toLocalFile();
        if (srcPath.isEmpty()){
            continue;
        }
        QFileInfo srcInfo(srcPath);
        if (!srcInfo.exists()){
            continue;
        }
        QString targetPath;

        if (m_isAborted)
            break;

        if (srcInfo.isSymLink()){
            handleSymlinkFile(srcPath, tarDirPath, &targetPath);
        }else if (!srcInfo.isSymLink() && srcInfo.isDir()){
            adjustSymlinkPath(srcPath, tarDirPath);
            if (m_jobType == Copy){
                copyDir(srcPath, tarDirPath, false,  &targetPath);
            }else if (m_jobType == Move){
                if(m_isInSameDisk)
                {
                    if (!moveDir(srcPath, tarDirPath, &targetPath)) {
                        if(copyDir(srcPath, tarDirPath, true, &targetPath))
                            deleteDir(srcPath);
                    }
                }
                else
                {
                    if(copyDir(srcPath, tarDirPath, true, &targetPath))
                        deleteDir(srcPath);
                }
            }
        }else{
            adjustSymlinkPath(srcPath, tarDirPath);
            if (m_jobType == Copy){
                copyFile(srcPath, tarDirPath, false,  &targetPath);
            }else if (m_jobType == Move){
                if(m_isInSameDisk)
                {
                    if (!moveFile(srcPath, tarDirPath, &targetPath)) {
                        if(copyFile(srcPath, tarDirPath, true, &targetPath))
                            deleteFile(srcPath);
                    }
                }
                else
                {
                    if(copyFile(srcPath, tarDirPath, true, &targetPath))
                        deleteFile(srcPath);
                }
            }
        }

        if (!targetPath.isEmpty())
            list << DUrl::fromLocalFile(targetPath);
    }
    if(m_isJobAdded)
        jobRemoved();
    emit finished();
    qDebug() << "Do file operation is done" << m_jobDetail;

    return list;
}

void FileJob::doDelete(const DUrlList &files)
{
    qDebug() << "Do delete is started";
    for(int i = 0; i < files.size(); i++)
    {
        QUrl url = files.at(i);
        QFileInfo info(url.path());
        if (info.isFile() || info.isSymLink()){
            deleteFile(url.path());
        }else{
            if (!deleteDir(url.path())) {
                QProcess::execute("rm -r \"" + url.path().toUtf8() + "\"");
            }
        }
    }
    if(m_isJobAdded)
        jobRemoved();
    emit finished();
    qDebug() << "Do delete is done!";
}

DUrlList FileJob::doMoveToTrash(const DUrlList &files)
{
    QDir trashDir;
    DUrlList list;

    if(!trashDir.mkpath(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath))) {
        qDebug() << "mk" << DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) << "failed!";
        /// TODO

        return list;
    }

    if(!trashDir.mkpath(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath))) {
        qDebug() << "mk" << DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) << "failed!";
        /// TODO

        return list;
    }

    bool ok = true;

    for(int i = 0; i < files.size(); i++)
    {
        QUrl url = files.at(i);
        QDir dir(url.path());
        QString targetPath;

        if(dir.exists())
            ok = ok && moveDirToTrash(url.path(), &targetPath);
        else
            ok = ok && moveFileToTrash(url.path(), &targetPath);

        if (!targetPath.isEmpty())
            list << DUrl::fromLocalFile(targetPath);
    }
    if(m_isJobAdded)
        jobRemoved();

    emit finished();

    if (ok)
        qDebug() << "Move to Trash is done!";

    return list;
}

void FileJob::doTrashRestore(const QString &srcFilePath, const QString &tarFilePath)
{
//    qDebug() << srcFile << tarFile;
    qDebug() << "Do restore trash file is started";
    DUrlList files;
    files << QUrl::fromLocalFile(srcFilePath);
    m_totalSize = FileUtils::totalSize(files);
    jobPrepared();

    restoreTrashFile(srcFilePath, tarFilePath);

    if(m_isJobAdded)
        jobRemoved();
    emit finished();
    qDebug() << "Do restore trash file is done!";
}

void FileJob::paused()
{
    m_status = FileJob::Paused;
}

void FileJob::started()
{
    m_status = FileJob::Started;
}

void FileJob::cancelled()
{
    m_status = FileJob::Cancelled;
}

void FileJob::handleJobFinished()
{
    qDebug() << m_status;
    m_bytesCopied = m_totalSize;
    m_bytesPerSec = -1;
    m_isFinished = true;
    jobUpdated();
}

void FileJob::jobUpdated()
{
    if (m_isCheckingDisk){
        emit requestJobDataUpdated(m_jobDetail, m_checkDiskJobDataDetail);
    }

    QMap<QString, QString> jobDataDetail;
    if (!m_isFinished){

        qint64 currentMsec = m_timer.elapsed();

        m_factor = (currentMsec - m_lastMsec) / 1000;

        if (m_factor <= 0)
            return;

        m_bytesPerSec /= m_factor;

        if (m_bytesPerSec == 0){
            return;
        }

        if(m_bytesPerSec > 0)
        {
            int remainTime = (m_totalSize - m_bytesCopied) / m_bytesPerSec;

            if (remainTime < 60){
                jobDataDetail.insert("remainTime", tr("%1 s").arg(QString::number(remainTime)));
            }else if (remainTime >=60  && remainTime < 3600){
                int min = remainTime / 60;
                int second = remainTime % 60;
                jobDataDetail.insert("remainTime", tr("%1 m %2 s").arg(QString::number(min),
                                                                            QString::number(second)));
            }else if (remainTime >=3600  && remainTime < 86400){
                int hour = remainTime / 3600;
                int min = (remainTime % 3600) / 60;
                int second = (remainTime % 3600) % 60;
                jobDataDetail.insert("remainTime", tr("%1 h %2 m %3 s").arg(QString::number(hour),
                                                                                 QString::number(min),
                                                                                 QString::number(second)));
            }else{
                int day = remainTime / 86400;
                int left = remainTime % 86400;
                int hour = left / 3600;
                int min = (left % 3600) / 60;
                int second = (left % 3600) % 60;
                jobDataDetail.insert("remainTime", tr("%1 d %2 h %3 m %4 s").arg(QString::number(day),
                                                                                      QString::number(hour),
                                                                                      QString::number(min),
                                                                                      QString::number(second)));
            }
        }
    }
    QString speed;

    if (m_bytesCopied == m_totalSize){
        speed = QString("0 MB/s");
    }else{
        if(m_bytesPerSec > ONE_MB_SIZE)
        {
            m_bytesPerSec = m_bytesPerSec / ONE_MB_SIZE;
            speed = QString("%1 MB/s").arg(QString::number(m_bytesPerSec));
        }
        else
        {
            m_bytesPerSec = m_bytesPerSec / ONE_KB_SIZE;
            speed = QString("%1 KB/s").arg(QString::number(m_bytesPerSec));
        }
    }

    jobDataDetail.insert("speed", speed);
    jobDataDetail.insert("file", m_srcFileName);
    jobDataDetail.insert("progress", QString::number(m_bytesCopied * 100/ m_totalSize));
    jobDataDetail.insert("destination", m_tarDirName);
    m_progress = jobDataDetail.value("progress");
    emit requestJobDataUpdated(m_jobDetail, jobDataDetail);

    m_lastMsec = m_timer.elapsed();
    m_bytesPerSec = 0;
}

void FileJob::jobAdded()
{
    if(m_isJobAdded)
        return;
    emit requestJobAdded(m_jobDetail);
    m_isJobAdded = true;
}

void FileJob::jobRemoved()
{
    emit requestJobRemoved(m_jobDetail);
}

void FileJob::jobAborted()
{
    m_isAborted = true;
    emit requestAbortTask(m_jobDetail);
}

void FileJob::jobPrepared()
{
    m_bytesCopied = 0;
    m_bytesPerSec = 0;
    m_isFinished = false;
    m_timer.start();
    m_lastMsec = m_timer.elapsed();
}

void FileJob::jobConflicted()
{
    jobAdded();
    QMap<QString, QString> jobDataDetail;
    jobDataDetail.insert("remainTime", "");
    jobDataDetail.insert("speed", "");
    jobDataDetail.insert("file", m_srcFileName);
    jobDataDetail.insert("progress", m_progress);
    jobDataDetail.insert("destination", m_tarDirName);
    jobDataDetail.insert("sourcePath",m_srcPath);
    jobDataDetail.insert("targetPath", m_tarPath);
    emit requestJobDataUpdated(m_jobDetail, jobDataDetail);
    emit requestConflictDialogShowed(m_jobDetail);
    m_status = Paused;
}
bool FileJob::isAborted() const
{
    return m_isAborted;
}

void FileJob::setIsAborted(bool isAborted)
{
    m_isAborted = isAborted;
}

bool FileJob::copyFile(const QString &srcFile, const QString &tarDir, bool isMoved, QString *targetPath)
{
    if (m_isAborted)
        return false;
    if(m_applyToAll && m_status == FileJob::Cancelled){
        m_skipandApplyToAll = true;
    }else if(!m_applyToAll && m_status == FileJob::Cancelled){
        m_status = Started;
    }

    QFileInfo srcFileInfo(srcFile);
    QFileInfo tarDirInfo(tarDir);
    m_srcFileName = srcFileInfo.fileName();
    m_tarDirName = tarDirInfo.fileName();
    m_srcPath = srcFile;
    m_tarPath = tarDir + "/" + m_srcFileName;
    QFile from(srcFile);
    QFile to(m_tarPath);
    QFileInfo targetInfo(m_tarPath);
    m_status = Started;

    //We only check the conflict of the files when
    //they are not in the same folder
    bool isTargetExists = targetInfo.exists();

    if(srcFileInfo.absolutePath() != targetInfo.absolutePath()){
        if(isTargetExists && !m_applyToAll)
        {
            if (!isMoved){
                jobConflicted();
            }else{
                m_isReplaced = true;
            }
        }else if (isTargetExists && m_skipandApplyToAll){
            return false;
        }
    }

#ifdef SPLICE_CP
    loff_t in_off = 0;
    loff_t out_off = 0;
    int buf_size = 16 * 1024;
    qint64 len = 0;
    ssize_t err = -1;
    int in_fd = 0;
    int out_fd = 0;
#else
    char block[Data_Block_Size];
#endif

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if (isTargetExists){
                    if(!m_isReplaced)
                    {
                        m_tarPath = checkDuplicateName(m_tarPath);
                        to.setFileName(m_tarPath);
                    }else
                    {
                        if (targetInfo.isSymLink()){
                            QFile(m_tarPath).remove();
                        }else if (!targetInfo.isSymLink() && targetInfo.isDir()){
                            QDir(m_tarPath).removeRecursively();
                        }

                        if(!m_applyToAll)
                            m_isReplaced = false;
                    }
                }

                if(!from.open(QIODevice::ReadOnly))
                {
                    //Operation failed
                    qDebug() << srcFile << "isn't read only";
                    return false;
                }

                if(!to.open(QIODevice::WriteOnly))
                {
                    //Operation failed
                    qDebug() << to.fileName() << to.error() << to.errorString() << "isn't write only";
                    QString toFileName =  tarDir + "/" + DUrl::toPercentEncoding(m_srcFileName);
                    qDebug() << "toPercentEncoding" <<  toFileName;
                    to.setFileName(toFileName);
                    if (!to.open(QIODevice::WriteOnly)){
                        qDebug() << to.fileName() << to.error() <<to.errorString() << "isn't write only";
                        return false;
                    }
                }
                if (!DMimeDatabase::isGvfsFile(to.fileName())){
                    if (!to.setPermissions(from.permissions())){
                        qDebug() << "Set permissions from " << srcFile << "to" << m_tarPath << "failed";
                    };
                }else{
                    qWarning() << "Set permissions from file" << srcFile << "to vfs file" << m_tarPath << "failed";
                }
                m_status = Run;
 #ifdef SPLICE_CP
                in_fd = from.handle();
                out_fd = to.handle();
                len = srcFileInfo.size();
                posix_fadvise(from.handle(), 0, len, POSIX_FADV_SEQUENTIAL);
                posix_fadvise(from.handle(), 0, len, POSIX_FADV_WILLNEED);
 #endif
                break;
            }
            case FileJob::Run:
            {

#ifdef SPLICE_CP
                if(len <= 0)
                {
                    if ((m_totalSize - m_bytesCopied) <= 1){
                        m_bytesCopied = m_totalSize;
                    }

                    to.flush();
//                    fsync(out_fd);
                    from.close();
                    to.close();

                    if (targetPath)
                        *targetPath = m_tarPath;

                    return true;
                }

                if(buf_size > len)
                    buf_size = len;
                /*
                 * move to pipe buffer.
                 */
                err = splice(in_fd, &in_off, m_filedes[1], NULL, buf_size, SPLICE_F_MOVE);
                if(err < 0) {
                    qDebug() << "splice pipe0 fail";
                    return false;
                }

                if (err < buf_size) {
                    qDebug() << QString("copy ret %1 (need %2)").arg(err, buf_size);
                    buf_size = err;
                }
                /*
                 * move from pipe buffer to out_fd
                 */
                err = splice(m_filedes[0], NULL, out_fd, &out_off, buf_size, SPLICE_F_MOVE );
                if(err < 0) {
                    qDebug() << "splice pipe1 fail";
                    return false;
                }
                len -= buf_size;

                m_bytesCopied += buf_size;
                m_bytesPerSec += buf_size;

                if (!m_isInSameDisk){
                    if (m_bytesCopied % (Data_Flush_Size) == 0){
                        fsync(out_fd);
                    }
                }
#else
                if(from.atEnd())
                {
                    if ((m_totalSize - m_bytesCopied) <= 1){
                        m_bytesCopied = m_totalSize;
                    }
                    to.flush();
                    from.close();
                    to.close();

                    if (targetPath)
                        *targetPath = m_tarPath;

                    if (from.size() == to.size()){
                        return true;
                    }else{
                        qWarning() << m_srcPath << "size:" << from.size() << FileUtils::formatSize(from.size());
                        qWarning() << m_tarPath << "size:" << to.size() << FileUtils::formatSize(to.size());
                        return false;
                    }
                }
                to.waitForBytesWritten(-1);

                qint64 inBytes = from.read(block, Data_Block_Size);
                to.write(block, inBytes);
                m_bytesCopied += inBytes;
                m_bytesPerSec += inBytes;

                if (m_bytesCopied % (Data_Flush_Size) == 0){
                    to.flush();
                    to.close();
                    if(!to.open(QIODevice::WriteOnly | QIODevice::Append))
                    {
                        //Operation failed
                        qDebug() << tarDir << "isn't write only";
                        return false;
                    }
                }
#endif
                break;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                m_lastMsec = m_timer.elapsed();
                break;
            case FileJob::Cancelled:
                from.close();
                to.close();
                return false;
            default:
                from.close();
                to.close();
                return false;
         }

    }
    return false;
}

bool FileJob::copyDir(const QString &srcDir, const QString &tarDir, bool isMoved, QString *targetPath)
{
    DUrl srcUrl(srcDir);
    DUrl tarUrl(tarDir);
    qDebug() << srcUrl<< tarUrl << isMoved;
    if(DUrl::childrenList(tarUrl).contains(srcUrl) && !isMoved){
        emit requestCopyMoveToSelfDialogShowed(m_jobDetail);
        return false;
    }

    if (m_isAborted)
        return false;

    if(m_applyToAll && m_status == FileJob::Cancelled){
        m_skipandApplyToAll = true;
    }else if(!m_applyToAll && m_status == FileJob::Cancelled){
        m_status = Started;
    }
    QDir sourceDir(srcDir);
    QDir targetDir(tarDir);
    QFileInfo srcDirInfo(srcDir);
    m_srcPath = srcDir;
    m_srcFileName = srcDirInfo.fileName();
    m_tarDirName = targetDir.dirName();
    m_tarPath = tarDir + "/" + m_srcFileName;
    QFileInfo targetInfo(m_tarPath);

    m_status = Started;

    bool isTargetExists = targetInfo.exists();

    if(srcDirInfo.absolutePath() != targetInfo.absolutePath()){
        if(isTargetExists && !m_applyToAll)
        {
            if (!isMoved){
                jobConflicted();
            }else{
                m_isReplaced = true;
            }
        }else if (isTargetExists && m_skipandApplyToAll){
            return false;
        }
    }

    while(true)
    {
        switch(m_status)
        {
        case Started:
        {
            if (isTargetExists){
                if(!m_isReplaced)
                {
                    m_tarPath = checkDuplicateName(m_tarPath);
                    isTargetExists = false;
                }
                else
                {
                    if (targetInfo.isSymLink()){
                        QFile(m_tarPath).remove();
                    }else if (!targetInfo.isSymLink() && targetInfo.isFile()){
                        QFile(m_tarPath).remove();
                    }

                    targetDir.mkdir(m_tarPath);

                    if(!m_applyToAll)
                        m_isReplaced = false;
                }
            }

            if(!isTargetExists)
            {
                if(!targetDir.mkdir(m_tarPath))
                    return false;
            }
            targetDir.setPath(m_tarPath);

            if (!DMimeDatabase::isGvfsFile(targetDir.path())){
                bool isSetPermissionsSuccess = setDirPermissions(srcDir, targetDir.path());
                if (!isSetPermissionsSuccess){
                    qDebug() << "Set Permissions of "<< m_tarPath << "same as" <<  srcDir << "failed";
                    return false;
                }
            }
            m_status = Run;
            break;
        }
        case Run:
        {
            QDirIterator tmp_iterator(sourceDir.absolutePath(),
                                      QDir::AllEntries | QDir::System
                                      | QDir::NoDotAndDotDot | QDir::NoSymLinks
                                      | QDir::Hidden);

            while (tmp_iterator.hasNext()) {

                if (m_isAborted)
                    break;

                tmp_iterator.next();
                const QFileInfo fileInfo = tmp_iterator.fileInfo();

                if (fileInfo.isSymLink()){
                    handleSymlinkFile(fileInfo.filePath(), targetDir.absolutePath());
                }else if(!fileInfo.isSymLink() && fileInfo.isDir()){
                    if(!copyDir(fileInfo.filePath(), targetDir.absolutePath())){
                        qDebug() << "coye dir" << fileInfo.filePath() << "failed";
                    }
                }else
                {
                    if(!copyFile(fileInfo.filePath(), targetDir.absolutePath()))
                    {
                        qDebug() << "coye file" << fileInfo.filePath() << "failed";
                    }
                }
            }

            if (targetPath)
                *targetPath = targetDir.absolutePath();

            return true;
        }
        case Paused:
            QThread::msleep(100);
            break;
        case Cancelled:
            return false;
        default: break;
        }
    }

    if (targetPath)
        *targetPath = targetDir.absolutePath();

    return true;
}

bool FileJob::moveFile(const QString &srcFile, const QString &tarDir, QString *targetPath)
{
    return handleMoveJob(srcFile, tarDir, targetPath);
}

bool FileJob::moveDir(const QString &srcDir, const QString &tarDir, QString *targetPath)
{
    return handleMoveJob(srcDir, tarDir, targetPath);
}

bool FileJob::handleMoveJob(const QString &srcPath, const QString &tarDir, QString *targetPath)
{
    QFileInfo scrFileInfo(srcPath);

    if (scrFileInfo.isDir() && scrFileInfo.exists()){
        DUrl srcUrl(srcPath);
        DUrl tarUrl(tarDir);
        if(DUrl::childrenList(tarUrl).contains(srcUrl)){
            emit requestCopyMoveToSelfDialogShowed(m_jobDetail);
            /*copyDir/deleteDir will excute if return false;*/
            return true;
        }
    }

    if (m_isAborted)
        return false;

    if(m_applyToAll && m_status == FileJob::Cancelled){
        m_skipandApplyToAll = true;
    }else if(!m_applyToAll && m_status == FileJob::Cancelled){
        m_status = Started;
    }

    QDir to(tarDir);
    m_srcFileName = scrFileInfo.fileName();
    m_tarDirName = to.dirName();
    m_srcPath = srcPath;
    m_tarPath = tarDir;
    m_status = Started;

    bool isTargetExists = to.exists(m_srcFileName);

    //We only check the conflict of the files when
    //they are not in the same folder
    if(scrFileInfo.absolutePath()== tarDir && isTargetExists)
        return true;
    else{
        if(isTargetExists && !m_applyToAll)
        {
            jobConflicted();
        }else if (isTargetExists && m_skipandApplyToAll){
            return false;
        }

    }

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if(!m_isReplaced)
                {
                    m_tarPath = checkDuplicateName(m_tarPath + "/" + m_srcFileName);
                }
                else
                {
                    m_tarPath = m_tarPath + "/" + m_srcFileName;
                }
                m_status = Run;
                break;
            }
            case FileJob::Run:
            {
                if(m_isReplaced)
                {
                    QFileInfo tarInfo(m_tarPath);
                    if (tarInfo.isDir()){
                        QDir tarDir(m_tarPath);
                        if(tarDir.exists())
                            tarDir.removeRecursively();
                    }else{
                        QFile tarFile(m_tarPath);
                        if(tarFile.exists()){
                            QFile(m_tarPath).remove();
                        }
                    }
                }

                bool ok(false);
                if (scrFileInfo.isDir()){
                    ok = QDir(srcPath).rename(srcPath, m_tarPath);
                }else{
                    QFile srcFile(srcPath);
                    ok = srcFile.rename(m_tarPath);
                }
                if (ok && targetPath)
                    *targetPath = m_tarPath;

                if(!m_applyToAll)
                    m_isReplaced = false;
                return ok;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                break;
            case FileJob::Cancelled:
                return true;
            default:
                return false;
         }

    }
    return false;

}

bool FileJob::handleSymlinkFile(const QString &srcFile, const QString &tarDir, QString *targetPath)
{
    if (m_isAborted)
        return false;
    QDir to(tarDir);
    QFileInfo fromInfo(srcFile);
    m_srcFileName = fromInfo.fileName();
    m_tarDirName = to.dirName();
    m_srcPath = srcFile;
    m_tarPath = tarDir;
    m_status = Started;

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                m_tarPath = checkDuplicateName(m_tarPath + "/" + m_srcFileName);
                m_status = Run;
                break;
            }
            case FileJob::Run:
            {
                QFile targetFile(fromInfo.symLinkTarget());
                bool ok = targetFile.link(m_tarPath);

                if (ok){
                    if (m_jobType == Move){
                        QFile from(srcFile);
                        from.remove();
                    }
                    if (targetPath){
                        *targetPath = m_tarPath;
                    }
                }

                if(!m_applyToAll){
                    m_isReplaced = false;
                }

                return ok;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                break;
            case FileJob::Cancelled:
                return true;
            default:
                return false;
         }

    }
    return false;
}

bool FileJob::restoreTrashFile(const QString &srcFile, const QString &tarFile)
{
    QFile from(srcFile);
    QFile to(tarFile);

    QFileInfo toInfo(tarFile);
    m_srcFileName = toInfo.fileName();
    m_tarPath = toInfo.absoluteDir().path();
    m_tarDirName = toInfo.absoluteDir().dirName();
    m_status = Started;

    if(toInfo.exists())
    {
        jobConflicted();
    }else{
        bool result = from.rename(tarFile);

        if (!result) {
            result = (QProcess::execute("mv -T \"" + from.fileName().toUtf8() + "\" \"" + tarFile.toUtf8() + "\"") == 0);
        }

        return result;
    }

    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if(!m_isReplaced)
                {
                    m_srcPath = checkDuplicateName(m_tarPath + "/" + toInfo.fileName());
                }
                else
                {
                    m_srcPath = m_tarPath + "/" + toInfo.fileName();
                }
                m_status = Run;
                break;
            }
            case FileJob::Run:
            {
                if(m_isReplaced)
                {
                    if(to.exists()){
                        if (toInfo.isDir()){
                            bool result = QDir(tarFile).removeRecursively();

                            if (!result) {
                                result = QProcess::execute("rm -r \"" + tarFile.toUtf8() + "\"") == 0;
                            }
                        }else if (toInfo.isFile()){
                            to.remove();
//                            qDebug() << to.error() << to.errorString();
                        }
                    }
                }
                bool result = from.rename(m_srcPath);

                if (!result) {
                    result = (QProcess::execute("mv -T \"" + from.fileName().toUtf8() + "\" \"" + m_srcPath.toUtf8() + "\"") == 0);
                }
//                qDebug() << m_srcPath << from.error() << from.errorString();
                return result;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                break;
            case FileJob::Cancelled:
                return true;
            default:
                return false;
         }

    }
    return false;
}

bool FileJob::deleteFile(const QString &file)
{
    if(QFile::remove(file)){
        qDebug() << " delete file:" << file << "successfully";
        return true;
    }
    else
    {
        qDebug() << "unable to delete file:" << file;
        return false;
    }
}

bool FileJob::deleteDir(const QString &dir)
{
    if (m_status == FileJob::Cancelled) {
        emit result("cancelled");
        return false;
    }

    QDir sourceDir(dir);

    sourceDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System | QDir::AllDirs);

    QDirIterator iterator(sourceDir, QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        const QFileInfo &fileInfo = iterator.next();
        if (fileInfo.exists()){
            if (fileInfo.isFile() || fileInfo.isSymLink()) {
                if (!deleteFile(fileInfo.filePath())) {
                    qDebug() << "Unable to remove file:" << fileInfo.filePath();
                    emit error("Unable to remove file");
                    return false;
                }
            } else {
                qDebug() << fileInfo.filePath() << fileInfo.isDir() << fileInfo.isFile() << fileInfo.isSymLink();
                if (fileInfo.isDir()){
                    if (!deleteDir(fileInfo.filePath()))
                        return false;
                }
            }
        }
    }
    qDebug() << "delete dir:" <<sourceDir.path();
    if (!sourceDir.rmdir(QDir::toNativeSeparators(sourceDir.path()))) {
        qDebug() << "Unable to remove dir:" << sourceDir.path();
        emit("Unable to remove dir: " + sourceDir.path());

        return false;
    }

    return true;
}

bool FileJob::moveDirToTrash(const QString &dir, QString *targetPath)
{
    if(m_status == FileJob::Cancelled)
    {
        emit result("cancelled");
        return false;
    }
    QDir sourceDir(dir);


    QString baseName = getNotExistsTrashFileName(sourceDir.dirName());
    QString newName = m_trashLoc + "/files/" + baseName;
    QString delTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (!writeTrashInfo(baseName, dir, delTime))
        return false;

    if (!sourceDir.rename(sourceDir.path(), newName)) {
        if (QProcess::execute("mv -T \"" + sourceDir.path().toUtf8() + "\" \"" + newName.toUtf8() + "\"") != 0) {
            qDebug() << "Unable to trash dir:" << sourceDir.path();
            return false;
        }
    }

    if (targetPath)
        *targetPath = newName;

    return true;
}

QString FileJob::getNotExistsTrashFileName(const QString &fileName)
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

    while (QFile::exists(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + "/" + name + suffix)) {
        name = QCryptographicHash::hash(name, QCryptographicHash::Md5).toHex();
    }

    return QString::fromUtf8(name + suffix);
}

bool FileJob::moveFileToTrash(const QString &file, QString *targetPath)
{
    if(m_status == FileJob::Cancelled)
    {
        emit result("cancelled");
        return false;
    }

    QFile localFile(file);
    QString path = m_trashLoc + "/files/";
    QString baseName = getNotExistsTrashFileName(localFile.fileName());
    QString newName = path + baseName;
    QString delTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (!writeTrashInfo(baseName, file, delTime))
        return false;

    if (!localFile.rename(newName))
    {
        if (QProcess::execute("mv -T \"" + file.toUtf8() + "\" \"" + newName.toUtf8() + "\"") != 0) {
            //Todo: find reason
            qDebug() << "Unable to trash file:" << localFile.fileName();
            return false;
        }
    }

    if (targetPath)
        *targetPath = newName;

    return true;
}

bool FileJob::writeTrashInfo(const QString &fileBaseName, const QString &path, const QString &time)
{
    QFile metadata( m_trashLoc + "/info/" + fileBaseName + ".trashinfo" );

    if (!metadata.open( QIODevice::WriteOnly )) {
        qDebug() << metadata.fileName() << "file open error:" << metadata.errorString();

        return false;
    }

    QByteArray data;

    data.append("[Trash Info]\n");
    data.append("Path=").append(path.toUtf8().toPercentEncoding("/")).append("\n");
    data.append("DeletionDate=").append(time).append("\n");

    qint64 size = metadata.write(data);

    metadata.close();

    if (size < 0) {
        qDebug() << "write file " << metadata.fileName() << "error:" << metadata.errorString();
    }

    return size > 0;
}

bool FileJob::checkDiskSpaceAvailable(const DUrlList &files, const DUrl &destination)
{
//    UDiskDeviceInfoPointer info = deviceListener->getDeviceByPath(destination.path()); // get disk info from mount point
//    if(!info)
//        info = deviceListener->getDeviceByFilePath(destination.path()); // get disk infor from mount mount point sub path
    if (DMimeDatabase::isGvfsFile(destination.toLocalFile())){
        m_totalSize = FileUtils::totalSize(files);
        return true;
    }

    qint64 freeBytes;
    freeBytes = QStorageInfo(destination.toLocalFile()).bytesFree();

    m_isCheckingDisk = true;

    bool isInLimit = true;
    QMap<QString, QString> jobDataDetail;

    jobDataDetail.insert("status", "calculating");
    jobDataDetail.insert("file", files.first().fileName());
    jobDataDetail.insert("progress", m_progress);
    jobDataDetail.insert("destination", destination.fileName());

    m_checkDiskJobDataDetail = jobDataDetail;

    //calculate files's sizes
    m_totalSize = FileUtils::totalSize(files, freeBytes, isInLimit);

    jobDataDetail["status"] = "working";

    m_checkDiskJobDataDetail = jobDataDetail;

    if(!isInLimit)
        qDebug() << QString ("Can't copy or move files to target disk, disk free: %1").arg(FileUtils::formatSize(freeBytes));

    return isInLimit;
}
