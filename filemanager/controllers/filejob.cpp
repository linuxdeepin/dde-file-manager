#include "filejob.h"
#include <QFile>
#include <QThread>
#include <QDir>

FileJob::FileJob(QObject *parent) : QObject(parent)
{
    m_status = FileJob::Started;
}

void FileJob::doJob(const QString &source, const QString &destination)
{
    QDir srcDir(source);
    if(srcDir.exists())
    {
        copyDir(source, destination);
    }
    else
        copyFile(source, destination);
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
