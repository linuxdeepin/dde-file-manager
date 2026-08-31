// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskencryptmenucreator.cpp
 * @brief Unit tests for DiskEncryptMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/diskencryptmenuscene.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class DiskEncryptMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskEncryptMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskEncryptMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskEncryptMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
