#include "trashmanager.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dfileproxywatcher.h"
#include "dfileinfo.h"
#include "models/trashfileinfo.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"

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

TrashManager::TrashManager(QObject *parent)
    : DAbstractFileController(parent),
      m_trashFileWatcher(new DFileWatcher(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath),this))
{
    m_isTrashEmpty = isEmpty();
    QString trashFilePath = DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath);
    //make sure trash file exists
    if(!QFile::exists(trashFilePath))
        QDir().mkdir(trashFilePath);

    connect(m_trashFileWatcher, &DFileWatcher::fileDeleted, this, &TrashManager::trashFilesChanged);
    connect(m_trashFileWatcher, &DFileWatcher::subfileCreated, this, &TrashManager::trashFilesChanged);
    m_trashFileWatcher->startWatcher();
}

const DAbstractFileInfoPointer TrashManager::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new TrashFileInfo(event->url()));
}

bool TrashManager::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    Q_UNUSED(event)

    qWarning() << "trash open action is disable : " << event->url();
//    return FileServices::instance()->openFile(DUrl::fromLocalFile(TRASHFILEPATH + fileUrl.path()));
    return false;
}

DUrlList TrashManager::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    TIMER_SINGLESHOT_CONNECT_TYPE(this, 0, fileService->deleteFiles(event->urlList(), event->sender()), Qt::AutoConnection, event);

    return DUrlList();
}

bool TrashManager::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    if (event->action() != DFMGlobal::CopyAction) {
        event->ignore();
        return false;
    }

    DUrlList localList;

    for(const DUrl &url : event->urlList()) {
        const QString &path = url.path();

        localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + path);

        if(path.lastIndexOf('/') > 0) {
            localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + path);
        }
    }

    fileService->writeFilesToClipboard(event->action(), localList, event->sender());

    return true;
}

DUrlList TrashManager::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    if (event->action() != DFMGlobal::CutAction || event->targetUrl() != DUrl::fromTrashFile("/")) {
        event->ignore();

        return DUrlList();
    }

    if (event->urlList().isEmpty())
        return DUrlList();

    return fileService->moveToTrashSync(event->urlList(), event->sender());
}


bool TrashManager::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList localList;

    for(const DUrl &url : event->urlList()) {
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

    fileService->deleteFilesSync(localList, event->sender());

    return true;
}

const DDirIteratorPointer TrashManager::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new TrashDirIterator(event->url(), event->nameFilters(), event->filters(), event->flags()));
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

DAbstractFileWatcher *TrashManager::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new DFileProxyWatcher(event->url(),
                                 new DFileWatcher(TrashManagerPrivate::trashToLocal(event->url())),
                                 TrashManagerPrivate::localToTrash);
}

bool TrashManager::restoreTrashFile(const DUrlList &fileUrl, const DFMEvent &event)
{
    bool ok = true;

    for(const DUrl &url : fileUrl) {
        TrashFileInfo info(url);

        const_cast<DFMEvent &>(event).setData(url);
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

void TrashManager::cleanTrash(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList list;
    list << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath))
         << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));

    fileService->deleteFilesSync(list, event->sender());
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

void TrashManager::trashFilesChanged(const DUrl& url)
{
    Q_UNUSED(url);
    if(m_isTrashEmpty == isEmpty())
        return;

    m_isTrashEmpty = isEmpty();
    emit fileSignalManager->trashStateChanged();
}
