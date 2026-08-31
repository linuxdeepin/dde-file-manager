// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventchannelmanager.cpp
 * @brief Unit tests for EventChannelManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/event/eventchannel.h"

#include <QTest>

using namespace include;

class EventChannelManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EventChannelManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EventChannelManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventChannelManagerTest, disconnect)
{
    // Test method: bool disconnect((const EventType &type))
    EventType _arg0{};
    auto result = obj->disconnect(_arg0);
    EXPECT_FALSE(result);

}
