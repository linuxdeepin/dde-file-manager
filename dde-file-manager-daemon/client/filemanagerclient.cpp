#include "filemanagerclient.h"
#include "app/global.h"
#include "dbusservice/dbustype/dbusinforet.h"
#include "dbusservice/dbusinterface/renamejob_interface.h"
#include <QDBusConnection>
#include <QDebug>

FileManagerClient::FileManagerClient(QObject *parent) : QObject(parent)
{
    DBusInfoRet::registerMetaType();
    QString objectPath = "/com/deepin/filemanager/daemon/Operations";
    m_fileOperationInterface = new FileOperationInterface(DaemonServicePath, objectPath, QDBusConnection::systemBus());
    testNewNameJob("11111", "222222222");
}

FileManagerClient::~FileManagerClient()
{

}

void FileManagerClient::testNewNameJob(const QString &oldFile, const QString &newFile)
{
    QDBusPendingReply<DBusInfoRet> reply = m_fileOperationInterface->NewRenameJob(oldFile, newFile);
    reply.waitForFinished();
    if (reply.isFinished()){
        DBusInfoRet result = qdbus_cast<DBusInfoRet>(reply.argumentAt(0));
        qDebug() << result;
        RenameJobInterface* renamejobInterface = new RenameJobInterface(result.ServicePath, result.InterfacePath.path(), QDBusConnection::systemBus());
        renamejobInterface->Execute();
    }else{

    }
}

