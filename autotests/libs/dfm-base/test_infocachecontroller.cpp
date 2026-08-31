// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_infocachecontroller.cpp
 * @brief Unit tests for InfoCacheController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/infocache.h"

#include <QTest>

using namespace src;

class InfoCacheControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new InfoCacheController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    InfoCacheController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(InfoCacheControllerTest, cacheDisable)
{
    // Test method: bool cacheDisable((const QString &scheme))
    QString _arg0{};
    auto result = obj->cacheDisable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(InfoCacheControllerTest, getCacheInfo)
{
    // Test method: FileInfoPointer getCacheInfo((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getCacheInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(InfoCacheControllerTest, setCacheDisbale)
{
    // Test setter: void setCacheDisbale((const QString &scheme, bool disable))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCacheDisbale(_arg0, false));
}
