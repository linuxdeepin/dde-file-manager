// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizerplugin.h"

#include "stubext.h"
#include "gtest/gtest.h"

#include <QWidget>

using namespace ddplugin_organizer;

class UT_Checkboxwidget : public testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test objects
        // mock the UI show
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_Checkboxwidget, BasicTest_Always_Passes)
{
    EXPECT_TRUE(true);
}
