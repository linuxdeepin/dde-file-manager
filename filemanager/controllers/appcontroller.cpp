#include "appcontroller.h"
#include "movejobcontroller.h"
#include "trashjobcontroller.h"
#include "copyjobcontroller.h"
#include "renamejobcontroller.h"
#include "deletejobcontroller.h"
#include "dbuscontroller.h"
#include "fileconflictcontroller.h"
#include "../app/global.h"
#include "histroycontroller.h"


AppController::AppController(QObject *parent) : QObject(parent),
    m_trashJobController(new TrashJobController),
    m_moveJobController(new MoveJobController),
    m_copyJobController(new CopyJobController),
    m_renameJobController(new RenameJobController),
    m_deleteJobController(new DeleteJobController),
    m_historyController(new HistroyController)
{

}

void AppController::initConnect(){

}

TrashJobController* AppController::getTrashJobController(){
    return m_trashJobController;
}

MoveJobController* AppController::getMoveJobController(){
    return m_moveJobController;
}

CopyJobController* AppController::getCopyJobController(){
    return m_copyJobController;
}

RenameJobController* AppController::getRenameJobController(){
    return m_renameJobController;
}

HistroyController *AppController::getHistroyController()
{
    return m_historyController;
}

AppController::~AppController()
{

}

