// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QString>
#include <QUrl>

#include <dfm-base/utils/traversaldirthread.h>

using namespace dfmbase;

TEST(TraversalDirThreadR15Test, SetQueryAttributes)
{
    TraversalDirThread t(QUrl::fromLocalFile("/tmp"));
    EXPECT_NO_FATAL_FAILURE({ t.setQueryAttributes("test_attribute"); });
}

TEST(TraversalDirThreadR15Test, SetEnableSortAndCheck)
{
    TraversalDirThread t(QUrl::fromLocalFile("/tmp"));
    EXPECT_FALSE(t.isSortEnabled());
    t.setEnableSort(true);
    EXPECT_TRUE(t.isSortEnabled());
    t.setEnableSort(false);
    EXPECT_FALSE(t.isSortEnabled());
}

TEST(TraversalDirThreadR15Test, D0DestructorPath)
{
    auto *ptr = new TraversalDirThread(QUrl::fromLocalFile("/tmp"));
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}
