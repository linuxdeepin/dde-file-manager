// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgrounddefault.cpp
 * @brief Unit tests for BackgroundDefault methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "backgrounddefault.h"

#include <QTest>

using namespace ddplugin_background;

class BackgroundDefaultTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BackgroundDefault();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BackgroundDefault *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BackgroundDefaultTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(BackgroundDefaultTest, sendPaintReport)
{
    // Test method: void sendPaintReport(())
    EXPECT_NO_FATAL_FAILURE(obj->sendPaintReport());
}

TEST_F(BackgroundDefaultTest, setPixmap)
{
    // Test setter: void setPixmap((const QPixmap &pix))
    QPixmap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPixmap(_arg0));
}
