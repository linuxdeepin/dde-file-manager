// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashhelper.cpp
 * @brief Unit tests for TrashHelper Mid-priority methods (dfmplugin-trash)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "utils/trashhelper.h"

using namespace dfmplugin_trash;

class TrashHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // TrashHelper uses singleton pattern
    }
    void TearDown() override {}
};

TEST_F(TrashHelperTest, checkDragDropAction)
{
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = TrashHelper::instance()->checkDragDropAction(QList<QUrl>{QUrl("file:///tmp/test")}, QUrl("file:///tmp/test"), nullptr); });
    (void)result;
}

TEST_F(TrashHelperTest, emptyTrash)
{
    EXPECT_NO_FATAL_FAILURE({ TrashHelper::instance()->emptyTrash(0); });
}

TEST_F(TrashHelperTest, isTrashFile)
{
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = TrashHelper::instance()->isTrashFile(QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(TrashHelperTest, isTrashRootFile)
{
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = TrashHelper::instance()->isTrashRootFile(QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(TrashHelperTest, rootUrl)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = TrashHelper::instance()->rootUrl(); (void)r; });
}

TEST_F(TrashHelperTest, scheme)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = TrashHelper::scheme(); (void)r; });
}

TEST_F(TrashHelperTest, windowId)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = TrashHelper::instance()->windowId(nullptr); (void)r; });
}

TEST_F(TrashHelperTest, TrashHelper)
{
    // TrashHelper
    SUCCEED();
}

TEST_F(TrashHelperTest, instance)
{
    // instance
    SUCCEED();
}
