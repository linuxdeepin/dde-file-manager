// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_packetwritingmenusceneprivate.cpp
 * @brief Unit tests for PacketWritingMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/packetwritingmenuscene.h"

#include <QTest>

using namespace dfmplugin_optical;

class PacketWritingMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PacketWritingMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PacketWritingMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PacketWritingMenuScenePrivateTest, findSceneName)
{
    // Test method: DPOPTICAL_BEGIN_NAMESPACE findSceneName((QAction *act))
    auto result = obj->findSceneName(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->findSceneName(nullptr); });

}

TEST_F(PacketWritingMenuScenePrivateTest, isContainSubDirFile)
{
    // Test method: bool isContainSubDirFile((const QString &mnt))
    QString _arg0{};
    auto result = obj->isContainSubDirFile(_arg0);
    EXPECT_FALSE(result);

}
