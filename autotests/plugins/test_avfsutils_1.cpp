// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsutils_1.cpp
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

TEST_F(AvfsUtilsTest, AvfsUtils)
{
    // Test constructor: AvfsUtils((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AvfsUtilsTest, isAvfsMounted)
{
    // Test bool getter: isAvfsMounted()
    bool result = obj->isAvfsMounted();
    EXPECT_FALSE(result);

}

TEST_F(AvfsUtilsTest, localArchiveToAvfsUrl)
{
    // Test method: QUrl localArchiveToAvfsUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->localArchiveToAvfsUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(AvfsUtilsTest, makeAvfsUrl)
{
    // Test method: QUrl makeAvfsUrl((const QString &path))
    QString _arg0{};
    auto result = obj->makeAvfsUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(AvfsUtilsTest, mountAvfs)
{
    // Test method: void mountAvfs(())
    EXPECT_NO_FATAL_FAILURE(obj->mountAvfs());
}

TEST_F(AvfsUtilsTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(AvfsUtilsTest, supportedArchives)
{
    // Test getter: QStringList supportedArchives()
    auto result = obj->supportedArchives();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AvfsUtilsTest, unmountAvfs)
{
    // Test method: void unmountAvfs(())
    EXPECT_NO_FATAL_FAILURE(obj->unmountAvfs());
}
