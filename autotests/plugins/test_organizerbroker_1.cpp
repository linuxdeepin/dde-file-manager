// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_organizerbroker_1.cpp
 * @brief Unit tests for OrganizerBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/organizerbroker.h"

#include <QTest>

using namespace ddplugin_organizer;

class OrganizerBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OrganizerBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OrganizerBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OrganizerBrokerTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}
