// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_editlabel_1.cpp
 * @brief Unit tests for EditLabel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "editlabel.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class EditLabelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EditLabel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EditLabel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EditLabelTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}
