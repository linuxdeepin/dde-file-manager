#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QCoreApplication>
#include <QDebug>
#include <QUrl>
#include <QProcess>

#include "durl.h"

#include "interfaces/dfileproxywatcher.h"
#include "interfaces/dfilewatcher.h"
#include "interfaces/dabstractfilewatcher.h"

namespace testFileWatcher
{
static DUrl urlConvertFun(const DUrl& url)
{
    qDebug() << "callback" << __PRETTY_FUNCTION__ << url;
    return url;
}

typedef decltype(testFileWatcher::urlConvertFun) callBack_t;

class DFileProxyWatcherTest: public testing::Test
{
public:
    DFileProxyWatcher *watcher = nullptr;
    std::function<callBack_t> callBack = testFileWatcher::urlConvertFun;
    QString RunPath = QCoreApplication::applicationDirPath();
    QString mkBefFileName = "/testFile0.txt";
    QString mvBefFileName = "/testFile1.txt";
    QString rnBefFileName = "/testFile2.txt";
    QString rmAftFileName = "/testFile2.txt";

    virtual void SetUp() override
    {
        qDebug() << __PRETTY_FUNCTION__;
        watcher = new DFileProxyWatcher(DUrl(RunPath),
                                        new DFileWatcher(RunPath),
                                        callBack);
    }

    virtual void TearDown() override
    {
        qDebug() << __PRETTY_FUNCTION__;
    }

public:

};

TEST_F(DFileProxyWatcherTest,start)
{
    qDebug() << __PRETTY_FUNCTION__;

    EXPECT_TRUE(watcher->startWatcher());

    qDebug() << "start watcher and path? >> "
             << qobject_cast<DAbstractFileWatcher*>(watcher)->fileUrl().toString();

    if(0 == QProcess::execute("touch", { RunPath + mkBefFileName })) //0
        qDebug() << "touch file ok:" << mkBefFileName;

    if(0 == QProcess::execute("echo", { "123123123",">>", RunPath + mkBefFileName })) // t > 0
        qDebug() << "write file ok:" << mkBefFileName;

    if(0 == QProcess::execute("mv", { RunPath + mkBefFileName, RunPath + mvBefFileName })) // 0 > 1
        qDebug() << "move" << mkBefFileName << "to" << mvBefFileName;

    if(0 == QProcess::execute("rm", { "-rf", RunPath + rmAftFileName })) // N < 1
        qDebug() << "remove file ok:" << rmAftFileName;
}
}
