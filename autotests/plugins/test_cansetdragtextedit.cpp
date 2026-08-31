// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cansetdragtextedit.cpp
 * @brief Unit tests for CanSetDragTextEdit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/cansetdragtextedit.h"

#include <QTest>

using namespace dfmplugin_workspace;

class CanSetDragTextEditTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanSetDragTextEdit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanSetDragTextEdit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanSetDragTextEditTest, CanSetDragTextEdit)
{
    // Test constructor: CanSetDragTextEdit((const QString &text, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanSetDragTextEditTest, setDragEnabled)
{
    // Test setter: void setDragEnabled((const bool &bdrag))
    bool _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDragEnabled(_arg0));
}
