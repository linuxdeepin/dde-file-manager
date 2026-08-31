// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfileinfoprivate_1.cpp
 * @brief Unit tests for VaultFileInfoPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileutils/vaultfileinfo.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultFileInfoPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultFileInfoPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultFileInfoPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultFileInfoPrivateTest, VaultFileInfoPrivate)
{
    // Test constructor: VaultFileInfoPrivate((const QUrl &url, VaultFileInfo *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultFileInfoPrivateTest, fileDisplayPath)
{
    // Test getter: QString fileDisplayPath()
    auto result = obj->fileDisplayPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultFileInfoPrivateTest, getUrlByNewFileName)
{
    // Test method: QUrl getUrlByNewFileName((const QString &fileName))
    QString _arg0{};
    auto result = obj->getUrlByNewFileName(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(VaultFileInfoPrivateTest, vaultUrl)
{
    // Test method: QUrl vaultUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->vaultUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(VaultFileInfoPrivateTest, VaultFileInfoPrivate_Destructor)
{
    // Test method:  ~VaultFileInfoPrivate(())
    EXPECT_NO_FATAL_FAILURE({ VaultFileInfoPrivate *tmp = new VaultFileInfoPrivate(); delete tmp; });
}
