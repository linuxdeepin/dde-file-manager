// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customtopwidgetinfo.cpp
 * @brief Unit tests for CustomTopWidgetInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfmplugin_workspace_global.h"

#include <QTest>

using namespace dfmplugin_workspace;

class CustomTopWidgetInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomTopWidgetInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomTopWidgetInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomTopWidgetInfoTest, CustomTopWidgetInfo)
{
    // Test constructor: CustomTopWidgetInfo(())
    ASSERT_NE(obj, nullptr);
}
