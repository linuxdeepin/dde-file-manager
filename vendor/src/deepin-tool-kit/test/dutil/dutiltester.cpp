#include "dutiltester.h"

#include <QCoreApplication>
#include <QtTest/QtTest>
#include <QStandardPaths>

#include "dlog/LogManager.h"
#include "dpathbuf.h"

DUTIL_USE_NAMESPACE

TestDUtil::TestDUtil()
{

}

void TestDUtil::testLogPath()
{
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName("deepin-test-dtk");

    DPathBuf logPath(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    logPath = logPath / ".cache/deepin/deepin-test-dtk/deepin-test-dtk.log";

    QCOMPARE(DLogManager::getlogFilePath(), logPath.toString());
}

