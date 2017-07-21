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
#include "singleton.h"

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
    return DFileService::instance()->createFileInfo(Q_NULLPTR, DUrl::fromTrashFile(filePath()));
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
    fileService->deleteFiles(event->sender(), event->urlList());

    return DUrlList();
}

bool TrashManager::restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const
{
    return restoreTrashFile(event->urlList());
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

    fileService->writeFilesToClipboard(event->sender(), event->action(), localList);

    return true;
}

DUrlList TrashManager::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    if (event->action() != DFMGlobal::CutAction || event->targetUrl() != DUrl::fromTrashFile("/")) {
        return DUrlList();
    }

    if (event->urlList().isEmpty())
        return DUrlList();

    return fileService->moveToTrash(event->sender(), event->urlList());
}


bool TrashManager::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList localList;

    for(const DUrl &url : event->urlList()) {
        if (DUrl::fromTrashFile("/") == url) {
            cleanTrash(event->sender());
            return true;
        }

        const QString &path = url.path();

        localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + path);

        if (path.lastIndexOf('/') == 0) {
            localList << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + path + ".trashinfo");
        }
    }

    fileService->deleteFiles(event->sender(), localList, true);

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

bool TrashManager::restoreTrashFile(const DUrlList &list)
{
    bool ok = true;

    for (const DUrl &url : list) {
        if (url == DUrl::fromTrashFile("/")) {
            // restore all
            DUrlList list;

            for (const DAbstractFileInfoPointer &info : DFileService::instance()->getChildren(Q_NULLPTR, DUrl::fromTrashFile("/"), QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot))
                list << info->fileUrl();

            if (list.isEmpty())
                return true;

            return restoreTrashFile(list);
        }

        //###(zccrs): 必须通过 DAbstractFileInfoPointer 使用
        //            因为对象会被缓存，所以有可能在其它线程中被使用
        //            如果直接定义一个TrashFileInfo对象，就可能存在对象被重复释放
        QExplicitlySharedDataPointer<TrashFileInfo> info(new TrashFileInfo(url));

        ok = ok && info->restore();
    }

    return ok;
}

void TrashManager::cleanTrash(const QObject *sender) const
{
    DUrlList list;
    list << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath))
         << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));

    fileService->deleteFiles(sender, list, true);
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
