/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
