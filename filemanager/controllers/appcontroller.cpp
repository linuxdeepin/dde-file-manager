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
    m_fileController(new FileController(this)),
    m_fileMonitor(new FileMonitor)
{

}

void AppController::initConnect(){

}

FileController *AppController::getFileController() const
{
    return m_fileController;
}

FileMonitor *AppController::getFileMonitor() const
{
    return m_fileMonitor;
}

AppController::~AppController()
{

}

