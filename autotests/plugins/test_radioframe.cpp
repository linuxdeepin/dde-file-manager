// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_radioframe.cpp
 * @brief Unit tests for RadioFrame methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/radioframe.h"

#include <QTest>

using namespace dfmplugin_vault;

class RadioFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RadioFrame();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RadioFrame *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RadioFrameTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}
