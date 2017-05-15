#include "logutil.h"
#include "commandlinemanager.h"
#include <DLog>

DUTIL_USE_NAMESPACE

LogUtil::LogUtil()
{

}

LogUtil::~LogUtil()
{

}

void LogUtil::registerLogger()
{
//#if !defined(QT_NO_DEBUG)
    DLogManager::registerConsoleAppender();
//#endif
    DLogManager::registerFileAppender();
}
