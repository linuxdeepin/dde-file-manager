#include "trashmanager.h"
#include "trashfileinfo.h"
#include "fileservices.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"

#include "../../filemonitor/filemonitor.h"

TrashManager *firstMe = Q_NULLPTR;

TrashManager::TrashManager(QObject *parent)
    : AbstractFileController(parent)
{
    if(!firstMe) {
        firstMe = this;

        connect(fileSignalManager, &FileSignalManager::requestRestoreTrashFile,
                this, &TrashManager::restoreTrashFile);
    }

    fileMonitor = new FileMonitor(const_cast<TrashManager*>(this));

    connect(fileMonitor, &FileMonitor::fileCreated,
            this, &TrashManager::onFileCreated);
    connect(fileMonitor, &FileMonitor::fileDeleted,
            this, &TrashManager::onFileRemove);
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

    const QString &path = fileUrl.path();

    QDir dir(TRASHFILEPATH + path);
    QList<AbstractFileInfo*> infoList;

    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(filter | QDir::NoDotAndDotDot);

        for(const QFileInfo fileInfo : fileInfoList) {
            infoList.append(new TrashFileInfo(DUrl::fromTrashFile(fileInfo.absoluteFilePath().mid((TRASHFILEPATH).size()))));
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

    fileMonitor->addMonitorPath(TRASHFILEPATH + fileUrl.path());

    return true;
}

bool TrashManager::removeUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    fileMonitor->removeMonitorPath(TRASHFILEPATH + fileUrl.path());

    return true;
}

bool TrashManager::restoreTrashFile(const DUrlList &fileUrl) const
{
    bool ok = true;

    TrashFileInfo info;

    for(const DUrl &url : fileUrl) {
        info.setUrl(url);
        ok = ok && info.restore();
    }

    return ok;
}

void TrashManager::onFileCreated(const QString &filePath) const
{
    emit childrenAdded(DUrl::fromTrashFile(filePath.mid((TRASHFILEPATH).size())));
}

void TrashManager::onFileRemove(const QString &filePath) const
{
    emit childrenRemoved(DUrl::fromTrashFile(filePath.mid((TRASHFILEPATH).size())));
}
