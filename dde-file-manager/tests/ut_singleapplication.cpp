#include <gtest/gtest.h>
#define private public
#include "singleapplication.h"
#undef private
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

TEST_F(SingleApplicationTest, test_setSingleInstance)
{
    ASSERT_NE(app, nullptr);
    QString uniqueKey = app->applicationName();
    bool isSingleInstance = app->setSingleInstance(uniqueKey);
    ASSERT_TRUE(isSingleInstance);
    isSingleInstance = app->setSingleInstance(uniqueKey);
    ASSERT_FALSE(isSingleInstance);
}

TEST_F(SingleApplicationTest, test_loadTranslator)
{
    ASSERT_NE(app, nullptr);
    QString appName = app->applicationName();
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

TEST_F(SingleApplicationTest, test_newClientProcess)
{
    ASSERT_NE(app->newClientProcess("testkey", nullptr), nullptr);
}
