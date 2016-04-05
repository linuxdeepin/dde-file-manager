#include "trashmanager.h"
#include "fileinfo.h"

TrashManager::TrashManager(QObject *parent)
{
    Q_UNUSED(parent)
    load();
}

void TrashManager::load()
{
    QString user = getenv("USER");
    QString trashPath = "/home/" + user + "/.local/share/Trash/files";
    QDir dir(trashPath);
    if(dir.exists())
    {
        QFileInfoList fileInfoList = dir.entryInfoList();
        foreach(QFileInfo fileInfo, fileInfoList)
        {
            if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                continue;

            m_fileInfos.append(new FileInfo(fileInfo));
        }
    }
}

void TrashManager::save()
{

}

void TrashManager::fetchFileInformation(const QString &url, int filter)
{
    qDebug() << m_fileInfos.size();
    emit updates(url, m_fileInfos);
}

void TrashManager::trashUpdated()
{
    qDeleteAll(m_fileInfos);
    m_fileInfos.clear();
    load();
}
