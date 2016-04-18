#include "trashmanager.h"
#include "trashfileinfo.h"
#include "fileservices.h"

#include "../../filemonitor/filemonitor.h"

TrashManager::TrashManager(QObject *parent)
    : AbstractFileController(parent)
{

}

AbstractFileInfo *TrashManager::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return new TrashFileInfo(fileUrl);
}

const QList<AbstractFileInfo *> TrashManager::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(filter)

    accepted = true;

    QString user = getenv("USER");
    QString trashPath = "/home/" + user + "/.local/share/Trash/files" + fileUrl.path();
    QDir dir(trashPath);
    QList<AbstractFileInfo*> infoList;

    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(filter | QDir::NoDotAndDotDot);

        QString path = fileUrl.path();

        if(path != "/")
            path.append('/');

        for(const QFileInfo fileInfo : fileInfoList) {
            infoList.append(new TrashFileInfo(DUrl::fromTrashFile(path + fileInfo.fileName())));
        }
    }

    return infoList;
}

bool TrashManager::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->openFile(DUrl::fromLocalFile(fileUrl.path()));
}

bool TrashManager::addUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    if(!fileMonitor) {
        fileMonitor = new FileMonitor(const_cast<TrashManager*>(this));

        connect(fileMonitor, &FileMonitor::fileCreated,
                this, &TrashManager::onFileCreated);
        connect(fileMonitor, &FileMonitor::fileDeleted,
                this, &TrashManager::onFileRemove);
    }

    QString user = getenv("USER");
    QString trashPath = "/home/" + user + "/.local/share/Trash/files";

    fileMonitor->addMonitorPath(trashPath + fileUrl.path());

    return true;
}

bool TrashManager::removeUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    if(!fileMonitor)
        return true;

    QString user = getenv("USER");
    QString trashPath = "/home/" + user + "/.local/share/Trash/files";

    fileMonitor->removeMonitorPath(trashPath + fileUrl.path());
    fileMonitor->deleteLater();
    fileMonitor = Q_NULLPTR;

    return true;
}

void TrashManager::onFileCreated(const QString &filePath)
{
    emit childrenAdded(DUrl::fromTrashFile(filePath));
}

void TrashManager::onFileRemove(const QString &filePath)
{
    emit childrenRemoved(DUrl::fromTrashFile(filePath));
}
