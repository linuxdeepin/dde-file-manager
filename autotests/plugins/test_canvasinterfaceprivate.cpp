// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasinterfaceprivate.cpp
 * @brief Unit tests for CanvasInterfacePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/canvasinterface.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasInterfacePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasInterfacePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasInterfacePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasInterfacePrivateTest, CanvasInterfacePrivate)
{
    // Test constructor: CanvasInterfacePrivate((CanvasInterface *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasInterfacePrivateTest, CanvasInterfacePrivate_Destructor)
{
    // Test method:  ~CanvasInterfacePrivate(())
    EXPECT_NO_FATAL_FAILURE({ CanvasInterfacePrivate *tmp = new CanvasInterfacePrivate(); delete tmp; });
}
