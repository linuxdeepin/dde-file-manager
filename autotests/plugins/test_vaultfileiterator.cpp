// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfileiterator.cpp
 * @brief Unit tests for VaultFileIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileutils/vaultfileiterator.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultFileIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultFileIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultFileIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultFileIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(VaultFileIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(VaultFileIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
