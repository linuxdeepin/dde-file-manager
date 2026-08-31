// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filepreviewfactory.cpp
 * @brief Unit tests for FilePreviewFactory methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/libdfm-preview/pluginInterface/filepreviewfactory.h"

#include <QTest>

using namespace src;

class FilePreviewFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FilePreviewFactory();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FilePreviewFactory *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FilePreviewFactoryTest, keys)
{
    // Test getter: QStringList keys()
    auto result = obj->keys();
    EXPECT_TRUE(result.isEmpty());

}
