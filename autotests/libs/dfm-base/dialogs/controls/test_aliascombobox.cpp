// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QPaintEvent>
#include <QPainter>
#include <QStringList>

#include <dfm-base/dialogs/settingsdialog/controls/aliascombobox.h>

// Include DTK widgets
#include <DComboBox>
#include <DWidget>
#include <DApplication>

// Include stub headers
#include "stubext.h"

DWIDGET_USE_NAMESPACE

class TestAliasComboBox : public testing::Test
{
protected:
    void SetUp() override {
        stub.clear();
        // Create application for GUI tests
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            new DApplication(argc, argv);
        }
    }
    
    void TearDown() override {
        stub.clear();
    }
    
public:
    stub_ext::StubExt stub;
};

// Test constructor without parent
TEST_F(TestAliasComboBox, TestConstructorNoParent)
{
    AliasComboBox comboBox;
    
    // Should not crash
    EXPECT_NE(&comboBox, nullptr);
    EXPECT_EQ(comboBox.parent(), nullptr);
    EXPECT_EQ(comboBox.count(), 0);
}

// Test constructor with parent
TEST_F(TestAliasComboBox, TestConstructorWithParent)
{
    QWidget parent;
    AliasComboBox comboBox(&parent);
    
    // Should not crash
    EXPECT_NE(&comboBox, nullptr);
    EXPECT_EQ(comboBox.parent(), &parent);
}

// Test setItemAlias and itemAlias
TEST_F(TestAliasComboBox, TestItemAlias)
{
    AliasComboBox comboBox;
    
    // Add some items
    comboBox.addItem("Item 1");
    comboBox.addItem("Item 2");
    comboBox.addItem("Item 3");
    
    // Set aliases for items
    comboBox.setItemAlias(0, "Alias 1");
    comboBox.setItemAlias(1, "Alias 2");
    comboBox.setItemAlias(2, "Alias 3");
    
    // Get aliases
    EXPECT_EQ(comboBox.itemAlias(0), "Alias 1");
    EXPECT_EQ(comboBox.itemAlias(1), "Alias 2");
    EXPECT_EQ(comboBox.itemAlias(2), "Alias 3");
}

// Test itemAlias with invalid index
TEST_F(TestAliasComboBox, TestItemAliasInvalidIndex)
{
    AliasComboBox comboBox;
    
    // Add some items
    comboBox.addItem("Item 1");
    comboBox.addItem("Item 2");
    
    // Test with negative index
    QString alias1 = comboBox.itemAlias(-1);
    EXPECT_TRUE(alias1.isEmpty());
    
    // Test with index out of range
    QString alias2 = comboBox.itemAlias(10);
    EXPECT_TRUE(alias2.isEmpty());
    
    // Test with index equal to count
    QString alias3 = comboBox.itemAlias(comboBox.count());
    EXPECT_TRUE(alias3.isEmpty());
}

// Test setItemAlias with invalid index
TEST_F(TestAliasComboBox, TestSetItemAliasInvalidIndex)
{
    AliasComboBox comboBox;
    
    // Add some items
    comboBox.addItem("Item 1");
    comboBox.addItem("Item 2");
    
    // Set alias for negative index - should not crash
    comboBox.setItemAlias(-1, "Invalid Alias");
    
    // Set alias for index out of range - should not crash
    comboBox.setItemAlias(10, "Invalid Alias");
    
    // Should not crash
    SUCCEED();
}

// Test setItemAlias with empty alias
TEST_F(TestAliasComboBox, TestSetItemAliasEmpty)
{
    AliasComboBox comboBox;
    
    // Add an item
    comboBox.addItem("Test Item");
    
    // Set empty alias
    comboBox.setItemAlias(0, "");
    
    // Get alias
    QString alias = comboBox.itemAlias(0);
    EXPECT_TRUE(alias.isEmpty());
}

// Test setItemAlias with special characters
TEST_F(TestAliasComboBox, TestSetItemAliasSpecialChars)
{
    AliasComboBox comboBox;
    
    // Add an item
    comboBox.addItem("Test Item");
    
    // Set alias with special characters
    QString specialAlias = "Special: !@#$%^&*()_+-=[]{}|;':\",./<>?";
    comboBox.setItemAlias(0, specialAlias);
    
    // Get alias
    QString alias = comboBox.itemAlias(0);
    EXPECT_EQ(alias, specialAlias);
}

// Test setItemAlias with Unicode
TEST_F(TestAliasComboBox, TestSetItemAliasUnicode)
{
    AliasComboBox comboBox;
    
    // Add an item
    comboBox.addItem("Test Item");
    
    // Set alias with Unicode characters
    QString unicodeAlias = "Unicode test: 中文测试 العربية русский 日本語 한국어";
    comboBox.setItemAlias(0, unicodeAlias);
    
    // Get alias
    QString alias = comboBox.itemAlias(0);
    EXPECT_EQ(alias, unicodeAlias);
}

// Test paintEvent
TEST_F(TestAliasComboBox, TestPaintEvent)
{
    AliasComboBox comboBox;
    
    // Add some items
    comboBox.addItem("Item 1");
    comboBox.addItem("Item 2");
    
    // Create a paint event
    QPaintEvent paintEvent(comboBox.rect());
    
    // Mock DComboBox::paintEvent to avoid actual painting
    // bool paintCalled = false;
    // stub.set_lamda((void(DComboBox::*)(QPaintEvent*))&DComboBox::paintEvent, 
    //                [&paintCalled](DComboBox*, QPaintEvent*) {
    //     paintCalled = true;
    //     __DBG_STUB_INVOKE__
    // });
    
    // Call paintEvent
    comboBox.paintEvent(&paintEvent);
    
    // Paint should be called
    // EXPECT_TRUE(paintCalled);
}

// Test multiple items with aliases
TEST_F(TestAliasComboBox, TestMultipleItemsWithAliases)
{
    AliasComboBox comboBox;
    
    // Add multiple items
    QStringList items = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
    QStringList aliases = {"Alias A", "Alias B", "Alias C", "Alias D", "Alias E"};
    
    for (int i = 0; i < items.count(); ++i) {
        comboBox.addItem(items[i]);
        comboBox.setItemAlias(i, aliases[i]);
    }
    
    // Verify all aliases
    for (int i = 0; i < items.count(); ++i) {
        EXPECT_EQ(comboBox.itemText(i), items[i]);
        EXPECT_EQ(comboBox.itemAlias(i), aliases[i]);
    }
}

// Test updating alias
TEST_F(TestAliasComboBox, TestUpdateAlias)
{
    AliasComboBox comboBox;
    
    // Add an item
    comboBox.addItem("Test Item");
    
    // Set initial alias
    comboBox.setItemAlias(0, "Initial Alias");
    EXPECT_EQ(comboBox.itemAlias(0), "Initial Alias");
    
    // Update alias
    comboBox.setItemAlias(0, "Updated Alias");
    EXPECT_EQ(comboBox.itemAlias(0), "Updated Alias");
    
    // Clear alias by setting empty string
    comboBox.setItemAlias(0, "");
    EXPECT_TRUE(comboBox.itemAlias(0).isEmpty());
}

// Test alias persistence after adding more items
TEST_F(TestAliasComboBox, TestAliasPersistence)
{
    AliasComboBox comboBox;
    
    // Add initial items with aliases
    comboBox.addItem("Item 1");
    comboBox.setItemAlias(0, "Alias 1");
    
    // Add more items
    comboBox.addItem("Item 2");
    comboBox.setItemAlias(1, "Alias 2");
    comboBox.addItem("Item 3");
    comboBox.setItemAlias(2, "Alias 3");
    
    // Verify all aliases are still correct
    EXPECT_EQ(comboBox.itemAlias(0), "Alias 1");
    EXPECT_EQ(comboBox.itemAlias(1), "Alias 2");
    EXPECT_EQ(comboBox.itemAlias(2), "Alias 3");
}

// Test with empty combobox
TEST_F(TestAliasComboBox, TestEmptyComboBox)
{
    AliasComboBox comboBox;
    
    // Empty combobox should have 0 items
    EXPECT_EQ(comboBox.count(), 0);
    
    // Getting alias from empty combobox should return empty string
    QString alias = comboBox.itemAlias(0);
    EXPECT_TRUE(alias.isEmpty());
}

// Test very long alias
TEST_F(TestAliasComboBox, TestLongAlias)
{
    AliasComboBox comboBox;
    
    // Add an item
    comboBox.addItem("Test Item");
    
    // Create a very long alias
    QString longAlias;
    for (int i = 0; i < 100; ++i) {
        longAlias += "This is a very long alias string. ";
    }
    
    // Set the long alias
    comboBox.setItemAlias(0, longAlias);
    
    // Get the alias
    QString retrievedAlias = comboBox.itemAlias(0);
    EXPECT_EQ(retrievedAlias, longAlias);
}

// Test combobox visibility
TEST_F(TestAliasComboBox, TestVisibility)
{
    AliasComboBox comboBox;
    
    // Initially visible
    EXPECT_TRUE(comboBox.isVisible());
    
    // Hide
    comboBox.hide();
    EXPECT_FALSE(comboBox.isVisible());
    
    // Show
    comboBox.show();
    EXPECT_TRUE(comboBox.isVisible());
}

// Test combobox enabled state
TEST_F(TestAliasComboBox, TestEnabledState)
{
    AliasComboBox comboBox;
    
    // Initially enabled
    EXPECT_TRUE(comboBox.isEnabled());
    
    // Disable
    comboBox.setEnabled(false);
    EXPECT_FALSE(comboBox.isEnabled());
    
    // Enable
    comboBox.setEnabled(true);
    EXPECT_TRUE(comboBox.isEnabled());
}

// Test combobox with items and current index
TEST_F(TestAliasComboBox, TestCurrentIndex)
{
    AliasComboBox comboBox;
    
    // Add items with aliases
    comboBox.addItem("Item 1");
    comboBox.setItemAlias(0, "Alias 1");
    comboBox.addItem("Item 2");
    comboBox.setItemAlias(1, "Alias 2");
    comboBox.addItem("Item 3");
    comboBox.setItemAlias(2, "Alias 3");
    
    // Initially current index should be 0
    EXPECT_EQ(comboBox.currentIndex(), 0);
    
    // Set current index
    comboBox.setCurrentIndex(1);
    EXPECT_EQ(comboBox.currentIndex(), 1);
    
    // Set to last index
    comboBox.setCurrentIndex(2);
    EXPECT_EQ(comboBox.currentIndex(), 2);
}

// Test clearing items
TEST_F(TestAliasComboBox, TestClearItems)
{
    AliasComboBox comboBox;
    
    // Add items with aliases
    comboBox.addItem("Item 1");
    comboBox.setItemAlias(0, "Alias 1");
    comboBox.addItem("Item 2");
    comboBox.setItemAlias(1, "Alias 2");
    
    // Verify items and aliases
    EXPECT_EQ(comboBox.count(), 2);
    EXPECT_EQ(comboBox.itemAlias(0), "Alias 1");
    EXPECT_EQ(comboBox.itemAlias(1), "Alias 2");
    
    // Clear items
    comboBox.clear();
    
    // Verify all cleared
    EXPECT_EQ(comboBox.count(), 0);
    EXPECT_TRUE(comboBox.itemAlias(0).isEmpty());
}

// Test destruction
TEST_F(TestAliasComboBox, TestDestructor)
{
    // Create and destroy combobox
    {
        AliasComboBox comboBox;
        comboBox.addItem("Test");
        comboBox.setItemAlias(0, "Test Alias");
        // Combobox will be destroyed when leaving scope
    }
    
    // Should not crash
    SUCCEED();
}