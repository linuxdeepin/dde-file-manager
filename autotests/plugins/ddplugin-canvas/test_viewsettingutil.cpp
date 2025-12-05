// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/operator/viewsettingutil.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>
#include <QTimer>

using namespace ddplugin_canvas;

class UT_ViewSettingUtil : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        util = new ViewSettingUtil();
    }

    virtual void TearDown() override
    {
        if (util) {
            delete util;
            util = nullptr;
        }

        stub.clear();
    }

public:
    QApplication *app = nullptr;
    ViewSettingUtil *util = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_ViewSettingUtil, constructor_CreateUtil_InitializesCorrectly)
{
    EXPECT_NE(util, nullptr);
}

TEST_F(UT_ViewSettingUtil, checkTouchDrag_WithNullEvent_DoesNotCrash)
{
    EXPECT_NO_THROW(util->checkTouchDrag(nullptr));
}

TEST_F(UT_ViewSettingUtil, checkTouchDrag_WithMouseEvent_HandlesCorrectly)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(100, 100), QPointF(100, 100), QPointF(100, 100), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);

    EXPECT_NO_THROW(util->checkTouchDrag(&event));
}

TEST_F(UT_ViewSettingUtil, checkTouchDrag_WithRegularMouseEvent_StopsTimer)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(100, 100), QPointF(100, 100), QPointF(100, 100), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventNotSynthesized);

    EXPECT_NO_THROW(util->checkTouchDrag(&event));
}

TEST_F(UT_ViewSettingUtil, isDelayDrag_WithActiveTimer_ReturnsTrue)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(100, 100), QPointF(100, 100), QPointF(100, 100), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    util->checkTouchDrag(&event);

    bool result = util->isDelayDrag();
    EXPECT_TRUE(result);
}

TEST_F(UT_ViewSettingUtil, isDelayDrag_WithInactiveTimer_ReturnsFalse)
{
    bool result = util->isDelayDrag();
    EXPECT_FALSE(result);
}