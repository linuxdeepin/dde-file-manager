#include "appcontroller.h"
#include "movejobcontroller.h"
#include "trashjobcontroller.h"
#include "copyjobcontroller.h"
#include "deletejobcontroller.h"
#include "filecontroller.h"
#include "recenthistorymanager.h"
#include "trashmanager.h"

#include "../app/global.h"


AppController::AppController(QObject *parent) : QObject(parent)
{
    new FileController(this);
    new RecentHistoryManager(this);
    new TrashManager(this);
}

void AppController::initConnect()
{

}

AppController::~AppController()
{

}

