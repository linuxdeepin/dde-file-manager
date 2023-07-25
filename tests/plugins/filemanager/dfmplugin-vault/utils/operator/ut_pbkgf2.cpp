// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/operator/pbkdf2.h"

#include <gtest/gtest.h>

#include <QString>

TEST(UT_PBKDF2, octalToHexadecimal)
{
    char *re = pbkdf2::octalToHexadecimal("123", 3);

    EXPECT_TRUE(QString(re) == "313233");
}

TEST(UT_PBKDF2, createRandomSalt)
{
    QString re = pbkdf2::createRandomSalt(10);

    EXPECT_TRUE(re.length() == 10);
}

TEST(UT_PBKDF2, pbkdf2EncrypyPassword_one)
{
    QString re = pbkdf2::pbkdf2EncrypyPassword("UT_TEST", "UT_TEST", 1020, 19);

    EXPECT_TRUE(re.isEmpty());
}

TEST(UT_PBKDF2, pbkdf2EncrypyPassword_two)
{
    QString re = pbkdf2::pbkdf2EncrypyPassword("UT_TEST", "UT_TEST", 1020, 10);

    EXPECT_TRUE(re == "d889b52a4a");
}
