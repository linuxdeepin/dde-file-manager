#include "appcontroller.h"
#include "fileoperation.h"
#include "app/global.h"


AppController::AppController(QObject *parent) : QObject(parent)
{
    initControllers();
    initConnect();
}

AppController::~AppController()
{

}

void AppController::initControllers()
{
    m_fileOperationController = new FileOperation(DaemonServicePath, this);
}

void AppController::initConnect()
{

}

