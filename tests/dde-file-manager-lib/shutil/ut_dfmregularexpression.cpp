// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
