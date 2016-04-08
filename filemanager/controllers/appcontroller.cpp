#include "appcontroller.h"
#include "movejobcontroller.h"
#include "trashjobcontroller.h"
#include "copyjobcontroller.h"
#include "deletejobcontroller.h"
#include "filecontroller.h"
#include "recenthistorymanager.h"

#include "filemonitor/filemonitor.h"

#include "../app/global.h"


AppController::AppController(QObject *parent) : QObject(parent),
    m_fileMonitor(new FileMonitor)
{
    new FileController(this);
    new RecentHistoryManager(this);
}

void AppController::initConnect()
{

}

FileMonitor *AppController::getFileMonitor() const
{
    return m_fileMonitor;
}

AppController::~AppController()
{

}

