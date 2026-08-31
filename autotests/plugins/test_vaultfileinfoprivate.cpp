// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfileinfoprivate.cpp
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

TEST_F(VaultFileInfoPrivateTest, absolutePath)
{
    // Test method: QString absolutePath((const QString &path))
    QString _arg0{};
    auto result = obj->absolutePath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
