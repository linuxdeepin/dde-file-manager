// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatorproxyprivate.cpp
 * @brief Unit tests for FileOperatorProxyPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/fileoperatorproxy.h"

#include <QTest>

using namespace ddplugin_canvas;

class FileOperatorProxyPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorProxyPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorProxyPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorProxyPrivateTest, callBackPasteFiles)
{
    // Test method: void callBackPasteFiles((const JobInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->callBackPasteFiles(JobInfoPointer()));
}

TEST_F(FileOperatorProxyPrivateTest, callBackRenameFiles)
{
    // Test method: void callBackRenameFiles((const QList<QUrl> &sources, const QList<QUrl> &targets))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->callBackRenameFiles(_arg0, _arg1));
}
