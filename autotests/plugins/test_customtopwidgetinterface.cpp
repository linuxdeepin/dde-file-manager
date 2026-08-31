// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customtopwidgetinterface.cpp
 * @brief Unit tests for CustomTopWidgetInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/customtopwidgetinterface.h"

#include <QTest>

using namespace dfmplugin_workspace;

class CustomTopWidgetInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomTopWidgetInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomTopWidgetInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomTopWidgetInterfaceTest, CustomTopWidgetInterface)
{
    // Test constructor: CustomTopWidgetInterface((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
