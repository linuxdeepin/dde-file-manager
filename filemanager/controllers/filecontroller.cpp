#include "filecontroller.h"
#include "../app/global.h"
#include "../app/filemanagerapp.h"
#include "fileinfogatherer.h"
#include "recenthistorymanager.h"
#include "../shutil/iconprovider.h"
#include "../../filemonitor/filemonitor.h"
#include "fileinfo.h"
#include "appcontroller.h"

#include <QFileIconProvider>
#include <QDesktopServices>

FileController::FileController(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<FileInfo*>>("QList<FileInfo*>");

    initGatherer();
    initRecentManager();
    initIconProvider();

    /// call later
    QMetaObject::invokeMethod(this, "initConnect", Qt::QueuedConnection);
}

FileController::~FileController()
{
    gathererThread->terminate();
    gathererThread->wait();
    gathererThread->quit();
    gatherer->deleteLater();
}

void FileController::initGatherer()
{
    gatherer = new FileInfoGatherer;
    gathererThread = new QThread(gatherer);

    gatherer->moveToThread(gathererThread);
    gathererThread->start();
}

void FileController::initRecentManager()
{
    recentManager = new RecentHistoryManager;
}

void FileController::initIconProvider()
{
    iconProvider = new IconProvider(this);
}

void FileController::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestIcon,
            this, &FileController::getIcon);
    connect(fileSignalManager, &FileSignalManager::requestChildren,
            this, &FileController::onFetchFileInformation);
    connect(gatherer, &FileInfoGatherer::updates,
            fileSignalManager, &FileSignalManager::childrenChanged);
    connect(recentManager, &RecentHistoryManager::updates,
            fileSignalManager, &FileSignalManager::childrenChanged);
    connect(bookmarkManager, &BookMarkManager::updates,
            fileSignalManager, &FileSignalManager::childrenChanged);
    connect(trashManager, &TrashManager::updates,
            fileSignalManager, &FileSignalManager::childrenChanged);
    connect(recentManager, &RecentHistoryManager::fileCreated,
            fileMonitor, &FileMonitor::fileCreated);
}

void FileController::getIcon(const QString &url) const
{
    QIcon icon = iconProvider->getFileIcon(url);

    emit fileSignalManager->iconChanged(url, icon);
}

void FileController::onFetchFileInformation(const QString &url, int filter)
{
    const QString &scheme = QUrl(url).scheme();

    if(scheme.isEmpty()) {
        gatherer->fetchFileInformation(url, filter);
    } else if(scheme == RECENT_SCHEME) {
        recentManager->fetchFileInformation(url, filter);
    } else if(scheme == BOOKMARK_SCHEME) {
        bookmarkManager->fetchFileInformation(url, filter);
    }else if(scheme == TRASH_SCHEME) {
        trashManager->fetchFileInformation(url, filter);
    }
}

bool FileController::openFile(const QString &url) const
{
    bool isOk = QDesktopServices::openUrl(QUrl::fromLocalFile(url));

    if(isOk)
        emit fileSignalManager->fileOpened(url);

    return isOk;
}
