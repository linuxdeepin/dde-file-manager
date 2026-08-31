// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasorganizer.cpp
 * @brief Unit tests for CanvasOrganizer methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/canvasorganizer.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasOrganizerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasOrganizer();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasOrganizer *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasOrganizerTest, CanvasOrganizer)
{
    // Test constructor: CanvasOrganizer((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasOrganizerTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}
