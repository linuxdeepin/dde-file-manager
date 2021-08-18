#include "abus/appbus.h"
#include "definitions/globaldefinitions.h"

#include <QCoreApplication>
#include <QMetaObject>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
class UT_AppBus : public testing::Test
{
public:
    AppBus *ins;
    virtual void SetUp() override {
        ins = new AppBus();
    }

    virtual void TearDown() override {
        delete ins;
        ins = nullptr;
    }
};

//单测试
TEST_F(UT_AppBus, test_onlineService)
{
    EXPECT_EQ(true,!ins->onlineServer().isEmpty());
}

TEST_F(UT_AppBus, test_mimeServer)
{
    EXPECT_EQ(true,!ins->mimeServer().isEmpty());
}

TEST_F(UT_AppBus, test_isMimeServer)
{
    EXPECT_EQ(true,ins->isMimeServer(ins->onlineServer()[0]));
    EXPECT_EQ(true,ins->isMimeServer(ins->mimeServer()));
    EXPECT_EQ(false,ins->isMimeServer(""));
    EXPECT_EQ(false,ins->isMimeServer("dadasdasdasd"));
    EXPECT_EQ(false,ins->isMimeServer(".dadada-1000.AppBusPrivate"));
}

//组网测试

