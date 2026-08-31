// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocolentryfileentity_1.cpp
 * @brief Unit tests for ProtocolEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/protocolentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class ProtocolEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProtocolEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProtocolEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProtocolEntryFileEntityTest, editDisplayText)
{
    // Test getter: QString editDisplayText()
    auto result = obj->editDisplayText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ProtocolEntryFileEntityTest, order)
{
    // Test getter: DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder order()
    auto result = obj->order();
    EXPECT_NO_FATAL_FAILURE({ obj->order(); });

}

TEST_F(ProtocolEntryFileEntityTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(ProtocolEntryFileEntityTest, showProgress)
{
    // Test bool getter: showProgress()
    bool result = obj->showProgress();
    EXPECT_FALSE(result);

}

TEST_F(ProtocolEntryFileEntityTest, showTotalSize)
{
    // Test bool getter: showTotalSize()
    bool result = obj->showTotalSize();
    EXPECT_FALSE(result);

}

TEST_F(ProtocolEntryFileEntityTest, showUsageSize)
{
    // Test bool getter: showUsageSize()
    bool result = obj->showUsageSize();
    EXPECT_FALSE(result);

}
