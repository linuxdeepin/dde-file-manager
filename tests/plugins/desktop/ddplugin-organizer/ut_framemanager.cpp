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

#include "private/framemanager_p.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

TEST(FrameManagerPrivate, showOptionWindow)
{
    FrameManagerPrivate obj(nullptr);
    ASSERT_EQ(obj.options, nullptr);

    stub_ext::StubExt stub;
    bool show = false;
    stub.set_lamda(&QWidget::show, [&show](){
        show = true;
    });

    obj.showOptionWindow();

    ASSERT_NE(obj.options, nullptr);
    EXPECT_TRUE(show);

    bool ac = false;
    stub.set_lamda(&QWidget::activateWindow, [&ac](){
        ac = true;
    });

    show = false;
    obj.showOptionWindow();
    auto old = obj.options;

    EXPECT_EQ(obj.options, old);
    EXPECT_TRUE(ac);
    EXPECT_FALSE(show);
}

