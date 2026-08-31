// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appendcompresshelper_1.cpp
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

TEST_F(AppendCompressHelperTest, appendCompress)
{
    // Test method: bool appendCompress((const QString &toFilePath, const QStringList &fromFilePaths))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->appendCompress(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(AppendCompressHelperTest, dragDropCompress)
{
    // Test method: bool dragDropCompress((const QUrl &toUrl, const QList<QUrl> &fromUrls))
    QUrl _arg0{};
    QList<QUrl> _arg1{};
    auto result = obj->dragDropCompress(_arg0, _arg1);
    EXPECT_FALSE(result);

}
