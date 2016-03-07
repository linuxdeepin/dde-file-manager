#include "filecontroller.h"
#include "../app/global.h"
#include "dbusinterface/listjob_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "dbusinterface/fileInfo_interface.h"

#include <QFileIconProvider>

FileController::FileController(QObject *parent) : QObject(parent)
{
    initConnect();
}

FileController::~FileController()
{

}

void FileController::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestIcon,
            this, &FileController::getIcon);
    connect(fileSignalManager, &FileSignalManager::requestChildren,
            this, &FileController::getChildren);
}

void FileController::getChildren(const QString &url)
{
    FileOperationsInterface *dbusInterface = dbusController->getFileOperationsInterface();

    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusInterface->NewListJob(url, 0);
    reply.waitForFinished();

    if(reply.isError()) {
        qDebug() << reply.error().message();
        return;
    }

    const QVariantList &args = reply.reply().arguments();

    ListJobInterface *listJob = new ListJobInterface(args[0].toString(),
                                   qvariant_cast<QDBusObjectPath>(args[1]).path(),
                                   args[2].toString());

    ASYN_CALL(listJob->Execute(), {
                  FileItemInfoList fileInfoList = (QDBusPendingReply<FileItemInfoList>(*watcher)).value();

                  foreach (FileItemInfo info, fileInfoList) {
                      m_fileItemInfos.insert(info.URI, info);
                  }
                  emit fileSignalManager->childrenChanged(url, fileInfoList);
              }, this, url);
}

void FileController::getIcon(const QString &url) const
{
    QString iconPath = dbusController->getFileInfoInterface()->GetThemeIcon(url, 30);
    QIcon icon;

    if(iconPath.isEmpty()) {
        const QFileInfo &fileInfo = QFileInfo(QUrl(url).toLocalFile());

        QFileIconProvider prrovider;
        icon = prrovider.icon(fileInfo);
    } else {
        icon = QIcon(iconPath);
    }

    emit fileSignalManager->iconChanged(url, icon);
}
