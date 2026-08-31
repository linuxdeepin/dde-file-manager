// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfileinfo.cpp
 * @brief Unit tests for VaultFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileutils/vaultfileinfo.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultFileInfoTest, canAttributes)
{
    // Test method: bool canAttributes((const CanableInfoType type))
    auto result = obj->canAttributes(CanableInfoType());
    EXPECT_FALSE(result);

}

TEST_F(VaultFileInfoTest, countChildFile)
{
    // Test getter: int countChildFile()
    auto result = obj->countChildFile();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultFileInfoTest, displayOf)
{
    // Test method: QString displayOf((const DisPlayInfoType type))
    auto result = obj->displayOf(DisPlayInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultFileInfoTest, extendAttributes)
{
    // Test method: QVariant extendAttributes((const ExtInfoType type))
    auto result = obj->extendAttributes(ExtInfoType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(VaultFileInfoTest, extraProperties)
{
    // Test getter: QVariantHash extraProperties()
    auto result = obj->extraProperties();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultFileInfoTest, getUrlByType)
{
    // Test method: QUrl getUrlByType((const UrlInfoType type, const QString &fileName))
    QString _arg1{};
    auto result = obj->getUrlByType(UrlInfoType(), _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(VaultFileInfoTest, nameOf)
{
    // Test method: QString nameOf((const NameInfoType type))
    auto result = obj->nameOf(NameInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultFileInfoTest, pathOf)
{
    // Test method: QString pathOf((const PathInfoType type))
    auto result = obj->pathOf(PathInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultFileInfoTest, size)
{
    // Test getter: qint64 size()
    auto result = obj->size();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultFileInfoTest, viewOfTip)
{
    // Test method: QString viewOfTip((const FileInfo::ViewType type))
    auto result = obj->viewOfTip(FileInfo::ViewType());
    EXPECT_TRUE(result.isEmpty());

}
