// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QMetaEnum>
#include <QUrl>
#include <QVariantHash>
#include <QMap>
#include <QMultiMap>
#include <QPair>

#include "stubext.h"
#include "utils/propertydialogmanager.h"
#include "utils/computerpropertyhelper.h"
#include "dfmplugin_propertydialog_global.h"

using namespace dfmplugin_propertydialog;

class TestPropertyDialogManager : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TestPropertyDialogManager, instance)
{
    PropertyDialogManager &manager1 = PropertyDialogManager::instance();
    PropertyDialogManager &manager2 = PropertyDialogManager::instance();

    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(TestPropertyDialogManager, registerExtensionView)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    auto mockView = [](const QUrl &url) -> QWidget* {
        return nullptr;
    };

    bool result = manager.registerExtensionView(mockView, "test_name_unique1", 0);
    EXPECT_TRUE(result);

    // Test with different index
    bool result2 = manager.registerExtensionView(mockView, "test_name_unique2", 1);
    EXPECT_TRUE(result2);
}

TEST_F(TestPropertyDialogManager, registerExtensionView_WithIndexIncrement)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    auto mockView = [](const QUrl &url) -> QWidget* {
        return nullptr;
    };

    // Register with index 0
    bool result = manager.registerExtensionView(mockView, "test_name1_unique", 0);
    EXPECT_TRUE(result);

    // Register with same index 0, should increment to next available index (1)
    bool result2 = manager.registerExtensionView(mockView, "test_name2_unique", 0);
    EXPECT_TRUE(result2);

    // Get the registered options to verify the indexes
    QVariantHash option1 = manager.getCreatorOptionByName("test_name1_unique");
    QVariantHash option2 = manager.getCreatorOptionByName("test_name2_unique");

    // The first should have index 0
    EXPECT_EQ(option1.value(kOption_Key_ViewIndex).toInt(), 0);
    // The second one should have index 1, as the manager increments on conflict
    int secondIndex = option2.value(kOption_Key_ViewIndex).toInt();
    EXPECT_EQ(secondIndex, 1);  // After conflict resolution, should be 1
}

TEST_F(TestPropertyDialogManager, createExtensionView_Empty)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.createExtensionView(url);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestPropertyDialogManager, createExtensionView_WithRegisteredView)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    // First register a view
    auto mockView = [](const QUrl &url) -> QWidget* {
        return new QWidget();
    };
    manager.registerExtensionView(mockView, "test_name_unique", 0);

    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.createExtensionView(url);

    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result.size(), 1);

    // Clean up created widgets
    for (QWidget *widget : result) {
        delete widget;
    }
}

TEST_F(TestPropertyDialogManager, createExtensionView_WithOptions)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    // First register a view to get the original creator callback
    auto mockView = [](const QUrl &url) -> QWidget* {
        QWidget *w = new QWidget();
        w->setObjectName("test_widget");
        return w;
    };
    manager.registerExtensionView(mockView, "test_name_unique", 0);

    // Get the original registered option that contains the creator callback
    QVariantHash originalOption = manager.getCreatorOptionByName("test_name_unique");
    ASSERT_FALSE(originalOption.isEmpty());
    
    // Create new option with additional parameters but preserving the original creator callback
    QVariantHash option = originalOption; // Copy all original data including creator callback
    option.insert(kOption_Key_BasicInfoExpand, false); // Override or add new options

    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.createExtensionView(url, option);

    EXPECT_FALSE(result.isEmpty());

    // Clean up created widgets
    for (QWidget *widget : result) {
        delete widget;
    }
}

TEST_F(TestPropertyDialogManager, registerCustomView)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    auto mockView = [](const QUrl &url) -> QWidget* {
        return new QWidget();
    };

    bool result = manager.registerCustomView(mockView, "test_scheme_unique1");
    EXPECT_TRUE(result);

    // Register same scheme again, should fail
    bool result2 = manager.registerCustomView(mockView, "test_scheme_unique1");
    EXPECT_FALSE(result2);

    // Register different scheme, should succeed
    bool result3 = manager.registerCustomView(mockView, "test_scheme_unique2");
    EXPECT_TRUE(result3);

    // Clean up
    QWidget *testWidget = mockView(QUrl::fromLocalFile("/tmp"));
    if (testWidget) {
        delete testWidget;
    }
}

TEST_F(TestPropertyDialogManager, createCustomView)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");

    // Test with no registered views initially
    QWidget *result = manager.createCustomView(testUrl);
    
    // The behavior of createCustomView is to iterate over ALL registered functions
    // If other tests have registered functions, this may return a widget even for "no registered views"

    // Register a view - it will be used for all URLs as the implementation
    // iterates over all registered functions and returns the first non-null result
    auto mockView = [](const QUrl &url) -> QWidget* {
        // For testing, create a widget for any URL
        return new QWidget();
    };
    bool registerResult = manager.registerCustomView(mockView, "file_unique");
    EXPECT_TRUE(registerResult);

    // Now calling createCustomView with any URL will return a widget
    // because the registered function returns a widget for any URL
    QWidget *result2 = manager.createCustomView(QUrl("ftp://example.com/test"));
    EXPECT_NE(result2, nullptr);

    QWidget *result3 = manager.createCustomView(testUrl);  // file:///tmp/test 
    EXPECT_NE(result3, nullptr);

    if (result2) {
        delete result2;
    }
    if (result3) {
        delete result3;
    }
}

TEST_F(TestPropertyDialogManager, registerBasicViewExtension)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    auto mockFunc = [](const QUrl &url) -> QMap<QString, QMultiMap<QString, QPair<QString, QString>>> {
        QMap<QString, QMultiMap<QString, QPair<QString, QString>>> result;
        QMultiMap<QString, QPair<QString, QString>> subMap;
        subMap.insert("kFileSize", qMakePair(QString("Size"), QString("100KB")));
        result.insert("kFieldInsert", subMap);
        return result;
    };

    bool result = manager.registerBasicViewExtension(mockFunc, "test_scheme_unique1");
    EXPECT_TRUE(result);

    // Register same scheme again, should fail
    bool result2 = manager.registerBasicViewExtension(mockFunc, "test_scheme_unique1");
    EXPECT_FALSE(result2);
}

TEST_F(TestPropertyDialogManager, createBasicViewExtensionField)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");

    // Test with no registered functions
    auto result = manager.createBasicViewExtensionField(testUrl);
    EXPECT_TRUE(result.isEmpty());

    // Register a function
    auto mockFunc = [](const QUrl &url) -> QMap<QString, QMultiMap<QString, QPair<QString, QString>>> {
        QMap<QString, QMultiMap<QString, QPair<QString, QString>>> result;
        QMultiMap<QString, QPair<QString, QString>> subMap;
        subMap.insert("kFileSize", qMakePair(QString("Size"), QString("100KB")));
        result.insert("kFieldInsert", subMap);
        return result;
    };
    manager.registerBasicViewExtension(mockFunc, "file");

    // Test with registered function
    auto result2 = manager.createBasicViewExtensionField(testUrl);
    EXPECT_FALSE(result2.isEmpty());
}

TEST_F(TestPropertyDialogManager, addBasicFiledFiltes)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    bool result = manager.addBasicFiledFiltes("test_scheme_unique1", kPermission);
    EXPECT_TRUE(result);

    // Add same scheme again, should fail
    bool result2 = manager.addBasicFiledFiltes("test_scheme_unique1", kFileSizeFiled);
    EXPECT_FALSE(result2);
}

TEST_F(TestPropertyDialogManager, basicFiledFiltes)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");

    // Test with empty filter hash
    PropertyFilterType result = manager.basicFiledFiltes(testUrl);
    EXPECT_EQ(result, kNotFilter);

    // Add a filter
    manager.addBasicFiledFiltes("file", kPermission);

    // Test with matching scheme
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/test");
    PropertyFilterType result2 = manager.basicFiledFiltes(fileUrl);
    EXPECT_EQ(result2, kPermission);

    // Test with non-matching scheme
    QUrl ftpUrl("ftp://example.com/test");
    PropertyFilterType result3 = manager.basicFiledFiltes(ftpUrl);
    EXPECT_EQ(result3, kNotFilter);
}

TEST_F(TestPropertyDialogManager, getCreatorOptionByName)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    // Test with non-existent name
    QVariantHash result = manager.getCreatorOptionByName("nonexistent_unique1");
    EXPECT_TRUE(result.isEmpty());

    // Register a view
    auto mockView = [](const QUrl &url) -> QWidget* {
        return nullptr;
    };
    manager.registerExtensionView(mockView, "test_name_unique1", 0);

    // Test with existing name
    QVariantHash result2 = manager.getCreatorOptionByName("test_name_unique1");
    EXPECT_FALSE(result2.isEmpty());
    EXPECT_EQ(result2.value(kOption_Key_Name).toString(), "test_name_unique1");
    EXPECT_EQ(result2.value(kOption_Key_ViewIndex).toInt(), 0);
}

TEST_F(TestPropertyDialogManager, addComputerPropertyDialog)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();

    // Mock ComputerPropertyHelper::scheme() and createComputerProperty
    stub.set_lamda(&ComputerPropertyHelper::scheme,
                   []() -> QString {
                       return "computer_unique";
                   });

    bool createComputerPropertyCalled = false;
    stub.set_lamda(&ComputerPropertyHelper::createComputerProperty,
                   [&createComputerPropertyCalled](const QUrl &url) -> QWidget* {
                       createComputerPropertyCalled = true;
                       return new QWidget();
                   });

    manager.addComputerPropertyDialog();

    // Verify that ComputerPropertyHelper::createComputerProperty would be called
    // when creating a custom view for computer scheme
    QUrl computerUrl("computer_unique:///");
    QWidget *result = manager.createCustomView(computerUrl);

    EXPECT_TRUE(createComputerPropertyCalled);
    if (result) {
        delete result;
    }
}