// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "options/optionswindow_p.h"
#include "config/configpresenter.h"
#include "stubext.h"

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE

TEST(OptionsWindow, initialize_on)
{
    OptionsWindow win;
    stub_ext::StubExt stub;
    bool reset = false;
    stub.set_lamda(&OrganizationGroup::reset, [&reset](){
        reset = true;
    });

    SizeSlider::Mode mode = SizeSlider::Mode::Icon;
    stub.set_lamda(&SizeSlider::switchMode, [&mode](SizeSlider *, SizeSlider::Mode m){
        mode = m;
    });

    stub.set_lamda(&ConfigPresenter::isEnable, [](){
        return true;
    });

    stub.set_lamda(&OptionsWindowPrivate::isAutoArrange, [](){
        return false;
    });

    EXPECT_TRUE(win.initialize());

    EXPECT_NE(win.d->mainLayout, nullptr);
    EXPECT_NE(win.d->contentWidget, nullptr);
    EXPECT_NE(win.d->contentLayout, nullptr);
    EXPECT_NE(win.d->organization, nullptr);
    EXPECT_NE(win.d->sizeSlider, nullptr);
    EXPECT_TRUE(reset);
    EXPECT_EQ(mode, SizeSlider::Mode::View);

    ASSERT_NE(win.d->autoArrange, nullptr);
    EXPECT_FALSE(win.d->autoArrange->checked());
    EXPECT_TRUE(win.d->autoArrange->testAttribute(Qt::WA_WState_ExplicitShowHide));
    EXPECT_TRUE(win.d->autoArrange->testAttribute(Qt::WA_WState_Hidden));
}

TEST(OptionsWindow, initialize_off)
{
    OptionsWindow win;
    stub_ext::StubExt stub;
    bool reset = false;
    stub.set_lamda(&OrganizationGroup::reset, [&reset](){
        reset = true;
    });

    SizeSlider::Mode mode = SizeSlider::Mode::View;
    stub.set_lamda(&SizeSlider::switchMode, [&mode](SizeSlider *, SizeSlider::Mode m){
        mode = m;
    });

    stub.set_lamda(&ConfigPresenter::isEnable, [](){
        return false;
    });

    stub.set_lamda(&OptionsWindowPrivate::isAutoArrange, [](){
        return true;
    });

    EXPECT_TRUE(win.initialize());

    EXPECT_NE(win.d->mainLayout, nullptr);
    EXPECT_NE(win.d->contentWidget, nullptr);
    EXPECT_NE(win.d->contentLayout, nullptr);
    EXPECT_NE(win.d->organization, nullptr);
    EXPECT_NE(win.d->sizeSlider, nullptr);
    EXPECT_TRUE(reset);
    EXPECT_EQ(mode, SizeSlider::Mode::Icon);

    ASSERT_NE(win.d->autoArrange, nullptr);
    EXPECT_TRUE(win.d->autoArrange->checked());
    EXPECT_TRUE(win.d->autoArrange->testAttribute(Qt::WA_WState_ExplicitShowHide));
    EXPECT_FALSE(win.d->autoArrange->testAttribute(Qt::WA_WState_Hidden));
}
