// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/sendtomenuscene.h"
#include "menuscene/action_defines.h"
#include "menuscene/private/sendtomenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class StmsChannelReceiver : public QObject
{
public:
    explicit StmsChannelReceiver(QObject *parent = nullptr)
        : QObject(parent) { }

    bool btAvail()
    {
        return true;
    }
};

class UT_SendToMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new SendToMenuScene();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

    void stubPerfectMenuParams()
    {
        stub.set_lamda(&FileUtils::isComputerDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(ADDR(SystemPathUtil, isSystemPath), [](SystemPathUtil *, QString) {
            __DBG_STUB_INVOKE__
            return false;
        });
    }

    void stubInitializeBase()
    {
        stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                       [] {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
    }

    void stubFileInfoCreate()
    {
        stub.set_lamda(&InfoFactory::create<FileInfo>,
                       [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                           __DBG_STUB_INVOKE__
                           return QSharedPointer<FileInfo>(new FileInfo(url));
                       });
    }

    void stubCreateBase()
    {
        stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
            __DBG_STUB_INVOKE__
            return true;
        });
    }

    void stubNoDevices()
    {
        stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds),
                       [](DeviceProxyManager *, GlobalServerDefines::DeviceQueryOptions) -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });
    }

protected:
    SendToMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_SendToMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new SendToMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    SendToMenuCreator *creator { nullptr };
};

static QAction *stmsFindSendToAction(QMenu *menu)
{
    for (auto act : menu->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == ActionID::kSendTo)
            return act;
    }
    return nullptr;
}

TEST_F(UT_SendToMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(SendToMenuCreator::name(), "SendToMenu");
}

TEST_F(UT_SendToMenuCreator, Create_ReturnsSendToMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "SendToMenu");
    delete scene;
}

TEST_F(UT_SendToMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "SendToMenu");
}

TEST_F(UT_SendToMenuScene, Initialize_EmptySelectFiles_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    // no stub on initializeParamsIsValid: real check rejects empty select files
    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_SendToMenuScene, Initialize_ValidFiles_ReturnsTrue)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_SendToMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_SendToMenuScene, Create_NullParent_ReturnsFalse)
{
    EXPECT_FALSE(scene->create(nullptr));
}

TEST_F(UT_SendToMenuScene, Create_EmptyArea_DelegatesToBase)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kIsEmptyArea] = true;

    stubInitializeBase();
    stubCreateBase();

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(UT_SendToMenuScene, Create_SingleFile_AddsLinkAndDesktopActions)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stubNoDevices();

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    EXPECT_TRUE(sendToAction->isVisible());

    QMenu *sendToMenu = sendToAction->menu();
    ASSERT_NE(sendToMenu, nullptr);
    bool hasLink = false;
    bool hasDesktop = false;
    for (auto act : sendToMenu->actions()) {
        auto id = act->property(ActionPropertyKey::kActionID).toString();
        if (id == ActionID::kCreateSymlink)
            hasLink = true;
        else if (id == ActionID::kSendToDesktop)
            hasDesktop = true;
    }
    EXPECT_TRUE(hasLink);
    EXPECT_TRUE(hasDesktop);
}

TEST_F(UT_SendToMenuScene, Create_WithRemovableDevice_AddsDeviceAction)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds),
                   [](DeviceProxyManager *, GlobalServerDefines::DeviceQueryOptions) -> QStringList {
                       __DBG_STUB_INVOKE__
                       return QStringList { "dev-usb", "dev-usb", "dev-cdrom" };
                   });

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo),
                   [](DeviceProxyManager *, const QString &id, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       QVariantMap info;
                       info[GlobalServerDefines::DeviceProperty::kMountPoint] = "/media/usb";
                       info[GlobalServerDefines::DeviceProperty::kDevice] = "/dev/sdb1";
                       if (id == "dev-cdrom")
                           info[GlobalServerDefines::DeviceProperty::kOptical] = true;
                       return info;
                   });

    typedef QString (*ConvertDisplayNameFunc)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertDisplayNameFunc>(&DeviceUtils::convertSuitableDisplayName),
                   [](const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       return QString("USB Disk");
                   });
    typedef bool (*IsBuiltInDiskFunc)(const QVariantMap &);
    stub.set_lamda(static_cast<IsBuiltInDiskFunc>(&DeviceUtils::isBuiltInDisk),
                   [](const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    QMenu *sendToMenu = sendToAction->menu();
    ASSERT_NE(sendToMenu, nullptr);

    QAction *deviceAction { nullptr };
    for (auto act : sendToMenu->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString().startsWith(ActionID::kSendToRemovablePrefix))
            deviceAction = act;
    }
    ASSERT_NE(deviceAction, nullptr);
    EXPECT_EQ(deviceAction->text(), QString("USB Disk"));
    EXPECT_EQ(deviceAction->data().toUrl(), QUrl::fromLocalFile("/media/usb"));

    // the optical device should be skipped, only one removable action
    int removableCount = 0;
    for (auto act : sendToMenu->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString().startsWith(ActionID::kSendToRemovablePrefix))
            ++removableCount;
    }
    EXPECT_EQ(removableCount, 1);
}

TEST_F(UT_SendToMenuScene, Create_NoTargetItems_HidesSendToAction)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(
            QList<QUrl> { QUrl::fromLocalFile("/tmp/a.txt"), QUrl::fromLocalFile("/tmp/b.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = true;
    params[MenuParamKey::kIsSystemPathIncluded] = false;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = true;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stubNoDevices();

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    EXPECT_FALSE(sendToAction->isVisible());
}

TEST_F(UT_SendToMenuScene, Create_BluetoothAvailable_AddsBluetoothAction)
{
    // the slot topic must be registered before push/connect can resolve it
    dpfEvent->registerEventType(DPF_NAMESPACE::EventStratege::kSlot,
                                "dfmplugin_utils", "slot_Bluetooth_IsAvailable");
    DPF_NAMESPACE::EventType btType = dpfEvent->eventType("dfmplugin_utils", "slot_Bluetooth_IsAvailable");

    StmsChannelReceiver receiver;
    dpfSlotChannel->connect(btType, &receiver, &StmsChannelReceiver::btAvail);

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stubNoDevices();
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    QMenu *sendToMenu = sendToAction->menu();
    ASSERT_NE(sendToMenu, nullptr);

    QAction *bluetoothAction { nullptr };
    for (auto act : sendToMenu->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == ActionID::kSendToBluetooth)
            bluetoothAction = act;
    }
    ASSERT_NE(bluetoothAction, nullptr);
    EXPECT_TRUE(bluetoothAction->isEnabled());

    dpfSlotChannel->disconnect(btType);
}

TEST_F(UT_SendToMenuScene, UpdateState_DelegatesToBase)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&baseCalled](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    QMenu menu;
    scene->updateState(&menu);
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_SendToMenuScene, UpdateState_EmptyArea_SkipsDeviceCleanup)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&baseCalled](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kIsEmptyArea] = true;

    stubInitializeBase();
    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_SendToMenuScene, UpdateState_NotExistsFile_DisablesSendTo)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stubNoDevices();

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    stub.set_lamda(VADDR(FileInfo, exists), [](FileInfo *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    ASSERT_TRUE(sendToAction->isEnabled());

    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_FALSE(sendToAction->isEnabled());
}

TEST_F(UT_SendToMenuScene, UpdateState_RemovesDeviceSelfAction)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/media/usb/file.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stub.set_lamda(VADDR(FileInfo, exists), [](FileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds),
                   [](DeviceProxyManager *, GlobalServerDefines::DeviceQueryOptions) -> QStringList {
                       __DBG_STUB_INVOKE__
                       return QStringList { "dev-usb" };
                   });
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo),
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       __DBG_STUB_INVOKE__
                       QVariantMap info;
                       info[GlobalServerDefines::DeviceProperty::kMountPoint] = "/media/usb";
                       info[GlobalServerDefines::DeviceProperty::kDevice] = "/dev/sdb1";
                       return info;
                   });
    typedef QString (*ConvertDisplayNameFunc)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertDisplayNameFunc>(&DeviceUtils::convertSuitableDisplayName),
                   [](const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       return QString("USB Disk");
                   });
    typedef bool (*IsBuiltInDiskFunc)(const QVariantMap &);
    stub.set_lamda(static_cast<IsBuiltInDiskFunc>(&DeviceUtils::isBuiltInDisk),
                   [](const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    QMenu *sendToMenu = sendToAction->menu();
    ASSERT_NE(sendToMenu, nullptr);

    int countBefore = 0;
    for (auto act : sendToMenu->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString().startsWith(ActionID::kSendToRemovablePrefix))
            ++countBefore;
    }
    ASSERT_EQ(countBefore, 1);

    scene->updateState(&menu);

    int countAfter = 0;
    for (auto act : sendToMenu->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString().startsWith(ActionID::kSendToRemovablePrefix))
            ++countAfter;
    }
    EXPECT_EQ(countAfter, 0);
}

TEST_F(UT_SendToMenuScene, Triggered_CreateSymlinkAction_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kWindowId] = static_cast<quint64>(66);

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stubNoDevices();

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    QAction *linkAction { nullptr };
    for (auto act : sendToAction->menu()->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == ActionID::kCreateSymlink)
            linkAction = act;
    }
    ASSERT_NE(linkAction, nullptr);

    EXPECT_TRUE(scene->triggered(linkAction));
}

TEST_F(UT_SendToMenuScene, Triggered_SendToDesktopAction_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stubNoDevices();
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &, QList<QUrl> *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    QAction *desktopAction { nullptr };
    for (auto act : sendToAction->menu()->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == ActionID::kSendToDesktop)
            desktopAction = act;
    }
    ASSERT_NE(desktopAction, nullptr);

    EXPECT_TRUE(scene->triggered(desktopAction));
}

TEST_F(UT_SendToMenuScene, Triggered_BluetoothAction_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    ASSERT_TRUE(scene->initialize(params));

    stub.set_lamda(VADDR(FileInfo, pathOf), [](FileInfo *, const PathInfoType) {
        __DBG_STUB_INVOKE__
        return QString("/tmp/test.txt");
    });

    QMenu menu;
    QAction btAction("bluetooth");
    btAction.setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSendToBluetooth));
    menu.addAction(&btAction);
    scene->d.data()->predicateAction[ActionID::kSendToBluetooth] = &btAction;

    // the bluetooth branch pushes to slot channel then falls through
    EXPECT_FALSE(scene->triggered(&btAction));
}

TEST_F(UT_SendToMenuScene, Triggered_RemovableDeviceAction_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    QAction devAction("usb");
    devAction.setProperty(ActionPropertyKey::kActionID, QString("send-to-removable-0"));
    devAction.setData(QUrl::fromLocalFile("/media/usb"));
    menu.addAction(&devAction);
    scene->d.data()->predicateAction["send-to-removable-0"] = &devAction;

    EXPECT_TRUE(scene->triggered(&devAction));
}

TEST_F(UT_SendToMenuScene, Triggered_SendToTopAction_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stubNoDevices();

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);

    // own action without a matching branch
    EXPECT_FALSE(scene->triggered(sendToAction));
}

TEST_F(UT_SendToMenuScene, Triggered_NotOwnAction_DelegatesToBase)
{
    QAction action("test");
    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_SendToMenuScene, Triggered_NullAction_ReturnsFalse)
{
    EXPECT_FALSE(scene->triggered(nullptr));
}

TEST_F(UT_SendToMenuScene, Scene_OwnAction_ReturnsThis)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stubCreateBase();
    stubNoDevices();

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *sendToAction = stmsFindSendToAction(&menu);
    ASSERT_NE(sendToAction, nullptr);
    EXPECT_EQ(scene->scene(sendToAction), scene);
}
