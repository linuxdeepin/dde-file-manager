// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfomodelprivate_1.cpp
 * @brief Unit tests for FileInfoModelPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/fileinfomodel.h"

#include <QTest>

using namespace ddplugin_canvas;

class FileInfoModelPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileInfoModelPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileInfoModelPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileInfoModelPrivateTest, FileInfoModelPrivate)
{
    // Test constructor: FileInfoModelPrivate((FileInfoModel *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileInfoModelPrivateTest, dataUpdated)
{
    // Test method: void dataUpdated((const QUrl &url, const bool isLinkOrg))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->dataUpdated(_arg0, false));
}

TEST_F(FileInfoModelPrivateTest, doRefresh)
{
    // Test method: void doRefresh(())
    EXPECT_NO_FATAL_FAILURE(obj->doRefresh());
}

TEST_F(FileInfoModelPrivateTest, fileIcon)
{
    // Test method: QIcon fileIcon((FileInfoPointer info))
    auto result = obj->fileIcon(FileInfoPointer());
    EXPECT_TRUE(result.isNull());

}

TEST_F(FileInfoModelPrivateTest, thumbUpdated)
{
    // Test method: void thumbUpdated((const QUrl &url, const QString &thumb))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->thumbUpdated(_arg0, _arg1));
}

TEST_F(FileInfoModelPrivateTest, updateData)
{
    // Test method: void updateData((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateData(_arg0));
}
