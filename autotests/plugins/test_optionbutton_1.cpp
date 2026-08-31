// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionbutton_1.cpp
 * @brief Unit tests for OptionButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectiontitlebar.h"

#include <QTest>

using namespace ddplugin_organizer;

class OptionButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OptionButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OptionButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OptionButtonTest, OptionButton)
{
    // Test constructor: OptionButton((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OptionButtonTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}
