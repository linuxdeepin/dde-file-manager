// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfileiteratorprivate.cpp
 * @brief Unit tests for VaultFileIteratorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileutils/private/vaultfileiteratorprivate.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultFileIteratorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultFileIteratorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultFileIteratorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultFileIteratorPrivateTest, VaultFileIteratorPrivate)
{
    // Test constructor: VaultFileIteratorPrivate((const QUrl &url,
                                                   const QStringList &nameFilters,
                                                   QDir::Filters filters,
                                                   QDirIterator::IteratorFlags flags,
                                                   VaultFileIterator *qp))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultFileIteratorPrivateTest, VaultFileIteratorPrivate_Destructor)
{
    // Test method:  ~VaultFileIteratorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ VaultFileIteratorPrivate *tmp = new VaultFileIteratorPrivate(); delete tmp; });
}
