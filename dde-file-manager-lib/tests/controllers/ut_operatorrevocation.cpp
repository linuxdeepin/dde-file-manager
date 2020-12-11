#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "dfmevent.h"

#define protected public
#include "controllers/operatorrevocation.h"

DFM_BEGIN_NAMESPACE
namespace  {
    class TestOperatorRevocation : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            m_pController = new OperatorRevocation();
        }

        virtual void TearDown() override
        {
            delete m_pController;
            m_pController = nullptr;
        }

        OperatorRevocation *m_pController;
    };
}

TEST_F(TestOperatorRevocation, test_save_operation)
{
    ASSERT_NE(m_pController, nullptr);

    auto operatEvent = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList{DUrl()});
    auto event = dMakeEventPointer<DFMSaveOperatorEvent>(nullptr, operatEvent);
    EXPECT_TRUE(m_pController->fmEvent(event));
}

TEST_F(TestOperatorRevocation, test_revoc_operation)
{
    ASSERT_NE(m_pController, nullptr);

    auto operatEvent = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList{DUrl()});
    auto event_1 = dMakeEventPointer<DFMSaveOperatorEvent>(nullptr, operatEvent);
    auto event_2 = dMakeEventPointer<DFMSaveOperatorEvent>(nullptr, operatEvent);
    auto event_3 = dMakeEventPointer<DFMSaveOperatorEvent>(nullptr, operatEvent);
    m_pController->fmEvent(event_1);
    m_pController->fmEvent(event_2);
    m_pController->fmEvent(event_3);
    auto event = dMakeEventPointer<DFMRevocationEvent>(nullptr);
    EXPECT_TRUE(m_pController->fmEvent(event));
}

TEST_F(TestOperatorRevocation, test_clear_operation)
{
    ASSERT_NE(m_pController, nullptr);

    auto event = dMakeEventPointer<DFMCleanSaveOperatorEvent>(nullptr);
    EXPECT_FALSE(m_pController->fmEvent(event));
}

TEST_F(TestOperatorRevocation, test_instance)
{
    EXPECT_NE(OperatorRevocation::instance(), nullptr);
}

DFM_END_NAMESPACE
