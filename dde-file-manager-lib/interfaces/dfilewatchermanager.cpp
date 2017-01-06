#include "dfilewatchermanager.h"
#include "dfilewatcher.h"
#include "durl.h"

DFM_BEGIN_NAMESPACE

class DFileWatcherManagerPrivate
{
public:
    DFileWatcherManagerPrivate();

    QMap<QString, DFileWatcher*> watchersMap;
};

DFileWatcherManagerPrivate::DFileWatcherManagerPrivate()
{

}

DFileWatcherManager::DFileWatcherManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFileWatcherManagerPrivate())
{

}

DFileWatcherManager::~DFileWatcherManager()
{

}

DFileWatcher *DFileWatcherManager::add(const QString &filePath)
{
    Q_D(DFileWatcherManager);

    DFileWatcher *watcher = d->watchersMap.value(filePath);

    if (watcher)
        return watcher;

    watcher = new DFileWatcher(filePath, this);

    connect(watcher, &DFileWatcher::fileAttributeChanged, this, [this](const DUrl &url) {
        emit fileAttributeChanged(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileClosed, this, [this](const DUrl &url) {
        emit fileClosed(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileDeleted, this, [this](const DUrl &url) {
        emit fileDeleted(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileModified, this, [this](const DUrl &url) {
        emit fileModified(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileMoved, this, [this](const DUrl &fromUrl, const DUrl &toUrl) {
        emit fileMoved(fromUrl.toLocalFile(), toUrl.toLocalFile());
    });
    connect(watcher, &DFileWatcher::subfileCreated, this, [this](const DUrl &url) {
        emit subfileCreated(url.toLocalFile());
    });

    d->watchersMap[filePath] = watcher;
    watcher->startWatcher();

    return watcher;
}

void DFileWatcherManager::remove(const QString &filePath)
{
    Q_D(DFileWatcherManager);

    DFileWatcher *watcher = d->watchersMap.take(filePath);

    if (watcher)
        watcher->deleteLater();
}

DFM_END_NAMESPACE
