#include "trashmanager.h"
#include "fileservices.h"

#include "models/fileinfo.h"
#include "models/trashfileinfo.h"

#include "app/global.h"
#include "app/filesignalmanager.h"
#include "app/fmevent.h"

#include "filemonitor/filemonitor.h"
#include "interfaces/dfmstandardpaths.h"
#include "widgets/singleton.h"

#include <QDebug>
#include <QUrlQuery>

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
    const AbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
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

const AbstractFileInfoPointer TrashDirIterator::fileInfo() const
{
    return FileServices::instance()->createFileInfo(DUrl::fromTrashFile(filePath()));
}

QString TrashDirIterator::path() const
{
    return iterator->path().remove(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));
}

TrashManager *firstMe = Q_NULLPTR;

TrashManager::TrashManager(QObject *parent)
    : AbstractFileController(parent)
{
    if(!firstMe) {
        firstMe = this;

        connect(fileSignalManager, &FileSignalManager::requestRestoreTrashFile,
                this, &TrashManager::restoreTrashFile);
        connect(fileSignalManager, &FileSignalManager::requestRestoreAllTrashFile,
                this, &TrashManager::restoreAllTrashFile);
    }

    fileMonitor = new FileMonitor(const_cast<TrashManager*>(this));

    connect(fileMonitor, &FileMonitor::fileCreated,
            this, &TrashManager::onFileCreated);
    connect(fileMonitor, &FileMonitor::fileDeleted,
            this, &TrashManager::onFileRemove);
}

const AbstractFileInfoPointer TrashManager::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return AbstractFileInfoPointer(new TrashFileInfo(fileUrl));
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

    const AbstractFileInfoPointer &file = createFileInfo(fileUrl, accepted);

    if (!file->exists())
        return false;

    DUrl parentUrl = file->parentUrl();
    QUrlQuery query;

    query.addQueryItem("selectUrl", fileUrl.toString());
    parentUrl.setQuery(query);

    fileService->openNewWindow(parentUrl);

    return true;
}

bool TrashManager::addUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    fileMonitor->addMonitorPath(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + fileUrl.path());

    return true;
}

bool TrashManager::removeUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    fileMonitor->removeMonitorPath(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + fileUrl.path());

    return true;
}

bool TrashManager::copyFiles(const DUrlList &urlList, bool &accepted) const
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

    fileService->copyFiles(localList);

    return true;
}

DUrlList TrashManager::pasteFile(AbstractFileController::PasteType type, const DUrlList &urlList,
                                 const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(event)

    accepted = (type == AbstractFileController::CutType);

    if (!accepted)
        return DUrlList();

    return fileService->moveToTrashSync(urlList);
}


bool TrashManager::deleteFiles(const DUrlList &urlList, const FMEvent &event, bool &accepted) const
{
    accepted = true;

    DUrlList localList;

    for(const DUrl &url : urlList) {
        const QString &path = url.path();

        localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + path);

        if (path.lastIndexOf('/') == 0) {
            localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + path + ".trashinfo");
        }
    }

    fileService->deleteFiles(localList, event);

    return true;
}

const DDirIteratorPointer TrashManager::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                          bool &accepted) const
{
    accepted = true;

    return DDirIteratorPointer(new TrashDirIterator(fileUrl, nameFilters, filters, flags));
}

bool TrashManager::restoreTrashFile(const DUrlList &fileUrl, const FMEvent &event) const
{
    bool ok = true;

    for(const DUrl &url : fileUrl) {
        TrashFileInfo info;
        info.setUrl(url);
        const_cast<FMEvent &>(event) << url;
        info.restore(event);
    }

    return ok;
}

bool TrashManager::restoreAllTrashFile(const FMEvent &event)
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

void TrashManager::onFileCreated(const QString &filePath) const
{
    QString path;

    if (QString(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath)).startsWith(filePath)) {
        path = "/";
    } else {
        path = filePath.mid((DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath)).size());

        if (path.isEmpty())
            return;
    }

    emit childrenAdded(DUrl::fromTrashFile(path));
}

void TrashManager::onFileRemove(const QString &filePath) const
{
    QString path;

    if (QString(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath)).startsWith(filePath)) {
        path = "/";
    } else {
        path = filePath.mid((DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath)).size());

        if (path.isEmpty())
            return;
    }

    emit childrenRemoved(DUrl::fromTrashFile(path));
}
