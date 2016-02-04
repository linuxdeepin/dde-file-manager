#include "trashjobcontroller.h"
#include "dbusinterface/trashjob_interface.h"
#include "dbusinterface/trashmonitor_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "dbusinterface/emptytrashjob_interface.h"
#include "../app/global.h"
#include "dbuscontroller.h"
#include "dialogs/cleartrashdialog.h"
#include "fileconflictcontroller.h"
#include "dbusinterface/services/conflictdaptor.h"


TrashJobController::TrashJobController(QObject *parent) : QObject(parent)
{
    m_trashMonitorInterface = new TrashMonitorInterface(FileMonitor_service, TrashMonitor_path, QDBusConnection::sessionBus(), this);
    m_conflictController = new FileConflictController();
    initConnect();
}

void TrashJobController::initConnect(){
    connect(signalManager, SIGNAL(trashEmptyChanged(bool)), this, SLOT(setTrashEmptyFlag(bool)));
    connect(signalManager, SIGNAL(trashingAboutToExcute(QStringList)),
            this, SLOT(trashJobExcute(QStringList)));
    connect(signalManager, SIGNAL(trashingAboutToAborted()),
            this, SLOT(trashJobAbort()));

    connect(signalManager, SIGNAL(requestEmptyTrash()), this, SLOT(confirmDelete()));
    connect(signalManager, SIGNAL(actionHandled(int)), this, SLOT(handleTrashAction(int)));
}

void TrashJobController::asyncRequestTrashCount(){
    QDBusPendingReply<uint> reply = m_trashMonitorInterface->ItemCount();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                        this, SLOT(asyncRequestTrashCountFinihsed(QDBusPendingCallWatcher*)));
}

void TrashJobController::asyncRequestTrashCountFinihsed(QDBusPendingCallWatcher *call){
    QDBusPendingReply<uint> reply = *call;
    if (!reply.isError()){
        uint count = reply.argumentAt(0).toUInt();
        if (count == 0){
            m_isTrashEmpty = true;
        }else{
            m_isTrashEmpty = false;
        }
    }else{
        qCritical() << reply.error().message();
    }
    connect(m_trashMonitorInterface, SIGNAL(ItemCountChanged(uint)), this, SLOT(updateTrashIconByCount(uint)));
    call->deleteLater();
}



void TrashJobController::setTrashEmptyFlag(bool flag){
    m_isTrashEmpty = flag;
}

void TrashJobController::trashJobExcute(const QStringList &files){
    qDebug() << files;
    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewTrashJob(
                files,
                false,
                ConflictAdaptor::staticServerPath(),
                m_conflictController->getObjectPath(),
                ConflictAdaptor::staticInterfaceName()
                );
//    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewTrashJob(
//                files,
//                false,
//                "",
//                "",
//                ""
//                );
    reply.waitForFinished();
    if (!reply.isError()){
        QString service = reply.argumentAt(0).toString();
        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
        m_trashJobInterface = new TrashJobInterface(service, path, QDBusConnection::sessionBus(), this);
        connectTrashSignal();
        m_trashJobInterface->Execute();
    }else{
        qCritical() << reply.error().message() << files;
    }
}

void TrashJobController::connectTrashSignal(){
    if (m_trashJobInterface){
        connect(m_trashJobInterface, SIGNAL(Done()), this, SLOT(trashJobExcuteFinished()));
        connect(m_trashJobInterface, SIGNAL(Aborted()), this, SLOT(trashJobAbortFinished()));
        connect(m_trashJobInterface, SIGNAL(Trashing(QString)), this, SLOT(onTrashingFile(QString)));
        connect(m_trashJobInterface, SIGNAL(Deleting(QString)), this, SLOT(onDeletingFile(QString)));
        connect(m_trashJobInterface, SIGNAL(ProcessedAmount(qlonglong,ushort)), this, SLOT(onProcessAmount(qlonglong,ushort)));
    }
}

void TrashJobController::disconnectTrashSignal(){
    if (m_trashJobInterface){
        disconnect(m_trashJobInterface, SIGNAL(Done()), this, SLOT(trashJobAbortFinished()));
        disconnect(m_trashJobInterface, SIGNAL(Aborted()), this, SLOT(trashJobAbortFinished()));
        disconnect(m_trashJobInterface, SIGNAL(Trashing(QString)), this, SLOT(onTrashingFile(QString)));
        disconnect(m_trashJobInterface, SIGNAL(Deleting(QString)), this, SLOT(onDeletingFile(QString)));
        disconnect(m_trashJobInterface, SIGNAL(ProcessedAmount(qlonglong,ushort)), this, SLOT(onProcessAmount(qlonglong,ushort)));
    }
}

void TrashJobController::trashJobExcuteFinished(){
    disconnectTrashSignal();
    m_trashJobInterface->deleteLater();
    m_trashJobInterface = NULL;

    qDebug() << "trash files deleted";
}


void TrashJobController::trashJobAbort(){
    if (m_trashJobInterface){
        m_trashJobInterface->Abort();
    }
}

void TrashJobController::trashJobAbortFinished(){
    disconnectTrashSignal();
    m_trashJobInterface->deleteLater();
    m_trashJobInterface = NULL;
}

void TrashJobController::onTrashingFile(QString file){
    emit signalManager->trashingFileChanged(file);
}

void TrashJobController::onDeletingFile(QString file){
    emit signalManager->deletingFileChanged(file);
}

void TrashJobController::onProcessAmount(qlonglong progress, ushort info){
    emit signalManager->processAmountChanged(progress, info);
}

void TrashJobController::updateTrashIconByCount(uint count){
    if (count == 0){
        if (!m_isTrashEmpty){
            dbusController->asyncRequestTrashIcon();
        }
        m_isTrashEmpty = true;
    }else{
        if (m_isTrashEmpty){
            dbusController->asyncRequestTrashIcon();
        }
        m_isTrashEmpty = false;
    }
}


void TrashJobController::confirmDelete(){
    QDBusPendingReply<uint> reply = m_trashMonitorInterface->ItemCount();
    reply.waitForFinished();
    if (!reply.isError()){
        uint count = reply.argumentAt(0).toUInt();
        if (count == 0){
             qCritical() << "count read error";
        }else{
             emit signalManager->confimClear(count);
        }
    }else{
        qCritical() << reply.error().message();
    }
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
    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewEmptyTrashJob(false, "", "", "");
    reply.waitForFinished();
    if (!reply.isError()){
        QString service = reply.argumentAt(0).toString();
        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
        m_emptyTrashJobInterface = new EmptyTrashJobInterface(service, path, QDBusConnection::sessionBus(), this);
        connect(m_emptyTrashJobInterface, SIGNAL(Done()), this, SLOT(emptyTrashJobExcuteFinished()));
        m_emptyTrashJobInterface->Execute();
    }else{
        qCritical() << reply.error().message();
    }
}

void TrashJobController::emptyTrashJobExcuteFinished(){
    disconnect(m_emptyTrashJobInterface, SIGNAL(Done()), this, SLOT(emptyTrashJobExcuteFinished()));
    m_trashJobInterface->deleteLater();
    m_trashJobInterface = NULL;
}

TrashJobController::~TrashJobController()
{

}

