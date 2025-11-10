// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include "stubext.h"
#include <QFileDevice>

#include "dfmplugin_propertydialog_global.h"
#include "propertydialog.h"
#include "events/propertyeventreceiver.h"
#include "events/propertyeventcall.h"
#include "utils/propertydialogmanager.h"
#include "utils/propertydialogutil.h"
#include "utils/computerpropertyhelper.h"
#include "utils/mediainfofetchworker.h"
#include "menu/propertymenuscene.h"
#include "views/filepropertydialog.h"
#include "views/computerpropertydialog.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

using namespace dfmplugin_propertydialog;
using namespace dfmplugin_menu_util;

class TestPropertyDialog : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // Ensure plugin is stopped at beginning of each test
        propertyDialog.stop();
    }

    void TearDown() override
    {
        // Ensure plugin is stopped at end of each test
        propertyDialog.stop();
        stub.clear();
    }

    stub_ext::StubExt stub;
    static PropertyDialog propertyDialog;   // Use static instance like other plugins
};

PropertyDialog TestPropertyDialog::propertyDialog;

// Test PropertyDialog class - minimal stubbing to avoid memory alignment issues
TEST_F(TestPropertyDialog, InitializeTest)
{
    EXPECT_NO_THROW(propertyDialog.initialize());
    EXPECT_TRUE(true);
}

TEST_F(TestPropertyDialog, StartTest)
{
    bool result = propertyDialog.start();
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, StopTest)
{
    // First start the plugin then stop it
    propertyDialog.start();
    EXPECT_NO_THROW(propertyDialog.stop());
    // Stopping again should be safe
    EXPECT_NO_THROW(propertyDialog.stop());
}

// Test PropertyEventReceiver class
TEST_F(TestPropertyDialog, InstanceTest)
{
    PropertyEventReceiver *instance1 = PropertyEventReceiver::instance();
    PropertyEventReceiver *instance2 = PropertyEventReceiver::instance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TestPropertyDialog, BindEventsTest)
{
    PropertyEventReceiver receiver;
    EXPECT_NO_THROW(receiver.bindEvents());
    EXPECT_TRUE(true);
}

TEST_F(TestPropertyDialog, HandleShowPropertyDialogTest)
{
    PropertyEventReceiver receiver;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    EXPECT_NO_THROW(receiver.handleShowPropertyDialog(urls, option));
}

TEST_F(TestPropertyDialog, HandleViewExtensionRegisterTest)
{
    PropertyEventReceiver receiver;
    auto viewFunc = [](const QUrl &url) -> QWidget* { return nullptr; };
    bool result = receiver.handleViewExtensionRegister(viewFunc, "test_name", 0);
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, HandleCustomViewRegisterTest)
{
    PropertyEventReceiver receiver;
    auto viewFunc = [](const QUrl &url) -> QWidget* { return nullptr; };
    bool result = receiver.handleCustomViewRegister(viewFunc, "test_scheme");
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, HandleBasicViewExtensionRegisterTest)
{
    PropertyEventReceiver receiver;
    auto func = [](const QUrl &url) -> QMap<QString, QMultiMap<QString, QPair<QString, QString>>> { 
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(); 
    };
    bool result = receiver.handleBasicViewExtensionRegister(func, "test_scheme");
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, HandleBasicFiledFilterAddTest)
{
    PropertyEventReceiver receiver;
    QStringList enums;
    enums << "kBasisInfo";
    bool result1 = receiver.handleBasicFiledFilterAdd("test_scheme_unique4", enums);
    EXPECT_TRUE(result1);
    
    enums.clear();
    enums << "kBasisInfo" << "kPermission";
    bool result2 = receiver.handleBasicFiledFilterAdd("test_scheme_unique5", enums);
    EXPECT_TRUE(result2);
}

// Test PropertyDialogManager class
TEST_F(TestPropertyDialog, ManagerInstanceTest)
{
    PropertyDialogManager &manager1 = PropertyDialogManager::instance();
    PropertyDialogManager &manager2 = PropertyDialogManager::instance();
    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(TestPropertyDialog, RegisterExtensionViewTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    auto viewFunc = [](const QUrl &url) -> QWidget* { return nullptr; };
    bool result = manager.registerExtensionView(viewFunc, "test_name", 0);
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, CreateExtensionViewTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.createExtensionView(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestPropertyDialog, RegisterCustomViewTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    auto viewFunc = [](const QUrl &url) -> QWidget* { return nullptr; };
    bool result = manager.registerCustomView(viewFunc, "test_scheme_unique1");
    EXPECT_TRUE(result);

    bool result2 = manager.registerCustomView(viewFunc, "test_scheme_unique1");
    EXPECT_FALSE(result2);
}

TEST_F(TestPropertyDialog, CreateCustomViewTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QWidget *result = manager.createCustomView(url);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestPropertyDialog, RegisterBasicViewExtensionTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    auto func = [](const QUrl &url) -> QMap<QString, QMultiMap<QString, QPair<QString, QString>>> { 
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(); 
    };
    bool result = manager.registerBasicViewExtension(func, "test_scheme_unique2");
    EXPECT_TRUE(result);

    bool result2 = manager.registerBasicViewExtension(func, "test_scheme_unique2");
    EXPECT_FALSE(result2);
}

TEST_F(TestPropertyDialog, CreateBasicViewExtensionFieldTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.createBasicViewExtensionField(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestPropertyDialog, AddBasicFiledFiltesTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    bool result = manager.addBasicFiledFiltes("test_scheme_unique3", kPermission);
    EXPECT_TRUE(result);

    bool result2 = manager.addBasicFiledFiltes("test_scheme_unique3", kFileSizeFiled);
    EXPECT_FALSE(result2);
}

TEST_F(TestPropertyDialog, BasicFiledFiltesTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.basicFiledFiltes(url);
    EXPECT_EQ(result, kNotFilter);
}

TEST_F(TestPropertyDialog, GetCreatorOptionByNameTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    auto result = manager.getCreatorOptionByName("nonexistent");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestPropertyDialog, AddComputerPropertyDialogTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    EXPECT_NO_THROW(manager.addComputerPropertyDialog());
}

// Test PropertyDialogUtil class
TEST_F(TestPropertyDialog, UtilInstanceTest)
{
    PropertyDialogUtil *util1 = PropertyDialogUtil::instance();
    PropertyDialogUtil *util2 = PropertyDialogUtil::instance();
    EXPECT_EQ(util1, util2);
}

TEST_F(TestPropertyDialog, ShowPropertyDialogTest)
{
    PropertyDialogUtil util;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;

    EXPECT_NO_THROW(util.showPropertyDialog(urls, option));
}

TEST_F(TestPropertyDialog, ShowFilePropertyDialogTest)
{
    PropertyDialogUtil util;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;

    EXPECT_NO_THROW(util.showFilePropertyDialog(urls, option));
}

TEST_F(TestPropertyDialog, ShowCustomDialogTest)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");

    EXPECT_NO_THROW(util.showCustomDialog(url));
}

TEST_F(TestPropertyDialog, CloseFilePropertyDialogTest)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");

    EXPECT_NO_THROW(util.closeFilePropertyDialog(url));
}

TEST_F(TestPropertyDialog, CloseCustomPropertyDialogTest)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    
    EXPECT_NO_THROW(util.closeCustomPropertyDialog(url));
}

TEST_F(TestPropertyDialog, CloseAllFilePropertyDialogTest)
{
    PropertyDialogUtil util;
    
    EXPECT_NO_THROW(util.closeAllFilePropertyDialog());
}

TEST_F(TestPropertyDialog, CloseAllPropertyDialogTest)
{
    PropertyDialogUtil util;
    
    EXPECT_NO_THROW(util.closeAllPropertyDialog());
}

TEST_F(TestPropertyDialog, CreateControlViewTest)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    EXPECT_NO_THROW(util.createControlView(url, option));
}

TEST_F(TestPropertyDialog, UpdateCloseIndicatorTest)
{
    PropertyDialogUtil util;
    
    EXPECT_NO_THROW(util.updateCloseIndicator());
}

// Test ComputerPropertyHelper class
TEST_F(TestPropertyDialog, ComputerPropertyHelperSchemeTest)
{
    QString scheme = ComputerPropertyHelper::scheme();
    EXPECT_FALSE(scheme.isEmpty());
}

TEST_F(TestPropertyDialog, ComputerPropertyHelperCreateComputerPropertyTest)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QWidget *widget = ComputerPropertyHelper::createComputerProperty(url);
    EXPECT_TRUE(widget == nullptr);
}

// Test MediaInfoFetchWorker class
TEST_F(TestPropertyDialog, MediaInfoFetchWorkerTest)
{
    MediaInfoFetchWorker worker;
    EXPECT_NO_THROW(worker.getDuration("/tmp/test.mp4"));
}

// Test PropertyEventCall class
TEST_F(TestPropertyDialog, PropertyEventCallSendSetPermissionManagerTest)
{
    EXPECT_NO_THROW(PropertyEventCall::sendSetPermissionManager(12345, QUrl::fromLocalFile("/tmp/test"), QFileDevice::ReadOwner));
}

TEST_F(TestPropertyDialog, PropertyEventCallSendFileHideTest)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    EXPECT_NO_THROW(PropertyEventCall::sendFileHide(12345, urls));
}

// Test PropertyMenuScene class
TEST_F(TestPropertyDialog, PropertyMenuCreatorNameTest)
{
    QString name = PropertyMenuCreator::name();
    EXPECT_EQ(name, "PropertyMenu");
}

TEST_F(TestPropertyDialog, PropertyMenuSceneNameTest)
{
    PropertyMenuScene scene;
    QString name = scene.name();
    EXPECT_EQ(name, "PropertyMenu");
}

// Test ComputerPropertyDialog class
TEST_F(TestPropertyDialog, ComputerPropertyDialogTest)
{
    EXPECT_NO_THROW({
        ComputerPropertyDialog dialog;
    });
}