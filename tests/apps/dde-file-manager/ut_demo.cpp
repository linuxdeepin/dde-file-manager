/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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
#include "uttestclass.h"

using namespace testing;

namespace  {
    class FirstTest : public Test
    {
    public:
        UtTestClass *utTest;
        virtual void SetUp() override {
            utTest = new UtTestClass();
        }

        virtual void TearDown() override {
            delete utTest;
            utTest = nullptr;
        }
    };
}

TEST_F(FirstTest, nothing)
{
    int result = utTest->sum(1,1);
    EXPECT_EQ(result, 2);
}
