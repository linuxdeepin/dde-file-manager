#include "appcontroller.h"
#include "movejobcontroller.h"
#include "trashjobcontroller.h"
#include "copyjobcontroller.h"
#include "renamejobcontroller.h"
#include "deletejobcontroller.h"
#include "../app/global.h"
#include "filecontroller.h"
#include "filemonitor/filemonitor.h"

AppController::AppController(QObject *parent) : QObject(parent),
    m_trashJobController(new TrashJobController(this)),
    m_moveJobController(new MoveJobController(this)),
    m_copyJobController(new CopyJobController(this)),
    m_renameJobController(new RenameJobController(this)),
    m_deleteJobController(new DeleteJobController(this)),
    m_fileController(new FileController(this)),
    m_fileMonitor(new FileMonitor(this))
{

}

void AppController::initConnect(){

}

TrashJobController* AppController::getTrashJobController() const{
    return m_trashJobController;
}

MoveJobController* AppController::getMoveJobController() const{
    return m_moveJobController;
}

CopyJobController* AppController::getCopyJobController() const{
    return m_copyJobController;
}

RenameJobController* AppController::getRenameJobController() const{
    return m_renameJobController;
}

FileMonitor *AppController::getFileMonitor() const
{
    return m_fileMonitor;
}

AppController::~AppController()
{

}

