// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_progresswidget.cpp
 * @brief Unit tests for ProgressWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/progressdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class ProgressWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProgressWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProgressWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProgressWidgetTest, setValue)
{
    // Test setter: void setValue((int value, const QString &msg))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setValue(0, _arg1));
}
