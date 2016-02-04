#include "movejobcontroller.h"
#include "dbusinterface/movejob_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "dbusinterface/services/conflictdaptor.h"
#include "../app/global.h"
#include "movejobworker.h"
#include "../controllers/fileconflictcontroller.h"
#include "dbusinterface/services/conflictdaptor.h"

MoveJobController::MoveJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void MoveJobController::initConnect(){
    connect(signalManager, SIGNAL(moveFilesExcuted(QStringList,QString)),
            this, SLOT(createMoveJob(QStringList,QString)));
    connect(signalManager, SIGNAL(conflictRepsonseConfirmed(QMap<QString,QString>,QMap<QString,QVariant>)),
            this, SLOT(handleConflictConfirmResponse(QMap<QString,QString>,QMap<QString,QVariant>)));
}

void MoveJobController::createMoveJob(QStringList files, QString destination){
    MovejobWorker* worker = new MovejobWorker(files, destination);
    m_works.append(worker);
    emit worker->startJob();
    qDebug() << worker;
}


void MoveJobController::handleConflictConfirmResponse(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response){
    for(int i=0; i< m_works.length(); i++){
        if (m_works.at(i)->getJobDetail() == jobDetail){
            ConflictInfo obj;
            if (response.value("code").toInt() == 0){
                obj.code = static_cast<int>(ConflictInfo::ResponseAutoRename);
            }else if (response.value("code").toInt() == 1){
                obj.code = static_cast<int>(ConflictInfo::ResponseOverwrite);
            }else if (response.value("code").toInt() == 2){
                obj.code = static_cast<int>(ConflictInfo::ResponseSkip);
            }
            obj.applyToAll = response.value("applyToAll").toBool();
            obj.userData = "";
            qDebug() << obj.code;
            m_works.at(i)->handleResponse(obj);
            break;
        }
    }
}

MoveJobController::~MoveJobController()
{

}

