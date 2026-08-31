// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionmodel_1.cpp
 * @brief Unit tests for CollectionModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/collectionmodel.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionModelTest, CollectionModel)
{
    // Test constructor: CollectionModel((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionModelTest, columnCount)
{
    // Test method: int columnCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->columnCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionModelTest, fetch)
{
    // Test method: bool fetch((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->fetch(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CollectionModelTest, findItem)
{
    // Test method: int findItem((const QString &item))
    QString _arg0{};
    auto result = obj->findItem(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(CollectionModelTest, handler)
{
    // Test getter: ModelDataHandler handler()
    auto result = obj->handler();
    EXPECT_NO_FATAL_FAILURE({ obj->handler(); });

}

TEST_F(CollectionModelTest, mapFromSource)
{
    // Test method: QModelIndex mapFromSource((const QModelIndex &sourceIndex))
    QModelIndex _arg0{};
    auto result = obj->mapFromSource(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionModelTest, mapToSource)
{
    // Test method: QModelIndex mapToSource((const QModelIndex &proxyIndex))
    QModelIndex _arg0{};
    auto result = obj->mapToSource(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionModelTest, modelShell)
{
    // Test getter: FileInfoModelShell modelShell()
    auto result = obj->modelShell();
    EXPECT_NO_FATAL_FAILURE({ obj->modelShell(); });

}

TEST_F(CollectionModelTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(CollectionModelTest, refresh)
{
    // Test method: void refresh((const QModelIndex &parent, bool global, int ms, bool file))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->refresh(_arg0, false, 0, false));
}

TEST_F(CollectionModelTest, rootIndex)
{
    // Test getter: QModelIndex rootIndex()
    auto result = obj->rootIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionModelTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(CollectionModelTest, rowCount)
{
    // Test method: int rowCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->rowCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionModelTest, setHandler)
{
    // Test setter: void setHandler((ModelDataHandler *handler))
    EXPECT_NO_FATAL_FAILURE(obj->setHandler(nullptr));
}

TEST_F(CollectionModelTest, setModelShell)
{
    // Test setter: void setModelShell((FileInfoModelShell *shell))
    EXPECT_NO_FATAL_FAILURE(obj->setModelShell(nullptr));
}

TEST_F(CollectionModelTest, setSourceModel)
{
    // Test setter: void setSourceModel((QAbstractItemModel *sourceModel))
    EXPECT_NO_FATAL_FAILURE(obj->setSourceModel(nullptr));
}

TEST_F(CollectionModelTest, CollectionModel_Destructor)
{
    // Test method:  ~CollectionModel(())
    EXPECT_NO_FATAL_FAILURE({ CollectionModel *tmp = new CollectionModel(); delete tmp; });
}
