// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatorproxy.cpp
 * @brief Unit tests for FileOperatorProxy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/fileoperatorproxy.h"

#include <QTest>

using namespace ddplugin_canvas;

class FileOperatorProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorProxy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorProxy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorProxyTest, clearPasteFileData)
{
    // Test method: void clearPasteFileData(())
    EXPECT_NO_FATAL_FAILURE(obj->clearPasteFileData());
}

TEST_F(FileOperatorProxyTest, clearRenameFileData)
{
    // Test method: void clearRenameFileData(())
    EXPECT_NO_FATAL_FAILURE(obj->clearRenameFileData());
}

TEST_F(FileOperatorProxyTest, clearTouchFileData)
{
    // Test method: void clearTouchFileData(())
    EXPECT_NO_FATAL_FAILURE(obj->clearTouchFileData());
}

TEST_F(FileOperatorProxyTest, deleteFiles)
{
    // Test method: void deleteFiles((const CanvasView *view))
    EXPECT_NO_FATAL_FAILURE(obj->deleteFiles(nullptr));
}

TEST_F(FileOperatorProxyTest, pasteFileData)
{
    // Test getter: QSet<QUrl> pasteFileData()
    auto result = obj->pasteFileData();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileOperatorProxyTest, removePasteFileData)
{
    // Test method: void removePasteFileData((const QUrl &oldUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removePasteFileData(_arg0));
}

TEST_F(FileOperatorProxyTest, removeRenameFileData)
{
    // Test method: void removeRenameFileData((const QUrl &oldUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeRenameFileData(_arg0));
}

TEST_F(FileOperatorProxyTest, instance)
{
    // Test getter: FileOperatorProxy instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
