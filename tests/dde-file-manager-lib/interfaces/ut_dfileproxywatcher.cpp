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

class TestDFileProxyWatcher: public testing::Test
{
public:

    std::function<callBack_t> callBack = testFileWatcher::urlConvertFun;
    QString RunPath = QCoreApplication::applicationDirPath();
    QString creatFileName = "/testFile0.txt";
    QString moveFileName  = "/testFile1.txt";

    virtual void SetUp() override
    {

    }

    virtual void TearDown() override
    {

    }

public:

};


TEST_F(TestDFileProxyWatcher,start)
{
//    DFileWatcher* fileWatcher = new DFileWatcher(RunPath);
//    DFileProxyWatcher* proxyWatcher = new DFileProxyWatcher(DUrl(RunPath),fileWatcher,callBack);

//    EXPECT_TRUE(proxyWatcher->startWatcher()); //start

//    if (0 == QProcess::execute("touch", { RunPath + creatFileName })) {
//        qDebug() << "touch file ok:" << creatFileName;
//    }

//    if (0 == QProcess::execute("echo", { "123123123",">>", RunPath + creatFileName })) {
//        qDebug() << "write file ok:" << creatFileName;
//    }

//    if (0 == QProcess::execute("mv", { RunPath + creatFileName, RunPath + moveFileName })) {
//        qDebug() << "move" << creatFileName << "to" << moveFileName;
//    }

//    if (0 == QProcess::execute("rm", { "-rf", RunPath + moveFileName })) {
//        qDebug() << "remove file ok:" << moveFileName;
//    }

//    emit fileWatcher->fileModified(DUrl("RunPath"));

//    qApp->processEvents(); //wait the callback function exectue

//    EXPECT_TRUE(proxyWatcher->stopWatcher()); //stop

//    if (fileWatcher) {
//        delete fileWatcher;
//        fileWatcher = nullptr;
//    }

//    if (proxyWatcher) {
//        delete proxyWatcher;
//        proxyWatcher = nullptr;
//    }

}

}
