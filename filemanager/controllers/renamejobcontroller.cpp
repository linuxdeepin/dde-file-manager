#include "renamejobcontroller.h"
#include "../app/global.h"


RenameJobController::RenameJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void RenameJobController::initConnect(){

}

void RenameJobController::rename(QString url, QString newName){
    qDebug() << url << newName;
//    m_newName = newName;
//    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewRenameJob(url, newName);
//    reply.waitForFinished();
//    if (!reply.isError()){
//        QString service = reply.argumentAt(0).toString();
//        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
//        m_renameJobInterface = new RenameJobInterface(service, path, QDBusConnection::sessionBus(), this);
//        connect(m_renameJobInterface, SIGNAL(Done(QString)), this, SLOT(renameJobExcuteFinished(QString)));
//        m_renameJobInterface->Execute();
//    }else{
//        qCritical() << reply.error().message();
//    }
}

void RenameJobController::renameJobExcuteFinished(QString message){
    qDebug() << "rename job return" << message;

}

RenameJobController::~RenameJobController()
{

}

