// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
