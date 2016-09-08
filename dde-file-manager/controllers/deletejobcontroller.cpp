#include "deletejobcontroller.h"
#include "dialogs/confirmdeletedialog.h"
#include "app/global.h"
#include "deletejobworker.h"

DeleteJobController::DeleteJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void DeleteJobController::initConnect(){

}

void DeleteJobController::createDeleteJob(const QStringList &files){
    DeletejobWorker* worker = new DeletejobWorker(files);
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    emit worker->startJob();
    qDebug() << worker;
}

DeleteJobController::~DeleteJobController()
{

}

