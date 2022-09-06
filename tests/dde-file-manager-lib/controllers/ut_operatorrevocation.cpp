// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "dfmevent.h"

#define protected public
#define private public
#include "controllers/operatorrevocation.h"
#include "stub.h"

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

TEST_F(TestOperatorRevocation, test_slotRevocationEvent)
{
    ASSERT_NE(m_pController, nullptr);

    auto event = dMakeEventPointer<DFMSaveOperatorEvent>();
    event->setProperty(QT_STRINGIFY(DFMSaveOperatorEvent::split), false);
    m_pController->operatorStack.push(*event.data());

    QSharedPointer<DFMEvent> (*st_event)() = []()->QSharedPointer<DFMEvent> {
        return QSharedPointer<DFMEvent>(new DFMEvent());
    };
    Stub stub;
    stub.set(ADDR(DFMSaveOperatorEvent, event), st_event);

    EXPECT_NO_FATAL_FAILURE(m_pController->slotRevocationEvent(QString("root")));
}

DFM_END_NAMESPACE
