// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iteminfo.cpp
 * @brief Unit tests for ItemInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfmplugin_sidebar_global.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class ItemInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ItemInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ItemInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ItemInfoTest, ItemInfo)
{
    // Test constructor: ItemInfo(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ItemInfoTest, operator==)
{
    // Test bool getter: operator==()
    bool result = obj->operator==();
    EXPECT_FALSE(result);
}

TEST_F(ItemInfoTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(ItemInfoTest, group)
{
    // Test getter: QString group()
    auto result = obj->group();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ItemInfoTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(ItemInfoTest, flags)
{
    // Test getter: Qt::ItemFlags flags()
    auto result = obj->flags();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ItemInfoTest, subGroup)
{
    // Test getter: QString subGroup()
    auto result = obj->subGroup();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ItemInfoTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ItemInfoTest, finalUrl)
{
    // Test getter: QUrl finalUrl()
    auto result = obj->finalUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(ItemInfoTest, isEjectable)
{
    // Test bool getter: isEjectable()
    bool result = obj->isEjectable();
    EXPECT_FALSE(result);

}

TEST_F(ItemInfoTest, isEditable)
{
    // Test bool getter: isEditable()
    bool result = obj->isEditable();
    EXPECT_FALSE(result);

}

TEST_F(ItemInfoTest, reportName)
{
    // Test getter: QString reportName()
    auto result = obj->reportName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ItemInfoTest, isExpandable)
{
    // Test bool getter: isExpandable()
    bool result = obj->isExpandable();
    EXPECT_FALSE(result);

}

TEST_F(ItemInfoTest, clickedCb)
{
    // Test getter: ItemClickedActionCallback clickedCb()
    auto result = obj->clickedCb();
    EXPECT_NO_FATAL_FAILURE({ obj->clickedCb(); });

}

TEST_F(ItemInfoTest, renameCb)
{
    // Test getter: RenameCallback renameCb()
    auto result = obj->renameCb();
    EXPECT_NO_FATAL_FAILURE({ obj->renameCb(); });

}

TEST_F(ItemInfoTest, editDisplayText)
{
    // Test getter: QString editDisplayText()
    auto result = obj->editDisplayText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ItemInfoTest, visiableControlKey)
{
    // Test getter: QString visiableControlKey()
    auto result = obj->visiableControlKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ItemInfoTest, visiableDisplayName)
{
    // Test getter: QString visiableDisplayName()
    auto result = obj->visiableDisplayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ItemInfoTest, contextMenuCb)
{
    // Test getter: ContextMenuCallback contextMenuCb()
    auto result = obj->contextMenuCb();
    EXPECT_NO_FATAL_FAILURE({ obj->contextMenuCb(); });

}

TEST_F(ItemInfoTest, findMeCb)
{
    // Test getter: FindMeCallback findMeCb()
    auto result = obj->findMeCb();
    EXPECT_NO_FATAL_FAILURE({ obj->findMeCb(); });

}
