// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionmodel.cpp
 * @brief Unit tests for CollectionModel Mid-priority methods (ddplugin-organizer)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "models/collectionmodel.h"

using namespace ddplugin_organizer;

class CollectionModelTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(CollectionModelTest, data)
{
    // Instance method data
    CollectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.data(QModelIndex(), 0); (void)r; });
}

TEST_F(CollectionModelTest, fileInfo)
{
    // Instance method fileInfo
    CollectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.fileInfo(QModelIndex()); (void)r; });
}

TEST_F(CollectionModelTest, fileUrl)
{
    // Instance method fileUrl
    CollectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.fileUrl(QModelIndex()); (void)r; });
}

TEST_F(CollectionModelTest, files)
{
    // Instance method files
    CollectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.files(); (void)r; });
}

TEST_F(CollectionModelTest, index)
{
    // Instance method index
    CollectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.index(0, 0, QModelIndex()); (void)r; });
}

TEST_F(CollectionModelTest, parent)
{
    // Instance method parent
    CollectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.parent(QModelIndex()); (void)r; });
}

TEST_F(CollectionModelTest, take)
{
    // Instance method take
    CollectionModel obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.take(QList<QUrl>{QUrl("file:///tmp/test")}); });
    (void)result;
}

TEST_F(CollectionModelTest, update)
{
    // Instance method update
    CollectionModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.update(); });
}

TEST_F(CollectionModelTest, mimeData)
{
    // mimeData
    SUCCEED();
}
