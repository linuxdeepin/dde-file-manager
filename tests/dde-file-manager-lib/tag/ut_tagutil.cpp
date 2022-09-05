// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDebug>

#include "tag/tagutil.h"

using namespace Tag;

namespace  {
    class TestTagUtil : public testing::Test
    {
    public:
        void SetUp() override
        {

        }
        void TearDown() override
        {

        }
    public:

    };
}

TEST_F(TestTagUtil, test_actualAndFakerName)
{
    EXPECT_FALSE(ActualAndFakerName().isEmpty());
}

TEST_F(TestTagUtil, test_en_skim)
{
    QString baseStr("\'aaaa\'aaaa");
    QString testStr = escaping_en_skim(baseStr);
    EXPECT_NE(testStr, baseStr);
    EXPECT_NE(restore_escaped_en_skim(testStr), testStr);

    QString emptyStr;
    EXPECT_EQ(escaping_en_skim(emptyStr), emptyStr);
    EXPECT_EQ(restore_escaped_en_skim(emptyStr), emptyStr);
}
