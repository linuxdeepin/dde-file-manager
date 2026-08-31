// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appendcompresshelper.cpp
 * @brief Unit tests for AppendCompressHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "appendcompress/appendcompresshelper.h"

#include <QTest>

using namespace dfmplugin_utils;

class AppendCompressHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppendCompressHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppendCompressHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppendCompressHelperTest, canAppendCompress)
{
    // Test method: bool canAppendCompress((const QList<QUrl> &fromUrls, const QUrl &toUrl))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->canAppendCompress(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressHelperTest, isCompressedFile)
{
    // Test method: bool isCompressedFile((const QUrl &toUrl))
    QUrl _arg0{};
    auto result = obj->isCompressedFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressHelperTest, setMouseStyle)
{
    // Test method: bool setMouseStyle((const QUrl &toUrl, const QList<QUrl> &fromUrls, Qt::DropAction *dropAction))
    QUrl _arg0{};
    QList<QUrl> _arg1{};
    auto result = obj->setMouseStyle(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}
