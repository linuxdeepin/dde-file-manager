#include "renamejobcontroller.h"
#include "dbusinterface/renamejob_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "../app/global.h"


RenameJobController::RenameJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void RenameJobController::initConnect(){
    connect(signalManager, SIGNAL(renameJobCreated(QString,QString)), this, SLOT(rename(QString,QString)));
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
    if (message.length() == 0){
        disconnect(m_renameJobInterface, SIGNAL(Done(QString)), this, SLOT(renameJobExcuteFinished(QString)));
        m_renameJobInterface->deleteLater();
        m_renameJobInterface = NULL;
        qDebug() << "rename job finished" << message;
    }else{
        QJsonParseError* error = new QJsonParseError();
        QJsonObject messageObj = QJsonDocument::fromJson(QByteArray(message.toStdString().c_str()), error).object();
        qDebug() << messageObj << error->errorString();
        if (messageObj.contains("Code")){
            if (messageObj.value("Code").toInt() == 2){
                emit signalManager->renameDialogShowed(m_newName);
            }else{
                qDebug() << messageObj;
            }
        }
    }
}

RenameJobController::~RenameJobController()
{

}

