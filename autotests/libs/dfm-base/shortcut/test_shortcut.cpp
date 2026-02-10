// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <dfm-base/shortcut/shortcut.h>

using namespace dfmbase;

class TestShortcut : public testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
    }

    void TearDown() override {
        // Cleanup code after each test
    }
};

// Test constructor
TEST_F(TestShortcut, TestConstructor) {
    QObject parent;
    Shortcut shortcut(&parent);
    
    EXPECT_EQ(shortcut.parent(), &parent);
}

// Test constructor with nullptr parent
TEST_F(TestShortcut, TestConstructorWithNullParent) {
    Shortcut shortcut(nullptr);
    
    EXPECT_EQ(shortcut.parent(), nullptr);
}

// Test toStr method
TEST_F(TestShortcut, TestToStr) {
    Shortcut shortcut;
    
    QString result = shortcut.toStr();
    
    // Result should not be empty
    EXPECT_FALSE(result.isEmpty());
    
    // Should be valid JSON
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    EXPECT_FALSE(doc.isNull());
    EXPECT_TRUE(doc.isObject());
}

// Test toStr JSON structure
TEST_F(TestShortcut, TestToStrStructure) {
    Shortcut shortcut;
    
    QString result = shortcut.toStr();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonObject obj = doc.object();
    
    // Should have shortcut key
    EXPECT_TRUE(obj.contains("shortcut"));
    
    QJsonArray groups = obj["shortcut"].toArray();
    
    // Should have 5 groups
    EXPECT_EQ(groups.size(), 5);
    
    // Check each group structure
    QStringList expectedGroups = {
        "Item",
        "New/Search",
        "View",
        "Switch display status",
        "Others"
    };
    
    for (int i = 0; i < groups.size(); ++i) {
        QJsonObject group = groups[i].toObject();
        EXPECT_TRUE(group.contains("groupName"));
        EXPECT_TRUE(group.contains("groupItems"));
        
        QString groupName = group["groupName"].toString();
        EXPECT_TRUE(expectedGroups.contains(groupName));
        
        QJsonArray items = group["groupItems"].toArray();
        EXPECT_FALSE(items.isEmpty());
        
        // Check each item structure
        for (int j = 0; j < items.size(); ++j) {
            QJsonObject item = items[j].toObject();
            EXPECT_TRUE(item.contains("name"));
            EXPECT_TRUE(item.contains("value"));
            
            QString name = item["name"].toString();
            QString value = item["value"].toString();
            EXPECT_FALSE(name.isEmpty());
            EXPECT_FALSE(value.isEmpty());
        }
    }
}

// Test Item group shortcuts
TEST_F(TestShortcut, TestItemGroupShortcuts) {
    Shortcut shortcut;
    
    QString result = shortcut.toStr();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray groups = doc.object()["shortcut"].toArray();
    
    // Find Item group (first group)
    QJsonObject itemGroup = groups[0].toObject();
    EXPECT_EQ(itemGroup["groupName"].toString(), "Item");
    
    QJsonArray items = itemGroup["groupItems"].toArray();
    EXPECT_GT(items.size(), 15);  // Should have many items
    
    // Check for specific shortcuts
    QStringList expectedShortcuts = {
        "Select to the first item",
        "Select to the last item",
        "Select leftwards",
        "Select rightwards",
        "Select to upper row",
        "Select to lower row",
        "Reverse selection",
        "Open",
        "To parent directory",
        "Permanently delete",
        "Delete file",
        "Select all",
        "Copy",
        "Cut",
        "Paste",
        "Rename",
        "Copy file path",
        "Open in terminal",
        "Undo",
        "Redo"
    };
    
    for (const QString &expectedName : expectedShortcuts) {
        bool found = false;
        for (int i = 0; i < items.size(); ++i) {
            QJsonObject item = items[i].toObject();
            if (item["name"].toString() == expectedName) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Shortcut not found: " << expectedName.toStdString();
    }
}

// Test New/Search group shortcuts
TEST_F(TestShortcut, TestNewSearchGroupShortcuts) {
    Shortcut shortcut;
    
    QString result = shortcut.toStr();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray groups = doc.object()["shortcut"].toArray();
    
    // Find New/Search group (second group)
    QJsonObject searchGroup = groups[1].toObject();
    EXPECT_EQ(searchGroup["groupName"].toString(), "New/Search");
    
    QJsonArray items = searchGroup["groupItems"].toArray();
    EXPECT_EQ(items.size(), 4);  // Should have 4 items
    
    QStringList expectedItems = {
        "New window",
        "New folder",
        "Search",
        "New tab"
    };
    
    for (const QString &expectedName : expectedItems) {
        bool found = false;
        for (int i = 0; i < items.size(); ++i) {
            QJsonObject item = items[i].toObject();
            if (item["name"].toString() == expectedName) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Shortcut not found: " << expectedName.toStdString();
    }
}

// Test View group shortcuts
TEST_F(TestShortcut, TestViewGroupShortcuts) {
    Shortcut shortcut;
    
    QString result = shortcut.toStr();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray groups = doc.object()["shortcut"].toArray();
    
    // Find View group (third group)
    QJsonObject viewGroup = groups[2].toObject();
    EXPECT_EQ(viewGroup["groupName"].toString(), "View");
    
    QJsonArray items = viewGroup["groupItems"].toArray();
    EXPECT_EQ(items.size(), 3);  // Should have 3 items
    
    QStringList expectedItems = {
        "Item information",
        "Help",
        "Keyboard shortcuts"
    };
    
    for (const QString &expectedName : expectedItems) {
        bool found = false;
        for (int i = 0; i < items.size(); ++i) {
            QJsonObject item = items[i].toObject();
            if (item["name"].toString() == expectedName) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Shortcut not found: " << expectedName.toStdString();
    }
}

// Test Switch display status group shortcuts
TEST_F(TestShortcut, TestSwitchDisplayGroupShortcuts) {
    Shortcut shortcut;
    
    QString result = shortcut.toStr();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray groups = doc.object()["shortcut"].toArray();
    
    // Find Switch display status group (fourth group)
    QJsonObject switchGroup = groups[3].toObject();
    EXPECT_EQ(switchGroup["groupName"].toString(), "Switch display status");
    
    QJsonArray items = switchGroup["groupItems"].toArray();
    EXPECT_EQ(items.size(), 5);  // Should have 5 items
    
    QStringList expectedItems = {
        "Hide item",
        "Input in address bar",
        "Switch to icon view",
        "Switch to list view",
        "Switch to tree view"
    };
    
    for (const QString &expectedName : expectedItems) {
        bool found = false;
        for (int i = 0; i < items.size(); ++i) {
            QJsonObject item = items[i].toObject();
            if (item["name"].toString() == expectedName) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Shortcut not found: " << expectedName.toStdString();
    }
}

// Test Others group shortcuts
TEST_F(TestShortcut, TestOthersGroupShortcuts) {
    Shortcut shortcut;
    
    QString result = shortcut.toStr();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray groups = doc.object()["shortcut"].toArray();
    
    // Find Others group (fifth group)
    QJsonObject othersGroup = groups[4].toObject();
    EXPECT_EQ(othersGroup["groupName"].toString(), "Others");
    
    QJsonArray items = othersGroup["groupItems"].toArray();
    EXPECT_EQ(items.size(), 8);  // Should have 8 items
    
    QStringList expectedItems = {
        "Close",
        "Close current tab",
        "Back",
        "Forward",
        "Switch to next tab",
        "Switch to previous tab",
        "Next file",
        "Previous file",
        "Switch tab by specified number between 1 to 8"
    };
    
    // Note: The last item combines the description, so we check separately
    for (int i = 0; i < 7; ++i) {
        const QString &expectedName = expectedItems[i];
        bool found = false;
        for (int j = 0; j < items.size(); ++j) {
            QJsonObject item = items[j].toObject();
            if (item["name"].toString() == expectedName) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Shortcut not found: " << expectedName.toStdString();
    }
    
    // Check the last special item
    bool foundLast = false;
    for (int i = 0; i < items.size(); ++i) {
        QJsonObject item = items[i].toObject();
        QString name = item["name"].toString();
        if (name.contains("Switch tab by specified number")) {
            foundLast = true;
            break;
        }
    }
    EXPECT_TRUE(foundLast);
}

// Test multiple calls to toStr
TEST_F(TestShortcut, TestMultipleToStrCalls) {
    Shortcut shortcut;
    
    QString result1 = shortcut.toStr();
    QString result2 = shortcut.toStr();
    
    // Results should be identical
    EXPECT_EQ(result1, result2);
    
    // And should be valid JSON
    QJsonDocument doc1 = QJsonDocument::fromJson(result1.toUtf8());
    QJsonDocument doc2 = QJsonDocument::fromJson(result2.toUtf8());
    
    EXPECT_FALSE(doc1.isNull());
    EXPECT_FALSE(doc2.isNull());
    EXPECT_EQ(doc1, doc2);
}

// Test toStr returns valid JSON format
TEST_F(TestShortcut, TestToStrValidJson) {
    Shortcut shortcut;
    
    QString result = shortcut.toStr();
    
    // Parse as JSON to validate format
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8(), &parseError);
    
    EXPECT_EQ(parseError.error, QJsonParseError::NoError);
    EXPECT_FALSE(doc.isNull());
    EXPECT_TRUE(doc.isObject());
    
    // Verify it's the expected structure
    QJsonObject obj = doc.object();
    EXPECT_TRUE(obj.contains("shortcut"));
    EXPECT_TRUE(obj["shortcut"].isArray());
}

// Test ShortcutItem struct
TEST_F(TestShortcut, TestShortcutItemStruct) {
    QString name = "Test Item";
    QString value = "Ctrl + T";
    
    ShortcutItem item(name, value);
    
    EXPECT_EQ(item.name, name);
    EXPECT_EQ(item.value, value);
}

// Test ShortcutGroup struct
TEST_F(TestShortcut, TestShortcutGroupStruct) {
    QString groupName = "Test Group";
    QList<ShortcutItem> items;
    items << ShortcutItem("Item 1", "Ctrl + 1");
    items << ShortcutItem("Item 2", "Ctrl + 2");
    
    ShortcutGroup group;
    group.groupName = groupName;
    group.groupItems = items;
    
    EXPECT_EQ(group.groupName, groupName);
    EXPECT_EQ(group.groupItems.size(), 2);
    EXPECT_EQ(group.groupItems[0].name, "Item 1");
    EXPECT_EQ(group.groupItems[1].name, "Item 2");
}