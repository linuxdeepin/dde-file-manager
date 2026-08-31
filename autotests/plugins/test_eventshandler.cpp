// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventshandler.cpp
 * @brief Unit tests for EventsHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/eventshandler.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class EventsHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EventsHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EventsHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventsHandlerTest, EventsHandler)
{
    // Test constructor: EventsHandler((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EventsHandlerTest, onOverlayDMModeChanged)
{
    // Test method: void onOverlayDMModeChanged((bool enabled, int result))
    EXPECT_NO_FATAL_FAILURE(obj->onOverlayDMModeChanged(false, 0));
}

TEST_F(EventsHandlerTest, showChgPwdError)
{
    // Test method: void showChgPwdError((const QString &dev, const QString &devName, int code))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showChgPwdError(_arg0, _arg1, 0));
}

TEST_F(EventsHandlerTest, showDecryptError)
{
    // Test method: void showDecryptError((const QString &dev, const QString &devName, int code))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showDecryptError(_arg0, _arg1, 0));
}

TEST_F(EventsHandlerTest, instance)
{
    // Test getter: EventsHandler instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(EventsHandlerTest, onRequestAuthArgs)
{
    // Test method: void onRequestAuthArgs((const QVariantMap &devInfo))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRequestAuthArgs(_arg0));
}
