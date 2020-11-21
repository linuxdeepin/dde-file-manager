#include <gtest/gtest.h>

#include "dfmevent.h"
#define protected public
#include "dfmabstracteventhandler.h"

DFM_USE_NAMESPACE

namespace  {
class TestDFMAbstractEventHandler: public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestDFMAbstractEventHandler" << std::endl;
        handler = new DFMAbstractEventHandler();
    }

    virtual void TearDown() override
    {
        std::cout << "start TestDFMAbstractEventHandler" << std::endl;
        if (handler) {
            delete handler;
            handler = nullptr;
        }
    }

public:
    DFMAbstractEventHandler *handler;
};
}

TEST_F(TestDFMAbstractEventHandler, object)
{
    EXPECT_TRUE(handler->object() == nullptr);
}

TEST_F(TestDFMAbstractEventHandler, fmEvent)
{
    EXPECT_FALSE(handler->fmEvent(dMakeEventPointer<DFMEvent>()));
}

TEST_F(TestDFMAbstractEventHandler, fmEventFilter)
{
    EXPECT_FALSE(handler->fmEventFilter(dMakeEventPointer<DFMEvent>()));
}
