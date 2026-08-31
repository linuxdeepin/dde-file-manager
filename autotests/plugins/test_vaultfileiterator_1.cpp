// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfileiterator_1.cpp
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

TEST_F(VaultFileIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(VaultFileIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(VaultFileIteratorTest, initIterator)
{
    // Test bool getter: initIterator()
    bool result = obj->initIterator();
    EXPECT_FALSE(result);

}

TEST_F(VaultFileIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(VaultFileIteratorTest, VaultFileIterator_Destructor)
{
    // Test method:  ~VaultFileIterator(())
    EXPECT_NO_FATAL_FAILURE({ VaultFileIterator *tmp = new VaultFileIterator(); delete tmp; });
}
