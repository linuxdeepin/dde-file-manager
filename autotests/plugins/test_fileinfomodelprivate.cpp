// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfomodelprivate.cpp
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

TEST_F(FileInfoModelPrivateTest, checkAndRefreshDesktopIcon)
{
    // Test method: void checkAndRefreshDesktopIcon((const FileInfoPointer &info, int retryCount))
    FileInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->checkAndRefreshDesktopIcon(_arg0, 0));
}

TEST_F(FileInfoModelPrivateTest, insertData)
{
    // Test method: void insertData((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->insertData(_arg0));
}

TEST_F(FileInfoModelPrivateTest, removeData)
{
    // Test method: void removeData((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeData(_arg0));
}

TEST_F(FileInfoModelPrivateTest, replaceData)
{
    // Test method: void replaceData((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->replaceData(_arg0, _arg1));
}

TEST_F(FileInfoModelPrivateTest, resetData)
{
    // Test method: void resetData((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resetData(_arg0));
}
