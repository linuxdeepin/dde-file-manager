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

#include "shutil/dfmregularexpression.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QRegularExpression>

namespace  {
    class TestDFMRegularExpression : public testing::Test {
    public:
        void SetUp() override
        {
        }
        void TearDown() override
        {
        }

    };
}

TEST_F(TestDFMRegularExpression, normal_path_check)
{
    QString oneKeyWord = "matchkey";
    QString newMatchKey = DFMRegularExpression::checkWildcardAndToRegularExpression(oneKeyWord);

    EXPECT_TRUE(newMatchKey.contains(oneKeyWord));

    newMatchKey = DFMRegularExpression::wildcardToRegularExpression(newMatchKey);

    EXPECT_FALSE(newMatchKey.contains(oneKeyWord));
}
