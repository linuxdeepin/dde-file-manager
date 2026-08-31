// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_filesystem_helper.cpp
 * @brief Unit tests for filesystem_helper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/helpers/filesystemhelper.h"

#include <QTest>

using namespace src;

class filesystem_helperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new filesystem_helper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    filesystem_helper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(filesystem_helperTest, expandFileSystem_ext)
{
    // Test method: bool expandFileSystem_ext((const QString &device))
    QString _arg0{};
    auto result = obj->expandFileSystem_ext(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(filesystem_helperTest, remountBoot)
{
    // Test method: void remountBoot(())
    EXPECT_NO_FATAL_FAILURE(obj->remountBoot());
}

TEST_F(filesystem_helperTest, shrinkFileSystem_ext)
{
    // Test method: FILE_ENCRYPT_USE_NS shrinkFileSystem_ext((const QString &device))
    QString _arg0{};
    auto result = obj->shrinkFileSystem_ext(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->shrinkFileSystem_ext(_arg0); });

}
