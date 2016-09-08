#include "movejobcontroller.h"
#include "app/global.h"
#include "movejobworker.h"

MoveJobController::MoveJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void MoveJobController::initConnect(){

}

void MoveJobController::createMoveJob(QStringList files, QString destination){
    MovejobWorker* worker = new MovejobWorker(files, destination);
    m_works.append(worker);
    emit worker->startJob();
    qDebug() << worker;
}


void MoveJobController::handleConflictConfirmResponse(const QMap<QString, QString> &/*jobDetail*/, const QMap<QString, QVariant> &/*response*/){

}

MoveJobController::~MoveJobController()
{

}

