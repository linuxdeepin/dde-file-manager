#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "views/dfilemanagerwindow.h"

#include "controllers/subscriber.h"

DFM_USE_NAMESPACE

enum ActionType {
    Open,
    OpenNewWindow,
    OpenNewTab
};


namespace  {

class TestSubscriber : public testing::Test
{
public:
    Subscriber *sub;
    virtual void SetUp() override
    {
        sub = new Subscriber();
    }

    virtual void TearDown() override
    {
    }
};

}

TEST_F(TestSubscriber, tst_eventKey)
{
    sub->setEventKey(Open);
    EXPECT_TRUE(sub->eventKey() == Open);
}


