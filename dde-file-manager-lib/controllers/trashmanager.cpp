#include "trashmanager.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dfileproxywatcher.h"
#include "dfileinfo.h"
#include "models/trashfileinfo.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"

#include "filemonitor/filemonitor.h"
#include "interfaces/dfmstandardpaths.h"
#include "widgets/singleton.h"

#include <QDebug>
#include <QUrlQuery>
#include <QTimer>
#include <QCoreApplication>
#include <QThread>

class TrashDirIterator : public DDirIterator
{
public:
    TrashDirIterator(const DUrl &url,
                     const QStringList &nameFilters,
                     QDir::Filters filter,
                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    QString filePath() const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;

private:
    QDirIterator *iterator;
};

TrashDirIterator::TrashDirIterator(const DUrl &url, const QStringList &nameFilters,
                                   QDir::Filters filter, QDirIterator::IteratorFlags flags)
    : DDirIterator()
{
    iterator = new QDirIterator(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + url.path(), nameFilters, filter, flags);
}

DUrl TrashDirIterator::next()
{
    return DUrl::fromTrashFile(DUrl::fromLocalFile(iterator->next()).path().remove(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath)));
}

bool TrashDirIterator::hasNext() const
{
    return iterator->hasNext();
}

QString TrashDirIterator::fileName() const
{
    return iterator->fileName();
}

QString TrashDirIterator::filePath() const
{
    return iterator->filePath().remove(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));
}

const DAbstractFileInfoPointer TrashDirIterator::fileInfo() const
{
    return DFileService::instance()->createFileInfo(DUrl::fromTrashFile(filePath()));
}

QString TrashDirIterator::path() const
{
    return iterator->path().remove(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));
}

TrashManager *firstMe = Q_NULLPTR;

TrashManager::TrashManager(QObject *parent)
    : DAbstractFileController(parent)
{
    if (!firstMe) {
        firstMe = this;

        connect(fileSignalManager, &FileSignalManager::requestRestoreTrashFile,
                this, &TrashManager::restoreTrashFile);
        connect(fileSignalManager, &FileSignalManager::requestRestoreAllTrashFile,
                this, &TrashManager::restoreAllTrashFile);
    }
}

const DAbstractFileInfoPointer TrashManager::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return DAbstractFileInfoPointer(new TrashFileInfo(fileUrl));
}

bool TrashManager::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    qDebug() << "trash open action is disable : " << fileUrl;
//    return FileServices::instance()->openFile(DUrl::fromLocalFile(TRASHFILEPATH + fileUrl.path()));
    return true;
}

bool TrashManager::openFileLocation(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    const DAbstractFileInfoPointer &file = createFileInfo(fileUrl, accepted);

    if (!file->exists())
        return false;

    DUrl parentUrl = file->parentUrl();
    QUrlQuery query;

    query.addQueryItem("selectUrl", fileUrl.toString());
    parentUrl.setQuery(query);

    fileService->openNewWindow(parentUrl);

    return true;
}

DUrlList TrashManager::moveToTrash(const DFMEvent &event, bool &accepted) const
{
    accepted = true;

    TIMER_SINGLESHOT_CONNECT_TYPE(this, 0, fileService->deleteFiles(event), Qt::AutoConnection, event);

    return DUrlList();
}

bool TrashManager::copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    DUrlList localList;

    for(const DUrl &url : urlList) {
        const QString &path = url.path();

        localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + path);

        if(path.lastIndexOf('/') > 0) {
            localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + path);
        }
    }

    fileService->copyFilesToClipboard(localList);

    return true;
}

DUrlList TrashManager::pasteFile(DAbstractFileController::PasteType type, const DUrl &targetUrl,
                                 const DFMEvent &event, bool &accepted) const
{
    Q_UNUSED(type)

    accepted = /*(type == DAbstractFileController::CutType)*/targetUrl == DUrl::fromTrashFile("/");

    if (!accepted || event.fileUrlList().isEmpty())
        return DUrlList();

    const_cast<DFMEvent&>(event) << event.fileUrlList().first();

    return fileService->moveToTrashSync(event);
}


bool TrashManager::deleteFiles(const DFMEvent &event, bool &accepted) const
{
    accepted = true;

    DUrlList localList;

    for(const DUrl &url : event.fileUrlList()) {
        if (url.isTrashFile() && url.path() == "/") {
            cleanTrash(event);
            return true;
        }

        const QString &path = url.path();

        localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + path);

        if (path.lastIndexOf('/') == 0) {
            localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + path + ".trashinfo");
        }
    }

    const_cast<DFMEvent&>(event) << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));
    const_cast<DFMEvent&>(event) << localList;
    fileService->deleteFilesSync(event);

    return true;
}

const DDirIteratorPointer TrashManager::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                          bool &accepted) const
{
    accepted = true;

    return DDirIteratorPointer(new TrashDirIterator(fileUrl, nameFilters, filters, flags));
}

namespace TrashManagerPrivate {
DUrl localToTrash(const DUrl &url)
{
    const QString &localPath = url.toLocalFile();
    const QString &trashPath = DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath);

    if (!localPath.startsWith(trashPath))
        return DUrl();

    DUrl u = DUrl::fromTrashFile(localPath.mid(trashPath.length()));

    if (u.path().isEmpty())
        u.setPath("/");

    return u;
}
QString trashToLocal(const DUrl &url)
{
    return DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + url.path();
}
}

DAbstractFileWatcher *TrashManager::createFileWatcher(const DUrl &fileUrl, QObject *parent, bool &accepted) const
{
    accepted = true;

    return new DFileProxyWatcher(fileUrl, new DFileWatcher(TrashManagerPrivate::trashToLocal(fileUrl)),
                                 TrashManagerPrivate::localToTrash, parent);
}

bool TrashManager::restoreTrashFile(const DUrlList &fileUrl, const DFMEvent &event) const
{
    bool ok = true;

    for(const DUrl &url : fileUrl) {
        TrashFileInfo info(url);

        const_cast<DFMEvent &>(event) << url;
        info.restore(event);
    }

    return ok;
}

bool TrashManager::restoreAllTrashFile(const DFMEvent &event)
{

    DUrl fileUrl = event.fileUrlList().at(0);
    const QString &path = fileUrl.path();

    QDir dir(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + path);
    DUrlList urlList;
    if(dir.exists()) {
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden);
        for(const QString name : entryList) {
            urlList << DUrl::fromTrashFile(QString("/%1").arg(name));
        }

        restoreTrashFile(urlList, event);
    }
    return true;
}

void TrashManager::cleanTrash(const DFMEvent &event) const
{
    DUrlList list;
    list << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath))
         << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));

    const_cast<DFMEvent&>(event) << list.last();
    const_cast<DFMEvent&>(event) << list;
    fileService->deleteFilesSync(event);
}

bool TrashManager::isEmpty()
{
    QDir dir(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));
    QStringList entryList = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden);
    if (dir.exists() && entryList.count() == 0){
        return true;
    }
    if (!dir.exists()){
        return true;
    }
    return false;
}
