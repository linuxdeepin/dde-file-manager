// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsutils.cpp
 * @brief Unit tests for AvfsUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/avfsutils.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsUtilsTest, archivePreviewEnabled)
{
    // Test bool getter: archivePreviewEnabled()
    bool result = obj->archivePreviewEnabled();
    EXPECT_FALSE(result);

}

TEST_F(AvfsUtilsTest, avfsMountPoint)
{
    // Test getter: QString avfsMountPoint()
    auto result = obj->avfsMountPoint();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AvfsUtilsTest, avfsUrlToLocal)
{
    // Test method: QUrl avfsUrlToLocal((const QUrl &avfsUrl))
    QUrl _arg0{};
    auto result = obj->avfsUrlToLocal(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(AvfsUtilsTest, isSupportedArchives)
{
    // Test method: bool isSupportedArchives((const QString &path))
    QString _arg0{};
    auto result = obj->isSupportedArchives(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AvfsUtilsTest, localUrlToAvfsUrl)
{
    // Test method: QUrl localUrlToAvfsUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->localUrlToAvfsUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(AvfsUtilsTest, parseDirIcon)
{
    // Test method: QString parseDirIcon((QString path))
    auto result = obj->parseDirIcon(QString());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AvfsUtilsTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AvfsUtilsTest, seperateUrl)
{
    // Test method: QList<QVariantMap> seperateUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->seperateUrl(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AvfsUtilsTest, instance)
{
    // Test getter: AvfsUtils instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
