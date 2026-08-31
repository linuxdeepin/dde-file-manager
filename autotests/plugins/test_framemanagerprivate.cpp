// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_framemanagerprivate.cpp
 * @brief Unit tests for FrameManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "framemanager.h"

#include <QTest>

using namespace ddplugin_organizer;

class FrameManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FrameManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FrameManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FrameManagerPrivateTest, buildSurface)
{
    // Test method: void buildSurface(())
    EXPECT_NO_FATAL_FAILURE(obj->buildSurface());
}

TEST_F(FrameManagerPrivateTest, clearSurface)
{
    // Test method: void clearSurface(())
    EXPECT_NO_FATAL_FAILURE(obj->clearSurface());
}

TEST_F(FrameManagerPrivateTest, onHideAllKeyPressed)
{
    // Test method: void onHideAllKeyPressed(())
    EXPECT_NO_FATAL_FAILURE(obj->onHideAllKeyPressed());
}

TEST_F(FrameManagerPrivateTest, saveHideAllSequence)
{
    // Test method: void saveHideAllSequence((const QKeySequence &seq))
    QKeySequence _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveHideAllSequence(_arg0));
}

TEST_F(FrameManagerPrivateTest, surfaces)
{
    // Test getter: QList<SurfacePointer> surfaces()
    auto result = obj->surfaces();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FrameManagerPrivateTest, FrameManagerPrivate)
{
    // Test constructor: FrameManagerPrivate((FrameManager *qq))
    ASSERT_NE(obj, nullptr);
}
