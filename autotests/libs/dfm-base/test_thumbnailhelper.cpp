// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_thumbnailhelper.cpp
 * @brief Unit tests for ThumbnailHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/thumbnail/thumbnailhelper.h"

#include <QTest>

using namespace src;

class ThumbnailHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ThumbnailHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ThumbnailHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ThumbnailHelperTest, checkMimeTypeSupport)
{
    // Test method: bool checkMimeTypeSupport((const QMimeType &mime))
    QMimeType _arg0{};
    auto result = obj->checkMimeTypeSupport(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ThumbnailHelperTest, checkThumbEnable)
{
    // Test method: bool checkThumbEnable((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->checkThumbEnable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ThumbnailHelperTest, dataToMd5Hex)
{
    // Test method: QByteArray dataToMd5Hex((const QByteArray &data))
    QByteArray _arg0{};
    auto result = obj->dataToMd5Hex(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ThumbnailHelperTest, defaultThumbnailDirs)
{
    // Test getter: QStringList defaultThumbnailDirs()
    auto result = obj->defaultThumbnailDirs();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ThumbnailHelperTest, saveThumbnail)
{
    // Test method: QString saveThumbnail((const QUrl &url, const QImage &img, ThumbnailSize size))
    QUrl _arg0{};
    QImage _arg1{};
    auto result = obj->saveThumbnail(_arg0, _arg1, ThumbnailSize());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ThumbnailHelperTest, sizeToFilePath)
{
    // Test method: QString sizeToFilePath((ThumbnailSize size))
    auto result = obj->sizeToFilePath(ThumbnailSize());
    EXPECT_TRUE(result.isEmpty());

}
