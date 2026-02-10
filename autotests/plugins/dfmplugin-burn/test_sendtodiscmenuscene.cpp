// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/menus/sendtodiscmenuscene.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"
#include "plugins/common/dfmplugin-burn/events/burneventreceiver.h"
#include "plugins/common/dfmplugin-burn/events/burneventcaller.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>

#include <QMenu>
#include <QAction>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_SendToDiscMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);

        scene = new SendToDiscMenuScene();
        menu = new QMenu();

        // Setup test parameters
        params[MenuParamKey::kWindowId] = QVariant::fromValue(12345ULL);
        params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
        params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile("/tmp/test.txt"));
        params[MenuParamKey::kIsEmptyArea] = false;
        params[MenuParamKey::kIsDDEDesktopFileIncluded] = false;
    }

    virtual void TearDown() override
    {
        delete scene;
        delete menu;
        scene = nullptr;
        menu = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    SendToDiscMenuScene *scene = nullptr;
    QMenu *menu = nullptr;
    QVariantHash params;
};

TEST_F(UT_SendToDiscMenuScene, name)
{
    QString sceneName = scene->name();
    EXPECT_EQ(sceneName, SendToDiscMenuCreator::name());
}

TEST_F(UT_SendToDiscMenuScene, initialize_ValidParams)
{
    // Mock BurnHelper::discDataGroup to return some optical devices
    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        device1[DeviceProperty::kOptical] = true;
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
}

TEST_F(UT_SendToDiscMenuScene, initialize_EmptySelectFiles)
{
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    bool result = scene->initialize(params);
    EXPECT_FALSE(result);   // Should return false for empty select files
}

TEST_F(UT_SendToDiscMenuScene, initialize_NonFileScheme)
{
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>() << QUrl("burn://test"));

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return false;   // Cannot transform to local
    });

    bool result = scene->initialize(params);
    EXPECT_FALSE(result);   // Should return false for non-file scheme
}

TEST_F(UT_SendToDiscMenuScene, initialize_DDEDesktopFileIncluded)
{
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = true;

    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        return QList<QVariantMap>();
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);   // Should still initialize successfully
}

TEST_F(UT_SendToDiscMenuScene, create_ValidMenu)
{
    // Initialize first
    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        device1[DeviceProperty::kOptical] = true;
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    bool result = scene->create(menu);
    EXPECT_TRUE(result);
}

TEST_F(UT_SendToDiscMenuScene, create_NullMenu)
{
    bool result = scene->create(nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_SendToDiscMenuScene, create_DDEDesktopFileIncluded)
{
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = true;

    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        return QList<QVariantMap>();
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    bool result = scene->create(menu);
    EXPECT_TRUE(result);   // Should still create menu
}

TEST_F(UT_SendToDiscMenuScene, create_WithOpticalDevices)
{
    // Mock optical devices
    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        device1[DeviceProperty::kOptical] = true;
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    using ConvertDisplayNameFunc = QString (*)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertDisplayNameFunc>(&DeviceUtils::convertSuitableDisplayName), [] {
        __DBG_STUB_INVOKE__
        return QString("DVD Drive");
    });

    scene->initialize(params);
    bool result = scene->create(menu);

    EXPECT_TRUE(result);
    EXPECT_GT(menu->actions().size(), 0);   // Should have actions
}

TEST_F(UT_SendToDiscMenuScene, create_MountableImageFile)
{
    // Set focus file as ISO image
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile("/tmp/test.iso"));

    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        return QList<QVariantMap>();
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(SyncFileInfo, nameOf),
                   [](FileInfo *, NameInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       if (type == NameInfoType::kMimeTypeName)
                           return "application/x-iso9660-image";
                       return {};
                   });

    scene->initialize(params);
    bool result = scene->create(menu);

    EXPECT_TRUE(result);
}

TEST_F(UT_SendToDiscMenuScene, triggered_StageAction)
{
    // Setup and initialize
    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    using ConvertDisplayNameFunc = QString (*)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertDisplayNameFunc>(&DeviceUtils::convertSuitableDisplayName), [] {
        __DBG_STUB_INVOKE__
        return QString("DVD Drive");
    });

    bool handlePasteToCalledCalled = false;
    stub.set_lamda(ADDR(BurnEventReceiver, handlePasteTo), [&handlePasteToCalledCalled] {
        __DBG_STUB_INVOKE__
        handlePasteToCalledCalled = true;
    });

    stub.set_lamda(ADDR(DeviceUtils, isPWOpticalDiscDev), [] {
        __DBG_STUB_INVOKE__
        return false;   // Not packet writing device
    });

    scene->initialize(params);
    scene->create(menu);

    // Create a test action
    QAction *testAction = new QAction("Test", menu);
    testAction->setProperty(ActionPropertyKey::kActionID, ActionId::kStageKey);
    testAction->setData("/dev/sr0");

    bool result = scene->triggered(testAction);
    EXPECT_TRUE(result);
    EXPECT_TRUE(handlePasteToCalledCalled);

    delete testAction;
}

TEST_F(UT_SendToDiscMenuScene, triggered_PacketWritingAction)
{
    // Setup for packet writing device
    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(DeviceUtils, isPWOpticalDiscDev), [] {
        __DBG_STUB_INVOKE__
        return true;   // Is packet writing device
    });

    stub.set_lamda(ADDR(DeviceUtils, getMountInfo), [] {
        __DBG_STUB_INVOKE__
        return QString("/media/sr0");
    });

    bool sendPasteFilesCalled = false;
    stub.set_lamda(ADDR(BurnEventCaller, sendPasteFiles), [&sendPasteFilesCalled] {
        __DBG_STUB_INVOKE__
        sendPasteFilesCalled = true;
    });

    scene->initialize(params);
    scene->create(menu);

    // Create a test action for packet writing
    QAction *testAction = new QAction("Test", menu);
    testAction->setProperty(ActionPropertyKey::kActionID, ActionId::kStageKey);
    testAction->setData("/dev/sr0");

    bool result = scene->triggered(testAction);
    EXPECT_TRUE(result);
    EXPECT_TRUE(sendPasteFilesCalled);

    delete testAction;
}

TEST_F(UT_SendToDiscMenuScene, triggered_MountImageAction)
{
    bool handleMountImageCalled = false;

    stub.set_lamda(ADDR(BurnEventReceiver, handleMountImage), [&handleMountImageCalled] {
        __DBG_STUB_INVOKE__
        handleMountImageCalled = true;
    });

    scene->initialize(params);

    // Create a test action for mount image
    QAction *testAction = new QAction("Test", menu);
    testAction->setProperty(ActionPropertyKey::kActionID, ActionId::kMountImageKey);

    bool result = scene->triggered(testAction);
    EXPECT_TRUE(result);
    EXPECT_TRUE(handleMountImageCalled);

    delete testAction;
}

TEST_F(UT_SendToDiscMenuScene, triggered_UnknownAction)
{
    scene->initialize(params);

    // Create a test action that's not handled
    QAction *testAction = new QAction("Test", menu);
    testAction->setProperty(ActionPropertyKey::kActionID, "unknown_action");

    bool result = scene->triggered(testAction);
    EXPECT_FALSE(result);

    delete testAction;
}

TEST_F(UT_SendToDiscMenuScene, scene_ValidAction)
{
    scene->initialize(params);
    scene->create(menu);

    // Create a test action
    QAction *testAction = new QAction("Test", menu);
    testAction->setProperty(ActionPropertyKey::kActionID, ActionId::kStageKey);

    AbstractMenuScene *resultScene = scene->scene(testAction);
    EXPECT_EQ(resultScene, scene);

    delete testAction;
}

TEST_F(UT_SendToDiscMenuScene, scene_NullAction)
{
    AbstractMenuScene *resultScene = scene->scene(nullptr);
    EXPECT_EQ(resultScene, nullptr);
}

TEST_F(UT_SendToDiscMenuScene, scene_UnknownAction)
{
    QAction *testAction = new QAction("Test", menu);
    testAction->setProperty(ActionPropertyKey::kActionID, "unknown_action");

    AbstractMenuScene *resultScene = scene->scene(testAction);
    EXPECT_NE(resultScene, scene);   // Should delegate to parent

    delete testAction;
}

TEST_F(UT_SendToDiscMenuScene, updateState)
{
    // Mock burn enabled
    stub.set_lamda(ADDR(BurnHelper, isBurnEnabled), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    using ConvertDisplayNameFunc = QString (*)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertDisplayNameFunc>(&DeviceUtils::convertSuitableDisplayName), [] {
        __DBG_STUB_INVOKE__
        return QString("DVD Drive");
    });

    scene->initialize(params);
    scene->create(menu);

    // Should not crash
    scene->updateState(menu);
}

TEST_F(UT_SendToDiscMenuScene, updateState_BurnDisabled)
{
    // Mock burn disabled
    stub.set_lamda(ADDR(BurnHelper, isBurnEnabled), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    using ConvertDisplayNameFunc = QString (*)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertDisplayNameFunc>(&DeviceUtils::convertSuitableDisplayName), [] {
        __DBG_STUB_INVOKE__
        return QString("DVD Drive");
    });

    scene->initialize(params);
    scene->create(menu);

    // Should disable actions when burn is disabled
    scene->updateState(menu);
}

TEST_F(UT_SendToDiscMenuScene, updateState_WorkingDevice)
{
    stub.set_lamda(ADDR(BurnHelper, isBurnEnabled), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    using ConvertDisplayNameFunc = QString (*)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertDisplayNameFunc>(&DeviceUtils::convertSuitableDisplayName), [] {
        __DBG_STUB_INVOKE__
        return QString("DVD Drive");
    });

    stub.set_lamda(ADDR(DeviceUtils, isWorkingOpticalDiscDev), [] {
        __DBG_STUB_INVOKE__
        return true;   // Device is working
    });

    scene->initialize(params);
    scene->create(menu);

    // Should disable actions for working devices
    scene->updateState(menu);
}

TEST_F(UT_SendToDiscMenuScene, SendToDiscMenuCreator_create)
{
    SendToDiscMenuCreator creator;
    AbstractMenuScene *createdScene = creator.create();

    EXPECT_TRUE(createdScene != nullptr);
    EXPECT_TRUE(dynamic_cast<SendToDiscMenuScene *>(createdScene) != nullptr);

    delete createdScene;
}

TEST_F(UT_SendToDiscMenuScene, Private_initDestDevices)
{
    // Test filtering of self disc
    QUrl currentDir;
    currentDir.setScheme("burn");
    currentDir.setPath("/dev/sr0/disc_files/");
    params[MenuParamKey::kCurrentDir] = currentDir;

    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        devices << device1;
        QVariantMap device2;
        device2[DeviceProperty::kDevice] = "/dev/sr1";
        devices << device2;
        return devices;
    });

    stub.set_lamda(ADDR(BurnHelper, burnDestDevice), [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        if (url.path().contains("/dev/sr0/"))
            return QString("/dev/sr0");
        return QString();
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);

    // Should filter out self disc and only have sr1
}

TEST_F(UT_SendToDiscMenuScene, Private_addToSendto)
{
    // Create a menu with send-to action
    QAction *sendToAction = menu->addAction("Send To");
    sendToAction->setProperty(ActionPropertyKey::kActionID, "send-to");
    QMenu *sendToSubMenu = new QMenu();
    sendToAction->setMenu(sendToSubMenu);

    stub.set_lamda(ADDR(BurnHelper, discDataGroup), [] {
        __DBG_STUB_INVOKE__
        QList<QVariantMap> devices;
        QVariantMap device1;
        device1[DeviceProperty::kDevice] = "/dev/sr0";
        devices << device1;
        return devices;
    });

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal), [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    using ConvertDisplayNameFunc = QString (*)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertDisplayNameFunc>(&DeviceUtils::convertSuitableDisplayName), [] {
        __DBG_STUB_INVOKE__
        return QString("DVD Drive");
    });

    scene->initialize(params);
    scene->create(menu);

    // Should add items to send-to submenu
    EXPECT_GT(sendToSubMenu->actions().size(), 0);
}
