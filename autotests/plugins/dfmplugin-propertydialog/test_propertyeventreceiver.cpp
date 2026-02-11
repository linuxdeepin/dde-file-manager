// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include "stubext.h"
#include <QUrl>
#include <QVariantHash>
#include <QList>
#include <QMetaEnum>

#include "events/propertyeventreceiver.h"
#include "utils/propertydialogutil.h"
#include "utils/propertydialogmanager.h"
#include "dfmplugin_propertydialog_global.h"
#include <dfm-framework/dpf.h>

using namespace dfmplugin_propertydialog;
using namespace DPF_NAMESPACE;

class TestPropertyEventReceiver : public testing::Test
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

TEST_F(TestPropertyEventReceiver, instance)
{
    PropertyEventReceiver *instance1 = PropertyEventReceiver::instance();
    PropertyEventReceiver *instance2 = PropertyEventReceiver::instance();
    
    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TestPropertyEventReceiver, bindEvents)
{
    PropertyEventReceiver receiver;
    
    // We can't easily stub DPF framework's connect function due to its template nature.
    // Instead, we just call the function to ensure it doesn't crash.
    // The actual event binding functionality would be tested in integration tests.
    EXPECT_NO_THROW(receiver.bindEvents());
}

TEST_F(TestPropertyEventReceiver, handleShowPropertyDialog)
{
    PropertyEventReceiver receiver;
    
    // Mock PropertyDialogUtil::instance() to return a mock instance
    PropertyDialogUtil *mockUtil = new PropertyDialogUtil();
    stub.set_lamda(&PropertyDialogUtil::instance,
                   [mockUtil]() -> PropertyDialogUtil* {
                       return mockUtil;
                   });
    
    // Track if showPropertyDialog is called on the mock
    bool showPropertyDialogCalled = false;
    stub.set_lamda(&PropertyDialogUtil::showPropertyDialog,
                   [&showPropertyDialogCalled](PropertyDialogUtil *self, const QList<QUrl> &urls, const QVariantHash &option) {
                       showPropertyDialogCalled = true;
                   });
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    receiver.handleShowPropertyDialog(urls, option);
    
    EXPECT_TRUE(showPropertyDialogCalled);
    
    delete mockUtil;
}

TEST_F(TestPropertyEventReceiver, handleShowPropertyDialogWithOptions)
{
    PropertyEventReceiver receiver;
    
    PropertyDialogUtil *mockUtil = new PropertyDialogUtil();
    stub.set_lamda(&PropertyDialogUtil::instance,
                   [mockUtil]() -> PropertyDialogUtil* {
                       return mockUtil;
                   });
    
    // Mock PropertyDialogManager::instance().getCreatorOptionByName
    QVariantHash mockOption;
    mockOption.insert(kOption_Key_Name, "test_name");
    stub.set_lamda(&PropertyDialogManager::getCreatorOptionByName,
                   [mockOption](PropertyDialogManager *self, const QString &name) -> QVariantHash {
                       return mockOption;
                   });
    
    bool showPropertyDialogCalled = false;
    stub.set_lamda(&PropertyDialogUtil::showPropertyDialog,
                   [&showPropertyDialogCalled](PropertyDialogUtil *self, const QList<QUrl> &urls, const QVariantHash &option) {
                       showPropertyDialogCalled = true;
                   });
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    option.insert(kOption_Key_Name, "test_name");
    option.insert("test_key", "override_value");
    
    receiver.handleShowPropertyDialog(urls, option);
    
    EXPECT_TRUE(showPropertyDialogCalled);
    
    delete mockUtil;
}

TEST_F(TestPropertyEventReceiver, handleViewExtensionRegister)
{
    PropertyEventReceiver receiver;
    
    bool registerExtensionViewCalled = false;
    stub.set_lamda(&PropertyDialogManager::registerExtensionView,
                   [&registerExtensionViewCalled](PropertyDialogManager *self, CustomViewExtensionView viewCreator, const QString &name, int index) {
                       registerExtensionViewCalled = true;
                       return true;
                   });
    
    auto mockView = [](const QUrl &url) -> QWidget* { return nullptr; };
    
    bool result = receiver.handleViewExtensionRegister(mockView, "test_name", 0);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(registerExtensionViewCalled);
}

TEST_F(TestPropertyEventReceiver, handleCustomViewRegister)
{
    PropertyEventReceiver receiver;
    
    bool registerCustomViewCalled = false;
    stub.set_lamda(&PropertyDialogManager::registerCustomView,
                   [&registerCustomViewCalled](PropertyDialogManager *self, CustomViewExtensionView view, const QString &scheme) {
                       registerCustomViewCalled = true;
                       return true;
                   });
    
    auto mockView = [](const QUrl &url) -> QWidget* { return nullptr; };
    
    bool result = receiver.handleCustomViewRegister(mockView, "test_scheme");
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(registerCustomViewCalled);
}

TEST_F(TestPropertyEventReceiver, handleBasicViewExtensionRegister)
{
    PropertyEventReceiver receiver;
    
    bool registerBasicViewExtensionCalled = false;
    stub.set_lamda(&PropertyDialogManager::registerBasicViewExtension,
                   [&registerBasicViewExtensionCalled](PropertyDialogManager *self, BasicViewFieldFunc func, const QString &scheme) {
                       registerBasicViewExtensionCalled = true;
                       return true;
                   });
    
    auto mockFunc = [](const QUrl &url) -> QMap<QString, QMultiMap<QString, QPair<QString, QString>>> {
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>();
    };
    
    bool result = receiver.handleBasicViewExtensionRegister(mockFunc, "test_scheme");
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(registerBasicViewExtensionCalled);
}

TEST_F(TestPropertyEventReceiver, handleBasicFiledFilterAdd_Success)
{
    PropertyEventReceiver receiver;
    
    bool addBasicFiledFiltesCalled = false;
    stub.set_lamda(&PropertyDialogManager::addBasicFiledFiltes,
                   [&addBasicFiledFiltesCalled](PropertyDialogManager *self, const QString &scheme, PropertyFilterType filters) {
                       addBasicFiledFiltesCalled = true;
                       return true;
                   });
    
    QStringList enums;
    enums << "kBasisInfo" << "kPermission";
    
    bool result = receiver.handleBasicFiledFilterAdd("test_scheme", enums);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(addBasicFiledFiltesCalled);
}

TEST_F(TestPropertyEventReceiver, handleBasicFiledFilterAdd_Failure)
{
    PropertyEventReceiver receiver;
    
    // Simulate when keysToValue fails
    bool addBasicFiledFiltesCalled = false;
    stub.set_lamda(&PropertyDialogManager::addBasicFiledFiltes,
                   [&addBasicFiledFiltesCalled](PropertyDialogManager *self, const QString &scheme, PropertyFilterType filters) {
                       addBasicFiledFiltesCalled = true;
                       return false; // Simulate failure
                   });
    
    QStringList enums;
    enums << "InvalidEnumValue"; // This will cause keysToValue to fail
    
    bool result = receiver.handleBasicFiledFilterAdd("test_scheme", enums);
    
    // Result should be false because enum parsing fails
    EXPECT_FALSE(result);
    EXPECT_FALSE(addBasicFiledFiltesCalled);
}