#include "filejob.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../shutil/fileutils.h"
#include <QFile>
#include <QThread>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>

void FileJob::setStatus(FileJob::Status status)
{
    m_status = status;
}

FileJob::FileJob(QObject *parent) : QObject(parent)
{
    m_status = FileJob::Started;
    QString user = getenv("USER");
    m_trashLoc = "/home/" + user + "/.local/share/Trash";
    m_id = QDateTime::currentDateTime().toString();
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
    while (file.exists())
    {
        if(num == 1)
        {
            if(startInfo.isDir())
                destUrl = QString("%1/%2(copy)").arg(startInfo.absolutePath()).
                        arg(startInfo.fileName());
            else
                destUrl = QString("%1/%2(copy).%3").arg(startInfo.absolutePath()).
                        arg(startInfo.baseName()).arg(startInfo.completeSuffix());
        }
        else
        {
            if(startInfo.isDir())
                destUrl = QString("%1/%2(copy %3)").arg(startInfo.absolutePath()).
                        arg(startInfo.fileName()).arg(num);
            else
                destUrl = QString("%1/%2(copy %3).%4").arg(startInfo.absolutePath()).
                        arg(startInfo.baseName()).arg(num).arg(startInfo.completeSuffix());
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

void FileJob::doCopy(const QList<QUrl> &files, const QString &destination)
{
    //pre-calculate total size
    m_totalSize = FileUtils::totalSize(files);
    jobPrepared();
    for(int i = 0; i < files.size(); i++)
    {
        QUrl url = files.at(i);
        QDir srcDir(url.toLocalFile());
        if(srcDir.exists())
        {
            copyDir(url.toLocalFile(), destination);
        }
        else
            copyFile(url.toLocalFile(), destination);
    }
    if(m_isJobAdded)
        jobRemoved();
    emit finished();
}

void FileJob::doDelete(const QList<QUrl> &files)
{
    for(int i = 0; i < files.size(); i++)
    {
        QUrl url = files.at(i);
        QDir dir(url.path());
        if(dir.exists())
            deleteDir(url.path());
        else
            deleteFile(url.path());
    }
    emit finished();
}

void FileJob::doMoveToTrash(const QList<QUrl> &files)
{
    for(int i = 0; i < files.size(); i++)
    {
        QUrl url = files.at(i);
        QDir dir(url.path());
        if(dir.exists())
            moveDirToTrash(url.path());
        else
            moveFileToTrash(url.path());
    }
    emit finished();
}

void FileJob::doCut(const QList<QUrl> &files, const QString &destination)
{
    Q_UNUSED(files)
    Q_UNUSED(destination)
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

void FileJob::jobUpdated()
{
    QMap<QString, QString> jobDataDetail;
    if(m_bytesPerSec > 0)
    {
        int remainTime = (m_totalSize - m_bytesCopied) / m_bytesPerSec;
        jobDataDetail.insert("remainTime", QString("%1 s").arg(QString::number(remainTime)));
    }
    QString speed;
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
    jobDataDetail.insert("speed", speed);
    jobDataDetail.insert("file", m_srcFileName);
    jobDataDetail.insert("progress", QString::number(m_bytesCopied * 100/ m_totalSize));
    jobDataDetail.insert("destination", m_tarFileName);
    emit fileSignalManager->jobDataUpdated(m_jobDetail, jobDataDetail);
}

void FileJob::jobAdded()
{
    m_jobDetail.insert("jobId", m_id);
    m_jobDetail.insert("type", "copy");
    emit fileSignalManager->jobAdded(m_jobDetail);
    m_isJobAdded = true;
}

void FileJob::jobRemoved()
{
    emit fileSignalManager->jobRemoved(m_jobDetail);
}

void FileJob::jobAborted()
{
    emit fileSignalManager->abortTask(m_jobDetail);
}

void FileJob::jobPrepared()
{
    m_bytesCopied = 0;
    m_bytesPerSec = 0;
    m_timer.start();
    lastMsec = m_timer.elapsed();
    currentMsec = m_timer.elapsed();
}

void FileJob::jobConflicted()
{
    jobAdded();
    QMap<QString, QString> jobDataDetail;
    jobDataDetail.insert("remainTime", "");
    jobDataDetail.insert("speed", "");
    jobDataDetail.insert("file", m_srcFileName);
    jobDataDetail.insert("progress", "");
    jobDataDetail.insert("destination", m_tarFileName);
    emit fileSignalManager->jobDataUpdated(m_jobDetail, jobDataDetail);
    emit fileSignalManager->conflictDialogShowed(m_jobDetail);
    m_status = Paused;
}

bool FileJob::copyFile(const QString &srcFile, const QString &tarDir)
{
    QFile from(srcFile);   
    QFileInfo sf(srcFile);
    QFileInfo tf(tarDir);
    m_srcFileName = sf.fileName();
    m_tarFileName = tf.fileName();
    m_srcPath = srcFile;
    m_tarPath = tarDir + "/" + m_srcFileName;
    QFile to(tarDir + "/" + m_srcFileName);
    m_status = Started;

    //We only check the conflict of the files when
    //they are not in the same folder
    if(sf.absolutePath() != tf.absoluteFilePath())
        if(to.exists() && !m_applyToAll)
        {
            jobConflicted();
        }

    bool isGreater = false;
    char block[DATA_BLOCK_SIZE];
    qint64 thres = 0;
    bool startToDisplay = false;
    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                m_tarPath = checkDuplicateName(m_tarPath);
                if(!from.open(QIODevice::ReadOnly))
                {
                    //Operation failed
                    return false;
                }
                to.setFileName(m_tarPath);
                if(!to.open(QIODevice::WriteOnly))
                {
                    //Operation failed
                    return false;
                }
                m_status = Run;
                break;
            }
            case FileJob::Run:
            {
                if(from.atEnd())
                {
                    jobUpdated();
                    return true;
                }
                qint64 inBytes = from.read(block, DATA_BLOCK_SIZE);
                to.write(block, inBytes);
                m_bytesCopied += inBytes;
                m_bytesPerSec += inBytes;
                currentMsec = m_timer.elapsed();
                if(startToDisplay)
                {
                    if(isGreater)
                    {
                        if(currentMsec - lastMsec > MSEC_FOR_DISPLAY)
                        {
                            m_factor = (currentMsec - lastMsec);
                            m_factor /= 1000;
                            m_bytesPerSec /= m_factor;
                            jobUpdated();
                            lastMsec = m_timer.elapsed();
                            m_bytesPerSec = 0;
                        }
                    }
                    else
                    {
                        if(m_bytesPerSec > thres)
                        {
                            m_factor = (currentMsec - lastMsec);
                            m_factor /= 1000;

                            if(m_factor)
                                m_bytesPerSec /= m_factor;
                            else
                                m_bytesPerSec = 0;
                            jobUpdated();
                            lastMsec = m_timer.elapsed();
                            m_bytesPerSec = 0;
                        }
                    }
                    break;
                }

                if(currentMsec - lastMsec > MSEC_FOR_DISPLAY && !startToDisplay)
                {
                    startToDisplay = true;
                    if(!m_isJobAdded)
                        jobAdded();

                    if(m_totalSize / m_bytesCopied > TRANSFER_RATE)
                        isGreater = true;
                    else
                    {
                        thres = (m_totalSize - m_bytesCopied) / TRANSFER_RATE;
                        m_bytesPerSec = 0;
                    }
                }
                break;
            }
            case FileJob::Paused:
                QThread::msleep(100);
                lastMsec = m_timer.elapsed();
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

bool FileJob::copyDir(const QString &srcPath, const QString &tarPath)
{
    if(m_status == FileJob::Cancelled)
    {
        return false;
    }
    QDir sourceDir(srcPath);
    QDir targetDir(tarPath + "/" + sourceDir.dirName());
    QFileInfo sf(srcPath);
    QFileInfo tf(tarPath + "/" + sourceDir.dirName());
    m_srcFileName = sf.fileName();
    m_tarFileName = tf.dir().dirName();
    m_srcPath = srcPath;
    m_tarPath = targetDir.absolutePath();
    m_status = Started;

    if(targetDir.exists())
    {
        jobConflicted();
    }
    while(true)
    {
        switch(m_status)
        {
        case Started:
        {
            m_tarPath = checkDuplicateName(m_tarPath);
            targetDir.setPath(m_tarPath);
            if(!targetDir.exists())
            {
                if(!targetDir.mkdir(targetDir.absolutePath()))
                    return false;
            }
            m_status = Run;
            break;
        }
        case Run:
        {
            QFileInfoList fileInfoList = sourceDir.entryInfoList();
            foreach(QFileInfo fileInfo, fileInfoList)
            {
                if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                    continue;

                if(fileInfo.isDir())
                {
                    if(!copyDir(fileInfo.filePath(), targetDir.absolutePath()))
                        return false;
                }
                else
                {
                    if(!copyFile(fileInfo.filePath(), targetDir.absolutePath()))
                    {
                        return false;
                    }
                }
            }
            return true;
            break;
        }
        case Paused:
            QThread::msleep(100);
            break;
        case Cancelled:
            return false;
        }
    }
    return true;
}

bool FileJob::deleteFile(const QString &file)
{
    if(QFile::remove(file))
        return true;
    else
    {
        qDebug() << "unable to delete file:" << file;
        return false;
    }
}

bool FileJob::deleteDir(const QString &dir)
{
    if(m_status == FileJob::Cancelled)
    {
        emit result("cancelled");
        return false;
    }
    QDir sourceDir(dir);
    QFileInfoList fileInfoList = sourceDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir() || fileInfo.isSymLink())
        {
            if(!deleteDir(fileInfo.filePath()))
                return false;
        }
        else
        {
            if(!deleteFile(fileInfo.filePath()))
            {
                emit error("Unable to remove file");
                return false;
            }
        }
    }

    if(!sourceDir.rmdir(QDir::toNativeSeparators(sourceDir.path())))
    {
        qDebug() << "Unable to remove dir:" << sourceDir.path();
        emit("Unable to remove dir: " + sourceDir.path());
        return false;
    }
    return true;
}

bool FileJob::moveDirToTrash(const QString &dir)
{
    if(m_status == FileJob::Cancelled)
    {
        emit result("cancelled");
        return false;
    }
    QDir sourceDir(dir);

    QString oldName = sourceDir.dirName();
    QString newName = m_trashLoc + "/files/" + baseName(sourceDir.dirName());
    QString delTime = QDateTime::currentDateTime().toString( "yyyyMMddThh:mm:ss" );

    QDir tempDir(newName);
    if(tempDir.exists())
        newName += delTime;
    if(!sourceDir.rename(sourceDir.path(), newName))
    {
        qDebug() << "Unable to trash dir:" << sourceDir.path();
        return false;
    }
    writeTrashInfo(oldName, dir, delTime);
    return true;
}

bool FileJob::moveFileToTrash(const QString &file)
{
    if(m_status == FileJob::Cancelled)
    {
        emit result("cancelled");
        return false;
    }
    QFile localFile(file);
    QString oldName = localFile.fileName();
    QString newName = m_trashLoc + "/files/" + baseName(localFile.fileName());
    QString delTime = QDateTime::currentDateTime().toString( "yyyyMMddThh:mm:ss" );
    if(QFile::exists(newName))
        newName += delTime;
    if(!localFile.rename(newName))
    {
        //Todo: find reason
        qDebug() << "Unable to trash file:" << localFile.fileName();
        return false;
    }
    writeTrashInfo(oldName, file, delTime);
    return true;
}

void FileJob::writeTrashInfo(const QString &name, const QString &path, const QString &time)
{
    QFile metadata( m_trashLoc + "/info/" + baseName(name) + ".trashinfo" );
    metadata.open( QIODevice::WriteOnly );
    metadata.write(
        QString(
            "[Trash Info]\n"
            "Path=%1\n"
            "DeletionDate=%2\n"
        ).arg( path ).arg( time ).toLocal8Bit()
    );
    metadata.close();
}

QString FileJob::baseName( QString path )
{
    while( path.contains( "//" ) )
        path = path.replace( "//", "/" );

    if ( path.endsWith( "/" ) )
        path.chop( 1 );

    char *dupPath = strdup( path.toLocal8Bit().data() );
    QString basePth = QString( basename( dupPath ) );
    free( dupPath );

    return basePth;
};
