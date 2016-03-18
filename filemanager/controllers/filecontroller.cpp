#include "filecontroller.h"
#include "../app/global.h"
#include "fileinfogatherer.h"
#include "../shutil/iconprovider.h"
#include "fileinfo.h"

#include <QFileIconProvider>

FileController::FileController(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<FileInfo*>>("QList<FileInfo*>");

    initGatherer();
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

void FileController::initIconProvider()
{
    iconProvider = new IconProvider(this);
}

void FileController::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestIcon,
            this, &FileController::getIcon);
    connect(fileSignalManager, &FileSignalManager::requestChildren,
            gatherer, &FileInfoGatherer::fetchFileInformation);
    connect(gatherer, &FileInfoGatherer::updates,
            fileSignalManager, &FileSignalManager::childrenChanged);
}

void FileController::getIcon(const QString &url) const
{
    QIcon icon = iconProvider->getFileIcon(url);

    emit fileSignalManager->iconChanged(url, icon);
}
