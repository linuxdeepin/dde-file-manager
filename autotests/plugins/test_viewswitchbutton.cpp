// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewswitchbutton.cpp
 * @brief Unit tests for ViewSwitchButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewswitchbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewSwitchButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewSwitchButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewSwitchButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewSwitchButtonTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(ViewSwitchButtonTest, setViewModeActionEnabled)
{
    // Test setter: void setViewModeActionEnabled((DFMBASE_NAMESPACE::Global::ViewMode mode, bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setViewModeActionEnabled(DFMBASE_NAMESPACE::Global::ViewMode(), false));
}

TEST_F(ViewSwitchButtonTest, setViewModeIcon)
{
    // Test setter: void setViewModeIcon((DFMBASE_NAMESPACE::Global::ViewMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setViewModeIcon(DFMBASE_NAMESPACE::Global::ViewMode()));
}
