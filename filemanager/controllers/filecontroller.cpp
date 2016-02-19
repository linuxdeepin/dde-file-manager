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

void FileController::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestIcon,
            this, &FileController::getIcon);
    connect(fileSignalManager, &FileSignalManager::requestChildren,
            this, &FileController::getChildren);
}

void FileController::getChildren(const QUrl &url)
{
    FileOperationsInterface *dbusInterface = dbusController->getFileOperationsInterface();

    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusInterface->NewListJob(url.toString(), 0);
    reply.waitForFinished();

    if(reply.isError()) {
        qDebug() << reply.error().message();
        return;
    }

    ListJobInterface *listJob = m_urlToListJobInterface.value(url);

    if(!listJob) {
        const QVariantList &args = reply.reply().arguments();

        listJob = new ListJobInterface(args[0].toString(),
                                       qvariant_cast<QDBusObjectPath>(args[1]).path(),
                                       args[2].toString());

        m_urlToListJobInterface[url] = listJob;
    }

    ASYN_CALL(listJob->Execute(), {
                  FileItemInfoList fileInfoList = (QDBusPendingReply<FileItemInfoList>(*watcher)).value();

                  emit fileSignalManager->childrenChanged(url, fileInfoList);
              }, this, url);
}

void FileController::getIcon(const QUrl &url) const
{
    QString iconPath = dbusController->getFileInfoInterface()->GetThemeIcon(url.toString(), 30);
    QIcon icon;

    if(iconPath.isEmpty()) {
        const QFileInfo &fileInfo = QFileInfo(url.toLocalFile());

        QFileIconProvider prrovider;
        icon = prrovider.icon(fileInfo);
    } else {
        icon = QIcon(iconPath);
    }

    emit fileSignalManager->iconChanged(url, icon);
}
