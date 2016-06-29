#include "trashmanager.h"
#include "trashfileinfo.h"
#include "fileservices.h"
#include "fileinfo.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"

#include "../../filemonitor/filemonitor.h"

class TrashDirIterator : public DDirIterator
{
public:
    TrashDirIterator(const DUrl &url,
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

TrashDirIterator::TrashDirIterator(const DUrl &url, QDir::Filters filter,
                                   QDirIterator::IteratorFlags flags)
    : DDirIterator()
{
    iterator = new QDirIterator(TRASHFILEPATH + url.path(), filter, flags);
}

DUrl TrashDirIterator::next()
{
    return DUrl::fromTrashFile(DUrl::fromLocalFile(iterator->next()).path().remove(TRASHFILEPATH));
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
    return iterator->filePath().remove(TRASHFILEPATH);
}

const AbstractFileInfoPointer TrashDirIterator::fileInfo() const
{
    return FileServices::instance()->createFileInfo(DUrl::fromTrashFile(filePath()));
}

QString TrashDirIterator::path() const
{
    return iterator->path().remove(TRASHFILEPATH);
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

const QList<AbstractFileInfoPointer> TrashManager::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(filter)

    accepted = true;

    const QString &path = fileUrl.path();

    QDir dir(TRASHFILEPATH + path);
    QList<AbstractFileInfoPointer> infoList;

    if(dir.exists()) {
        QFileInfoList fileInfoList = dir.entryInfoList(filter | QDir::NoDotAndDotDot | QDir::System);

        for(const QFileInfo fileInfo : fileInfoList) {
            const DUrl &fileUrl = DUrl::fromTrashFile(fileInfo.absoluteFilePath().mid((TRASHFILEPATH).size()));

            infoList.append(AbstractFileInfoPointer(new TrashFileInfo(fileUrl)));
        }
    }

    return infoList;
}

bool TrashManager::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;
    qDebug() << "trash open action is disable : " << fileUrl;
//    return FileServices::instance()->openFile(DUrl::fromLocalFile(TRASHFILEPATH + fileUrl.path()));
    return true;
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

bool TrashManager::copyFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    DUrlList localList;

    for(const DUrl &url : urlList) {
        const QString &path = url.path();

        localList << DUrl::fromLocalFile(TRASHFILEPATH + path);

        if(path.lastIndexOf('/') > 0) {
            localList << DUrl::fromLocalFile(TRASHINFOPATH + path);
        }
    }

    fileService->copyFiles(localList);

    return true;
}


bool TrashManager::deleteFiles(const DUrlList &urlList, const FMEvent &event, bool &accepted) const
{
    accepted = true;

    DUrlList localList;

    for(const DUrl &url : urlList) {
        const QString &path = url.path();

        localList << DUrl::fromLocalFile(TRASHFILEPATH + path);

        if(path.lastIndexOf('/') > 0) {
            localList << DUrl::fromLocalFile(TRASHINFOPATH + path);
        }
    }

    fileService->deleteFiles(localList, event);

    return true;
}

const DDirIteratorPointer TrashManager::createDirIterator(const DUrl &fileUrl, QDir::Filters filters,
                                                          QDirIterator::IteratorFlags flags, bool &accepted) const
{
    accepted = true;

    return DDirIteratorPointer(new TrashDirIterator(fileUrl, filters, flags));
}

bool TrashManager::restoreTrashFile(const DUrlList &fileUrl, const FMEvent &event) const
{
    bool ok = true;

    for(const DUrl &url : fileUrl) {
        TrashFileInfo info;
        info.setUrl(url);
        const_cast<FMEvent &>(event) = url;
        info.restore(event);
    }

    return ok;
}

bool TrashManager::restoreAllTrashFile(const FMEvent &event)
{

    DUrl fileUrl = event.fileUrlList().at(0);
    const QString &path = fileUrl.path();

    QDir dir(TRASHFILEPATH + path);
    DUrlList urlList;
    if(dir.exists()) {
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System);
        for(const QString name : entryList) {
            urlList << DUrl::fromTrashFile(QString("/%1").arg(name));
        }

        restoreTrashFile(urlList, event);
    }
    return true;
}

bool TrashManager::isEmpty()
{
    QDir dir(TRASHFILEPATH);
    QStringList entryList = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System);
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
    emit childrenAdded(DUrl::fromTrashFile(filePath.mid((TRASHFILEPATH).size())));
}

void TrashManager::onFileRemove(const QString &filePath) const
{
    emit childrenRemoved(DUrl::fromTrashFile(filePath.mid((TRASHFILEPATH).size())));
}
