// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/grouppolicy.h"

#include <gtest/gtest.h>

class Ut_GroupPolicy: public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(Ut_GroupPolicy, tst_addSyncFunc) {
    EXPECT_TRUE(GroupPolicy::instance()->addSyncFunc("hello", [](const QVariant &){}));
    EXPECT_FALSE(GroupPolicy::instance()->addSyncFunc("hello", [](const QVariant &){}));
    EXPECT_TRUE(GroupPolicy::instance()->addSyncFunc("world", [](const QVariant &){}));
}
