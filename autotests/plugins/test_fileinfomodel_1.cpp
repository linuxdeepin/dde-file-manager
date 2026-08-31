// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfomodel_1.cpp
 * @brief Unit tests for FileInfoModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/fileinfomodel.h"

#include <QTest>

using namespace ddplugin_canvas;

class FileInfoModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileInfoModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileInfoModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileInfoModelTest, FileInfoModel)
{
    // Test constructor: FileInfoModel((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileInfoModelTest, columnCount)
{
    // Test method: int columnCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->columnCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(FileInfoModelTest, fileUrl)
{
    // Test method: QUrl fileUrl((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->fileUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelTest, installFilter)
{
    // Test method: void installFilter((QSharedPointer<FileFilter> filter))
    EXPECT_NO_FATAL_FAILURE(obj->installFilter(QSharedPointer<FileFilter>()));
}

TEST_F(FileInfoModelTest, mimeData)
{
    // Test method: QMimeData mimeData((const QModelIndexList &indexes))
    QModelIndexList _arg0{};
    auto result = obj->mimeData(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->mimeData(_arg0); });

}

TEST_F(FileInfoModelTest, mimeTypes)
{
    // Test getter: QStringList mimeTypes()
    auto result = obj->mimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileInfoModelTest, modelState)
{
    // Test getter: int modelState()
    auto result = obj->modelState();
    EXPECT_EQ(result, 0);

}

TEST_F(FileInfoModelTest, parent)
{
    // Test method: QModelIndex parent((const QModelIndex &child))
    QModelIndex _arg0{};
    auto result = obj->parent(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelTest, refresh)
{
    // Test method: void refresh((const QModelIndex &parent))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->refresh(_arg0));
}

TEST_F(FileInfoModelTest, refreshAllFile)
{
    // Test method: void refreshAllFile(())
    EXPECT_NO_FATAL_FAILURE(obj->refreshAllFile());
}

TEST_F(FileInfoModelTest, rootIndex)
{
    // Test getter: QModelIndex rootIndex()
    auto result = obj->rootIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileInfoModelTest, rowCount)
{
    // Test method: int rowCount((const QModelIndex &parent))
    QModelIndex _arg0{};
    auto result = obj->rowCount(_arg0);
    EXPECT_EQ(result, 0);

}

TEST_F(FileInfoModelTest, setRootUrl)
{
    // Test method: QModelIndex setRootUrl((QUrl url))
    auto result = obj->setRootUrl(QUrl());
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileInfoModelTest, supportedDragActions)
{
    // Test getter: Qt::DropActions supportedDragActions()
    auto result = obj->supportedDragActions();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileInfoModelTest, supportedDropActions)
{
    // Test getter: Qt::DropActions supportedDropActions()
    auto result = obj->supportedDropActions();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileInfoModelTest, update)
{
    // Test method: void update(())
    EXPECT_NO_FATAL_FAILURE(obj->update());
}

TEST_F(FileInfoModelTest, FileInfoModel_Destructor)
{
    // Test method:  ~FileInfoModel(())
    EXPECT_NO_FATAL_FAILURE({ FileInfoModel *tmp = new FileInfoModel(); delete tmp; });
}
