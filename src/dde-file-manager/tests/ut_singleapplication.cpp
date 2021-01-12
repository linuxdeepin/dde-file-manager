#include <gtest/gtest.h>
#define private public
#include "singleapplication.h"
#undef private
#include <QtConcurrent>
#include "commandlinemanager.h"
#include <QLocalSocket>
#include "testhelper.h"
#include "dfilewatcher.h"
#include <QStandardPaths>

using namespace testing;

namespace  {
class SingleApplicationTest : public Test
{
public:
private:
    virtual void SetUp() override{
        app =  dynamic_cast<SingleApplication*>(qApp);
    }

    virtual void TearDown() override{
    }

public:
    SingleApplication *app;
};

}


TEST_F(SingleApplicationTest, test_loadTranslator)
{
    ASSERT_NE(app, nullptr);
    QString appName = app->applicationName();
    ASSERT_FALSE(app->loadTranslator());
    app->setApplicationName("dde-file-manager");
    ASSERT_TRUE(app->loadTranslator());
    app->setApplicationName(appName);
}

TEST_F(SingleApplicationTest, test_userServerName)
{
    QString key = "testKey";
    QString expectValue = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + "/" + key;
    QString serverName = app->userServerName(key);
    ASSERT_EQ(expectValue, serverName);
}

TEST_F(SingleApplicationTest, test_userId)
{
    ASSERT_EQ(app->userId(), SingleApplication::UserID);
}

TEST_F(SingleApplicationTest, test_getUserID)
{
    ASSERT_EQ(app->getUserID(), SingleApplication::UserID);
}

TEST_F(SingleApplicationTest, test_newClientProcess_get_monitor_files)
{
    ASSERT_NE(app, nullptr);
    app->initConnect();
    QString userKey = "SingleApplicationTest1";
    QString tepPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    DFileWatcher *watcher = new DFileWatcher(tepPath);
    watcher->startWatcher();
    TestHelper::runInLoop([&]{
        app->setSingleInstance(userKey);
        QtConcurrent::run([&](){
            QByteArray data;
            data.append(QString("dde-file-manager").toLocal8Bit().toBase64());
            data.append(' ');
            data.append(QString("--get-monitor-files").toLocal8Bit().toBase64());
            QLocalSocket *socket = SingleApplication::newClientProcess(userKey, data);
            ASSERT_EQ(socket->error(), QLocalSocket::UnknownSocketError);
            ASSERT_TRUE(socket->waitForReadyRead());
            QStringList result;
            for (const QByteArray &i : socket->readAll().split(' ')){
                QString s = QString::fromLocal8Bit(QByteArray::fromBase64(i));
                result << QString::fromLocal8Bit(QByteArray::fromBase64(i));
            }
            ASSERT_TRUE(result.contains(tepPath));
        });
    }, 1000);
    watcher->stopWatcher();
    delete watcher;
}

