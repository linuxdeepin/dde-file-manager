#include "appcontroller.h"
#include "fileoperation.h"
#include "usershare/usersharemanager.h"
#include "usbformatter/usbformatter.h"
#include "commandmanager/commandmanager.h"
#include "deviceinfo/deviceinfomanager.h"
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
//    m_fileOperationController = new FileOperation(DaemonServicePath, this);
    m_userShareManager = new UserShareManager(this);
    m_usbFormatter = new UsbFormatter(this);
//    m_commandManager = new CommandManager(this);
    m_deviceInfoManager = new DeviceInfoManager(this);
}

void AppController::initConnect()
{

}

