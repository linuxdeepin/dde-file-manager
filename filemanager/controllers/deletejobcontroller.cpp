#include "deletejobcontroller.h"
#include "dbusinterface/deletejob_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "dialogs/confirmdeletedialog.h"
#include "../app/global.h"
#include "deletejobworker.h"

DeleteJobController::DeleteJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void DeleteJobController::initConnect(){
    connect(signalManager, SIGNAL(filesDeleted(QStringList)),
            this, SLOT(createDeleteJob(QStringList)));
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

