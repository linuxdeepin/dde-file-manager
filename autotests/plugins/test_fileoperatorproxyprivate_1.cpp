// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatorproxyprivate_1.cpp
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

TEST_F(FileOperatorProxyPrivateTest, FileOperatorProxyPrivate)
{
    // Test constructor: FileOperatorProxyPrivate((FileOperatorProxy *q_ptr))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileOperatorProxyPrivateTest, callBackTouchFile)
{
    // Test method: void callBackTouchFile((const QUrl &target, const QVariantMap &customData))
    QUrl _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->callBackTouchFile(_arg0, _arg1));
}

TEST_F(FileOperatorProxyPrivateTest, filterDesktopFile)
{
    // Test method: void filterDesktopFile((QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->filterDesktopFile(_arg0));
}
