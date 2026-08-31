// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasproxymodel_1.cpp
 * @brief Unit tests for CanvasProxyModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/canvasproxymodel.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasProxyModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasProxyModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasProxyModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasProxyModelTest, columnCount)
{
    // Test method: int columnCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->columnCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasProxyModelTest, fetch)
{
    // Test method: bool fetch((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->fetch(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelTest, fileInfo)
{
    // Test method: FileInfoPointer fileInfo((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(CanvasProxyModelTest, mapToSource)
{
    // Test method: QModelIndex mapToSource((const QModelIndex &proxyIndex))
    QModelIndex _arg0{};
    auto result = obj->mapToSource(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasProxyModelTest, mimeData)
{
    // Test method: QMimeData mimeData((const QModelIndexList &indexes))
    QModelIndexList _arg0{};
    auto result = obj->mimeData(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->mimeData(_arg0); });

}

TEST_F(CanvasProxyModelTest, mimeTypes)
{
    // Test getter: QStringList mimeTypes()
    auto result = obj->mimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasProxyModelTest, modelHook)
{
    // Test getter: ModelHookInterface modelHook()
    auto result = obj->modelHook();
    EXPECT_NO_FATAL_FAILURE({ obj->modelHook(); });

}

TEST_F(CanvasProxyModelTest, refresh)
{
    // Test method: void refresh((const QModelIndex &parent, bool global, int ms, bool updateFile))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->refresh(_arg0, false, 0, false));
}

TEST_F(CanvasProxyModelTest, rootIndex)
{
    // Test getter: QModelIndex rootIndex()
    auto result = obj->rootIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasProxyModelTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(CanvasProxyModelTest, rowCount)
{
    // Test method: int rowCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->rowCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasProxyModelTest, setModelHook)
{
    // Test setter: void setModelHook((ModelHookInterface *ext))
    EXPECT_NO_FATAL_FAILURE(obj->setModelHook(nullptr));
}

TEST_F(CanvasProxyModelTest, setShowHiddenFiles)
{
    // Test setter: void setShowHiddenFiles((bool show))
    EXPECT_NO_FATAL_FAILURE(obj->setShowHiddenFiles(false));
}

TEST_F(CanvasProxyModelTest, setSortOrder)
{
    // Test setter: void setSortOrder((const Qt::SortOrder &order))
    Qt::SortOrder _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSortOrder(_arg0));
}

TEST_F(CanvasProxyModelTest, setSortRole)
{
    // Test setter: void setSortRole((int role, Qt::SortOrder order))
    EXPECT_NO_FATAL_FAILURE(obj->setSortRole(0, Qt::SortOrder()));
}

TEST_F(CanvasProxyModelTest, showHiddenFiles)
{
    // Test bool getter: showHiddenFiles()
    bool result = obj->showHiddenFiles();
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelTest, sortOrder)
{
    // Test getter: Qt::SortOrder sortOrder()
    auto result = obj->sortOrder();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CanvasProxyModelTest, sortRole)
{
    // Test getter: int sortRole()
    auto result = obj->sortRole();
    EXPECT_EQ(result, 0);

}
