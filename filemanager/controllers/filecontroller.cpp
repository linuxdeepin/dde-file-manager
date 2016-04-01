#include "filecontroller.h"
#include "../app/global.h"
#include "fileinfogatherer.h"
#include "recenthistorymanager.h"
#include "../shutil/iconprovider.h"
#include "fileinfo.h"

#include <QFileIconProvider>

FileController::FileController(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<FileInfo*>>("QList<FileInfo*>");

    initGatherer();
    initRecentManager();
    initIconProvider();
    initConnect();
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
    }
}
