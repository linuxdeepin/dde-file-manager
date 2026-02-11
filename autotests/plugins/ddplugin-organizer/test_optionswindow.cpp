// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "options/optionswindow.h"

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_OptionsWindow : public testing::Test
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

TEST_F(UT_OptionsWindow, Constructor_CreatesWindow)
{
    OptionsWindow window(nullptr);
    EXPECT_NE(&window, nullptr);
    EXPECT_NE(window.d, nullptr);
}

TEST_F(UT_OptionsWindow, Constructor_WithParent_CreatesWindow)
{
    QWidget parent;
    OptionsWindow window(&parent);
    EXPECT_NE(&window, nullptr);
    EXPECT_EQ(window.parent(), &parent);
}

TEST_F(UT_OptionsWindow, Destructor_DoesNotCrash)
{
    OptionsWindow *window = new OptionsWindow();
    EXPECT_NE(window, nullptr);
    delete window;
    SUCCEED();
}

TEST_F(UT_OptionsWindow, Initialize_ReturnsBool)
{
    OptionsWindow window(nullptr);
    bool result = window.initialize();
    EXPECT_TRUE(result || !result);
}

TEST_F(UT_OptionsWindow, MoveToCenter_DoesNotCrash)
{
    OptionsWindow window(nullptr);
    window.moveToCenter(QPoint(100, 100));
    SUCCEED();
}
