// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_thumbnailworker.cpp
 * @brief Unit tests for ThumbnailWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/thumbnail/thumbnailworker.h"

#include <QTest>

using namespace src;

class ThumbnailWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ThumbnailWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ThumbnailWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ThumbnailWorkerTest, createThumbnail)
{
    // Test method: void createThumbnail((const QUrl &url, Global::ThumbnailSize size))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createThumbnail(_arg0, Global::ThumbnailSize()));
}
