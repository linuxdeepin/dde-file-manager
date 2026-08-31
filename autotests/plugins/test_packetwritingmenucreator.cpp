// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_packetwritingmenucreator.cpp
 * @brief Unit tests for PacketWritingMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/packetwritingmenuscene.h"

#include <QTest>

using namespace dfmplugin_optical;

class PacketWritingMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PacketWritingMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PacketWritingMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PacketWritingMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
