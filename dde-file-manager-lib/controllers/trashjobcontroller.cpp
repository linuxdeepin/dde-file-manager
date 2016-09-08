#include "trashjobcontroller.h"
#include "app/global.h"
#include "dialogs/cleartrashdialog.h"

TrashJobController::TrashJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void TrashJobController::initConnect(){

}

void TrashJobController::asyncRequestTrashCount(){

}

void TrashJobController::setTrashEmptyFlag(bool flag){
    m_isTrashEmpty = flag;
}

void TrashJobController::trashJobExcute(const QStringList &files){
    qDebug() << files;
//    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewTrashJob(
//                files,
//                false,
//                ConflictAdaptor::staticServerPath(),
//                m_conflictController->getObjectPath(),
//                ConflictAdaptor::staticInterfaceName()
//                );
////    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewTrashJob(
////                files,
////                false,
////                "",
////                "",
////                ""
////                );
//    reply.waitForFinished();
//    if (!reply.isError()){
//        QString service = reply.argumentAt(0).toString();
//        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
//        m_trashJobInterface = new TrashJobInterface(service, path, QDBusConnection::sessionBus(), this);
//        connectTrashSignal();
//        m_trashJobInterface->Execute();
//    }else{
//        qCritical() << reply.error().message() << files;
//    }
}

void TrashJobController::connectTrashSignal(){

}

void TrashJobController::disconnectTrashSignal(){

}

void TrashJobController::trashJobExcuteFinished(){

    qDebug() << "trash files deleted";
}


void TrashJobController::trashJobAbort(){

}

void TrashJobController::trashJobAbortFinished(){

}

void TrashJobController::onTrashingFile(QString /*file*/){

}

void TrashJobController::onDeletingFile(QString /*file*/){

}

void TrashJobController::onProcessAmount(qlonglong /*progress*/, ushort /*info*/){

}

void TrashJobController::updateTrashIconByCount(uint /*count*/){
//    if (count == 0){
//        if (!m_isTrashEmpty){
//            dbusController->asyncRequestTrashIcon();
//        }
//        m_isTrashEmpty = true;
//    }else{
//        if (m_isTrashEmpty){
//            dbusController->asyncRequestTrashIcon();
//        }
//        m_isTrashEmpty = false;
//    }
}


void TrashJobController::confirmDelete(){

}

void TrashJobController::handleTrashAction(int index){
    switch (index) {
    case 0:
        return;
        break;
    case 1:
        createEmptyTrashJob();
        break;
    default:
        break;
    }
}

void TrashJobController::createEmptyTrashJob(){
//    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewEmptyTrashJob(false, "", "", "");
//    reply.waitForFinished();
//    if (!reply.isError()){
//        QString service = reply.argumentAt(0).toString();
//        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
//        m_emptyTrashJobInterface = new EmptyTrashJobInterface(service, path, QDBusConnection::sessionBus(), this);
//        connect(m_emptyTrashJobInterface, SIGNAL(Done()), this, SLOT(emptyTrashJobExcuteFinished()));
//        m_emptyTrashJobInterface->Execute();
//    }else{
//        qCritical() << reply.error().message();
//    }
}

void TrashJobController::emptyTrashJobExcuteFinished(){

}

TrashJobController::~TrashJobController()
{

}

