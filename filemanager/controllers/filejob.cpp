#include "filejob.h"
#include <QFile>
#include <QThread>
#include <QDir>
#include <QDebug>
#include <QDateTime>

FileJob::FileJob(QObject *parent) : QObject(parent)
{
    m_status = FileJob::Started;
    QString user = getenv("USER");
    m_trashLoc = "/home/" + user + "/.local/share/Trash";
}

void FileJob::doCopy(const QString &source, const QString &destination)
{
    QDir srcDir(source);
    if(srcDir.exists())
    {
        copyDir(source, destination);
    }
    else
        copyFile(source, destination);
    emit finished();
}

void FileJob::doDelete(const QString &source)
{
    QDir dir(source);
    if(dir.exists())
        deleteDir(source);
    else
        deleteFile(source);
    emit finished();
}

void FileJob::doMoveToTrash(const QString &source)
{
    QDir dir(source);
    if(dir.exists())
        moveDirToTrash(source);
    else
        moveFileToTrash(source);
    emit finished();
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

bool FileJob::copyFile(const QString &srcFile, const QString &tarFile)
{
    QFile from(srcFile);
    if(!from.exists())
    {
        emit error("Source file doesn't exist!");
        return false;
    }
    QFile to(tarFile);
    from.open(QIODevice::ReadOnly);
    to.open(QIODevice::WriteOnly);
    qint64 index = 0;
    qint64 size = from.size();
    while(true)
    {
        switch(m_status)
        {
            case FileJob::Started:
            {
                if(from.atEnd())
                    return true;
                to.write(from.read(index));
                from.seek(index);
                to.seek(index);
                index += 1024 * 64;
                emit progressPercent((int)(index *100 / size));
                break;
            }
            case FileJob::Paused:
                QThread::msleep(10);
                break;
            case FileJob::Cancelled:
                to.close();
                emit result("cancelled");
                return false;
            default:
                emit error("unknown status");
                return false;
         }

    }
    return false;
}

bool FileJob::copyDir(const QString &srcPath, const QString &tarPath)
{
    if(m_status == FileJob::Cancelled)
    {
        emit result("cancelled");
        return false;
    }
    QDir sourceDir(srcPath);
    QDir targetDir(tarPath);
    if(!targetDir.exists())
    {
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir())
        {
            if(!copyDir(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName())))
                return false;
        }
        else
        {
            if(targetDir.exists(fileInfo.fileName()))
            {
                //TODO: rename or skip
            }
            else
            {
                if(!copyFile(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName())))
                {
                    return false;
                }
            }
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
