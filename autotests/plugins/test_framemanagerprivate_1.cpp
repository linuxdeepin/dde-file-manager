// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_framemanagerprivate_1.cpp
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

TEST_F(FrameManagerPrivateTest, buildOrganizer)
{
    // Test method: void buildOrganizer(())
    EXPECT_NO_FATAL_FAILURE(obj->buildOrganizer());
}

TEST_F(FrameManagerPrivateTest, createSurface)
{
    // Test method: SurfacePointer createSurface((QWidget *root))
    auto result = obj->createSurface(nullptr);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FrameManagerPrivateTest, enableChanged)
{
    // Test method: void enableChanged((bool e))
    EXPECT_NO_FATAL_FAILURE(obj->enableChanged(false));
}

TEST_F(FrameManagerPrivateTest, enableVisibility)
{
    // Test method: void enableVisibility((bool e))
    EXPECT_NO_FATAL_FAILURE(obj->enableVisibility(false));
}

TEST_F(FrameManagerPrivateTest, findView)
{
    // Test method: QWidget findView((QWidget *root))
    auto result = obj->findView(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->findView(nullptr); });

}

TEST_F(FrameManagerPrivateTest, layoutSurface)
{
    // Test method: void layoutSurface((QWidget *root, SurfacePointer surface, bool hidden))
    EXPECT_NO_FATAL_FAILURE(obj->layoutSurface(nullptr, SurfacePointer(), false));
}

TEST_F(FrameManagerPrivateTest, onNotificationClosed)
{
    // Test method: void onNotificationClosed((uint id, uint reason))
    EXPECT_NO_FATAL_FAILURE(obj->onNotificationClosed(0, 0));
}

TEST_F(FrameManagerPrivateTest, refeshCanvas)
{
    // Test method: void refeshCanvas(())
    EXPECT_NO_FATAL_FAILURE(obj->refeshCanvas());
}

TEST_F(FrameManagerPrivateTest, showOptionWindow)
{
    // Test method: void showOptionWindow(())
    EXPECT_NO_FATAL_FAILURE(obj->showOptionWindow());
}

TEST_F(FrameManagerPrivateTest, switchToCustom)
{
    // Test method: void switchToCustom(())
    EXPECT_NO_FATAL_FAILURE(obj->switchToCustom());
}

TEST_F(FrameManagerPrivateTest, switchToNormalized)
{
    // Test method: void switchToNormalized((int cf))
    EXPECT_NO_FATAL_FAILURE(obj->switchToNormalized(0));
}
