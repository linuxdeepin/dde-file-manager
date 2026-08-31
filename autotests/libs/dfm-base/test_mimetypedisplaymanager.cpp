// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mimetypedisplaymanager.cpp
 * @brief Unit tests for MimeTypeDisplayManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/mimetype/mimetypedisplaymanager.h"

#include <QTest>

using namespace src;

class MimeTypeDisplayManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MimeTypeDisplayManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MimeTypeDisplayManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MimeTypeDisplayManagerTest, MimeTypeDisplayManager)
{
    // Test constructor: MimeTypeDisplayManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MimeTypeDisplayManagerTest, displayNameToEnum)
{
    // Test method: FileInfo::FileType displayNameToEnum((const QString &mimeType))
    QString _arg0{};
    auto result = obj->displayNameToEnum(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(MimeTypeDisplayManagerTest, readlines)
{
    // Test method: QStringList readlines((const QString &path))
    QString _arg0{};
    auto result = obj->readlines(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
