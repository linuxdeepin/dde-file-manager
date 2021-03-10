/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "funcwrapper.h"

#include <gtest/gtest.h>

TEST(TestToBase64, testCommonVault)
{
    unsigned char data[6] = {0x12, 0x32, 0x56, 0x78, 0x90, 0xab};
    int size = 6;
    int options = 1;
    char *str = toBase64(data, size, options);
    EXPECT_STREQ("EjJWeJCr", str);
    if (str) {
        delete[] str;
        str = nullptr;
    }
}

TEST(TestToBase64, testCommonVault2)
{
    unsigned char data[7] = {0x12, 0x32, 0x56, 0x78, 0x90, 0xab, 0x11};
    int size = 7;
    int options = 2;
    char *str = toBase64(data, size, options);
    EXPECT_STRNE("EjJWeJCr", str);
    if (str) {
        delete[] str;
        str = nullptr;
    }
}
