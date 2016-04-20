#include "appcontroller.h"
#include "movejobcontroller.h"
#include "trashjobcontroller.h"
#include "copyjobcontroller.h"
#include "deletejobcontroller.h"
#include "filecontroller.h"
#include "recenthistorymanager.h"
#include "trashmanager.h"
#include "searchcontroller.h"

#include "../app/global.h"


AppController::AppController(QObject *parent) : QObject(parent)
{
    FileServices::instance()->setFileUrlHandler(RECENT_SCHEME, "", new RecentHistoryManager(this));

    FileServices::dRegisterUrlHandler<FileController>(FILE_SCHEME, "");
    FileServices::dRegisterUrlHandler<TrashManager>(TRASH_SCHEME, "");
    FileServices::dRegisterUrlHandler<SearchController>(SEARCH_SCHEME, "");
}

void AppController::initConnect()
{

}

AppController::~AppController()
{

}

