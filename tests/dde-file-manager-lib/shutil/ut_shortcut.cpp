/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "shutil/shortcut.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <memory>

namespace  {
    class TestShortcut : public testing::Test {
    public:
        void SetUp() override
        {
           mShortcut.reset(new Shortcut());
        }
        void TearDown() override
        {
        }

    public:
            std::shared_ptr<Shortcut> mShortcut = nullptr;
    };
}

TEST_F(TestShortcut, can_transferred_to_string)
{
    QString value = mShortcut->toStr();
    EXPECT_TRUE( value.contains("Ctrl + Shift + Tab"));
    EXPECT_TRUE( value.contains("Ctrl + W"));
    EXPECT_TRUE( value.contains("Ctrl + C"));
    EXPECT_TRUE( value.contains("Ctrl + V"));
    EXPECT_TRUE( value.contains("Shift + Left"));
    EXPECT_TRUE( value.contains("Ctrl + A"));
    EXPECT_TRUE( value.contains("Shift + Delete"));
}
