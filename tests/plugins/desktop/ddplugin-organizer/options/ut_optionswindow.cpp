/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
