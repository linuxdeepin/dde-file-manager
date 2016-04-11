#include "trashmanager.h"
#include "trashfileinfo.h"

#include "../app/global.h"

#include "../controllers/fileservices.h"

#include "../../filemonitor/filemonitor.h"

TrashManager::TrashManager(QObject *parent)
    : AbstractFileController(parent)
{
    FileServices::setFileUrlHandler(TRASH_SCHEME, "", this);
}

AbstractFileInfo *TrashManager::createFileInfo(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    if(url.scheme() != TRASH_SCHEME) {
        accepted = false;

        return Q_NULLPTR;
    }

    accepted = true;

    return new TrashFileInfo(url.toString());
}

const QList<AbstractFileInfo *> TrashManager::getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(filter)

    QUrl url(fileUrl);

    QList<AbstractFileInfo*> infolist;

    if(url.path() != "/" || url.scheme() != TRASH_SCHEME) {
        accepted = false;

        return infolist;
    }

    accepted = true;

    QString user = getenv("USER");
    QString trashPath = "/home/" + user + "/.local/share/Trash/files";
    QDir dir(trashPath);
    QList<AbstractFileInfo*> infoList;

    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(filter | QDir::NoDotAndDotDot);

        for(const QFileInfo fileInfo : fileInfoList) {
            infoList.append(new TrashFileInfo(QString(TRASH_SCHEME)
                                              + "://" + fileInfo.absoluteFilePath()));
        }
    }

    return infoList;
}

bool TrashManager::openFile(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    accepted = false;

    if(url.scheme() != TRASH_SCHEME) {
        return false;
    }

    accepted = true;

    return fileService->openFile("file://" + url.path());
}

bool TrashManager::addUrlMonitor(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    accepted = false;

    if(url.path() != "/" && url.scheme() != TRASH_SCHEME) {
        return false;
    }

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

    fileMonitor->addMonitorPath(trashPath);

    return true;
}

bool TrashManager::removeUrlMonitor(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    accepted = false;

    if(url.path() != "/" && url.scheme() != TRASH_SCHEME) {
        return false;
    }

    accepted = true;

    if(!fileMonitor)
        return true;

    QString user = getenv("USER");
    QString trashPath = "/home/" + user + "/.local/share/Trash/files";

    fileMonitor->removeMonitorPath(trashPath);
    fileMonitor->deleteLater();
    fileMonitor = Q_NULLPTR;

    return true;
}

void TrashManager::onFileCreated(const QString &filePath)
{
    emit childrenAdded(QString(TRASH_SCHEME) + "://" + filePath);
}

void TrashManager::onFileRemove(const QString &filePath)
{
    emit childrenRemoved(QString(TRASH_SCHEME) + "://" + filePath);
}

void TrashManager::onFileRename(const QString &oldName, const QString &newName)
{
    onFileCreated(newName);
    onFileRemove(oldName);
}
