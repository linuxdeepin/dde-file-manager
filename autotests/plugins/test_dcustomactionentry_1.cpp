// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionentry_1.cpp
 * @brief Unit tests for DCustomActionEntry methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactiondata.h"

#include <QTest>

using namespace dfmplugin_menu;

class DCustomActionEntryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DCustomActionEntry();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DCustomActionEntry *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DCustomActionEntryTest, DCustomActionEntry)
{
    // Test constructor: DCustomActionEntry((const DCustomActionEntry &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DCustomActionEntryTest, operator=)
{
    // Test getter: DCustomActionEntry operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(DCustomActionEntryTest, package)
{
    // Test getter: QString package()
    auto result = obj->package();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, version)
{
    // Test getter: QString version()
    auto result = obj->version();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, comment)
{
    // Test getter: QString comment()
    auto result = obj->comment();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, data)
{
    // Test getter: DCustomActionData data()
    auto result = obj->data();
    EXPECT_NO_FATAL_FAILURE({ obj->data(); });

}

TEST_F(DCustomActionEntryTest, fileCombo)
{
    // Test getter: DCustomActionDefines::ComboTypes fileCombo()
    auto result = obj->fileCombo();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionEntryTest, mimeTypes)
{
    // Test getter: QStringList mimeTypes()
    auto result = obj->mimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, surpportSchemes)
{
    // Test getter: QStringList surpportSchemes()
    auto result = obj->surpportSchemes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, supportStuffix)
{
    // Test getter: QStringList supportStuffix()
    auto result = obj->supportStuffix();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, excludeMimeTypes)
{
    // Test getter: QStringList excludeMimeTypes()
    auto result = obj->excludeMimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, notShowIn)
{
    // Test getter: QStringList notShowIn()
    auto result = obj->notShowIn();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, packageName)
{
    // Test getter: QString packageName()
    auto result = obj->packageName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, packageVersion)
{
    // Test getter: QString packageVersion()
    auto result = obj->packageVersion();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, packageComment)
{
    // Test getter: QString packageComment()
    auto result = obj->packageComment();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, packageSign)
{
    // Test getter: QString packageSign()
    auto result = obj->packageSign();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, actionData)
{
    // Test getter: DCustomActionData actionData()
    auto result = obj->actionData();
    EXPECT_NO_FATAL_FAILURE({ obj->actionData(); });

}

TEST_F(DCustomActionEntryTest, actionFileCombo)
{
    // Test getter: DCustomActionDefines::ComboTypes actionFileCombo()
    auto result = obj->actionFileCombo();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionEntryTest, actionMimeTypes)
{
    // Test getter: QStringList actionMimeTypes()
    auto result = obj->actionMimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, actionSupportSchemes)
{
    // Test getter: QStringList actionSupportSchemes()
    auto result = obj->actionSupportSchemes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, actionSupportSuffix)
{
    // Test getter: QStringList actionSupportSuffix()
    auto result = obj->actionSupportSuffix();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, actionExcludeMimeTypes)
{
    // Test getter: QStringList actionExcludeMimeTypes()
    auto result = obj->actionExcludeMimeTypes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionEntryTest, actionNotShowIn)
{
    // Test getter: QStringList actionNotShowIn()
    auto result = obj->actionNotShowIn();
    EXPECT_TRUE(result.isEmpty());

}
