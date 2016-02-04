#include "copyjobcontroller.h"
#include "../app/global.h"
#include "dbuscontroller.h"
#include "dbusinterface/copyjob_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "dbusinterface/services/conflictdaptor.h"
#include "copyjobworker.h"
#include "../controllers/fileconflictcontroller.h"
#include "dbusinterface/services/conflictdaptor.h"


CopyJobController::CopyJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void CopyJobController::initConnect(){
    connect(signalManager, SIGNAL(copyFilesExcuted(QStringList,QString)),
            this, SLOT(createCopyJob(QStringList,QString)));
    connect(signalManager, SIGNAL(conflictRepsonseConfirmed(QMap<QString,QString>,QMap<QString,QVariant>)),
            this, SLOT(handleConflictConfirmResponse(QMap<QString,QString>,QMap<QString,QVariant>)));

    connect(signalManager, SIGNAL(conflictTimerStoped(QMap<QString,QString>)),
            this, SLOT(handleWokerTimerStop(QMap<QString,QString>)));
    connect(signalManager, SIGNAL(conflictTimerReStarted(QMap<QString,QString>)),
            this, SLOT(handleWokerTimerRestart(QMap<QString,QString>)));
}

void CopyJobController::createCopyJob(QStringList files, QString destination){
    CopyjobWorker* worker = new CopyjobWorker(files, destination);
    m_works.append(worker);
    emit worker->startJob();
    qDebug() << worker;
}

void CopyJobController::handleConflictConfirmResponse(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response){
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
            qDebug() << obj.code << obj.applyToAll;
            m_works.at(i)->handleResponse(obj);
            break;
        }
    }
}

void CopyJobController::handleWokerTimerRestart(const QMap<QString, QString> &jobDetail){
    for(int i=0; i< m_works.length(); i++){
        if (m_works.at(i)->getJobDetail() == jobDetail){
            m_works.at(i)->restartTimer();
        }
    }
}

void CopyJobController::handleWokerTimerStop(const QMap<QString, QString> &jobDetail){
    for(int i=0; i< m_works.length(); i++){
        if (m_works.at(i)->getJobDetail() == jobDetail){
            m_works.at(i)->stopTimer();
        }
    }
}

CopyJobController::~CopyJobController()
{

}

