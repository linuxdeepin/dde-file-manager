// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_packetwritingmenuscene.cpp
 * @brief Unit tests for PacketWritingMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/packetwritingmenuscene.h"

#include <QTest>

using namespace dfmplugin_optical;

class PacketWritingMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PacketWritingMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PacketWritingMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PacketWritingMenuSceneTest, PacketWritingMenuScene)
{
    // Test constructor: PacketWritingMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PacketWritingMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PacketWritingMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PacketWritingMenuSceneTest, PacketWritingMenuScene_Destructor)
{
    // Test method:  ~PacketWritingMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ PacketWritingMenuScene *tmp = new PacketWritingMenuScene(); delete tmp; });
}
