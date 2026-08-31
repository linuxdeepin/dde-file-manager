// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocolvirtualentryentity.cpp
 * @brief Unit tests for ProtocolVirtualEntryEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/info/protocolvirtualentryentity.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class ProtocolVirtualEntryEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProtocolVirtualEntryEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProtocolVirtualEntryEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProtocolVirtualEntryEntityTest, renamable)
{
    // Test bool getter: renamable()
    bool result = obj->renamable();
    EXPECT_FALSE(result);

}

TEST_F(ProtocolVirtualEntryEntityTest, targetUrl)
{
    // Test getter: QUrl targetUrl()
    auto result = obj->targetUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
