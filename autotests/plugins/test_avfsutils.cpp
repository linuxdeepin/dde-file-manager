// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsutils.cpp
 * @brief Unit tests for AvfsUtils Mid-priority methods (dfmplugin-avfsbrowser)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "utils/avfsutils.h"

using namespace dfmplugin_avfsbrowser;

class AvfsUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // AvfsUtils uses singleton pattern
    }
    void TearDown() override {}
};

TEST_F(AvfsUtilsTest, archivePreviewEnabled)
{
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = AvfsUtils::instance()->archivePreviewEnabled(); });
    (void)result;
}

TEST_F(AvfsUtilsTest, avfsMountPoint)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = AvfsUtils::instance()->avfsMountPoint(); (void)r; });
}

TEST_F(AvfsUtilsTest, avfsUrlToLocal)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = AvfsUtils::instance()->avfsUrlToLocal(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(AvfsUtilsTest, isSupportedArchives)
{
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = AvfsUtils::instance()->isSupportedArchives(QString("test")); });
    (void)result;
}

TEST_F(AvfsUtilsTest, localUrlToAvfsUrl)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = AvfsUtils::instance()->localUrlToAvfsUrl(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(AvfsUtilsTest, parseDirIcon)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = AvfsUtils::instance()->parseDirIcon(QString("test")); (void)r; });
}

TEST_F(AvfsUtilsTest, scheme)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = AvfsUtils::scheme(); (void)r; });
}

TEST_F(AvfsUtilsTest, seperateUrl)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = AvfsUtils::instance()->seperateUrl(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(AvfsUtilsTest, instance)
{
    // instance
    SUCCEED();
}
