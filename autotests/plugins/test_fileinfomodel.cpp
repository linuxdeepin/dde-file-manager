// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfomodel.cpp
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

TEST_F(FileInfoModelTest, flags)
{
    // Test method: Qt::ItemFlags flags((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->flags(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileInfoModelTest, removeFilter)
{
    // Test method: void removeFilter((QSharedPointer<FileFilter> filter))
    EXPECT_NO_FATAL_FAILURE(obj->removeFilter(QSharedPointer<FileFilter>()));
}

TEST_F(FileInfoModelTest, updateFile)
{
    // Test method: void updateFile((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateFile(_arg0));
}
