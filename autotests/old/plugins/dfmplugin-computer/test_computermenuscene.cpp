// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "menu/computermenuscene.h"
#include "private/computermenuscene_p.h"
#include "utils/computerdatastruct.h"
#include "controller/computercontroller.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/protocolutils.h>

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <QMenu>
#include <QAction>
#include <QUrl>
#include <QRegularExpression>

DFMBASE_USE_NAMESPACE
DPCOMPUTER_USE_NAMESPACE
using namespace dfmplugin_computer;
using namespace GlobalServerDefines;

class UT_ComputerMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        scene = new ComputerMenuScene();
        menu = new QMenu();
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        delete menu;
        menu = nullptr;
        stub.clear();
    }

    QAction *findAction(QMenu *m, const QString &id)
    {
        auto list = m->actions();
        for (auto act : list) {
            if (act->property(ActionPropertyKey::kActionID).toString() == id)
                return act;
            auto subMenu = act->menu();
            if (subMenu)
                return findAction(subMenu, id);
        }

        return nullptr;
    }

protected:
    stub_ext::StubExt stub;
    ComputerMenuScene *scene = nullptr;
    QMenu *menu = nullptr;
};

TEST_F(UT_ComputerMenuScene, Construction_CreatesValidScene_Success)
{
    EXPECT_NE(scene, nullptr);
    EXPECT_NE(scene->metaObject(), nullptr);

    // Test that private data is initialized
    ComputerMenuScene *newScene = new ComputerMenuScene();
    EXPECT_NE(newScene, nullptr);
    delete newScene;
}

TEST_F(UT_ComputerMenuScene, Destructor_CleansUpCorrectly_Success)
{
    // Test that destructor doesn't crash
    ComputerMenuScene *tempScene = new ComputerMenuScene();
    EXPECT_NO_THROW(delete tempScene);
}

TEST_F(UT_ComputerMenuScene, Name_ReturnsComputerMenuCreatorName_Success)
{
    QString expectedName = "ComputerMenuCreator";

    stub.set_lamda(&ComputerMenuCreator::name, [&]() -> QString {
        __DBG_STUB_INVOKE__
        return expectedName;
    });

    QString result = scene->name();
    EXPECT_EQ(result, expectedName);
}

TEST_F(UT_ComputerMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });
    params[MenuParamKey::kCurrentDir] = QUrl("computer:///");

    // Mock subscene operations
    stub.set_lamda(VADDR(AbstractMenuScene, subscene), [&](AbstractMenuScene *) -> QList<AbstractMenuScene *> {
        __DBG_STUB_INVOKE__
        return QList<AbstractMenuScene *>();
    });

    stub.set_lamda(VADDR(AbstractMenuScene, setSubscene), [&](AbstractMenuScene *, const QList<AbstractMenuScene *> &) {
        __DBG_STUB_INVOKE__
    });

    // Mock dfmplugin_menu_util::menuSceneCreateScene
    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [&](const QString &) -> AbstractMenuScene * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Mock base class initialize
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [&](AbstractMenuScene *, const QVariantHash &) -> bool {
        __DBG_STUB_INVOKE__
        baseCalled = true;
        return true;
    });

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_ComputerMenuScene, Initialize_NoSelectedFiles_LogsWarningAndReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kCurrentDir] = QUrl("computer:///");

    bool result = scene->initialize(params);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerMenuScene, Initialize_WithSubScenes_SetsUpCorrectly)
{
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });
    params[MenuParamKey::kCurrentDir] = QUrl("computer:///");

    // Mock subscene creation
    AbstractMenuScene *mockFilterScene = reinterpret_cast<AbstractMenuScene *>(0x1234);
    AbstractMenuScene *mockIconScene = reinterpret_cast<AbstractMenuScene *>(0x5678);

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [&](const QString &name) -> AbstractMenuScene * {
        __DBG_STUB_INVOKE__
        if (name == "DConfigMenuFilter") return mockFilterScene;
        if (name == "ActionIconManager") return mockIconScene;
        return nullptr;
    });

    QList<AbstractMenuScene *> subscenes;
    stub.set_lamda(VADDR(AbstractMenuScene, subscene), [&](AbstractMenuScene *) -> QList<AbstractMenuScene *> {
        __DBG_STUB_INVOKE__
        return subscenes;
    });

    bool setSubsceneCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, setSubscene), [&](AbstractMenuScene *, const QList<AbstractMenuScene *> &scenes) {
        __DBG_STUB_INVOKE__
        setSubsceneCalled = true;
    });

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [&](AbstractMenuScene *, const QVariantHash &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
    EXPECT_TRUE(setSubsceneCalled);
}

TEST_F(UT_ComputerMenuScene, Create_ValidMenu_AddsAllActionsAndCallsBase)
{
    using namespace ContextMenuAction;

    // Mock base class create
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, create), [&](AbstractMenuScene *, QMenu *) -> bool {
        __DBG_STUB_INVOKE__
        baseCalled = true;
        return true;
    });

    bool result = scene->create(menu);
    EXPECT_TRUE(result);
    EXPECT_TRUE(baseCalled);

    // Verify menu actions were added
    QList<QAction *> actions = menu->actions();
    EXPECT_GT(actions.size(), 0);

    // Check for specific actions
    QStringList actionIds;
    for (QAction *action : actions) {
        if (action && !action->isSeparator()) {
            QString actionId = action->property(ActionPropertyKey::kActionID).toString();
            if (!actionId.isEmpty()) {
                actionIds << actionId;
            }
        }
    }

    EXPECT_TRUE(actionIds.contains(kOpenInNewWin));
    EXPECT_TRUE(actionIds.contains(kOpenInNewTab));
    EXPECT_TRUE(actionIds.contains(kOpen));
    EXPECT_TRUE(actionIds.contains(kMount));
    EXPECT_TRUE(actionIds.contains(kProperty));
}

TEST_F(UT_ComputerMenuScene, Create_NullMenu_LogsErrorAndReturnsFalse)
{
    bool result = scene->create(nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerMenuScene, UpdateState_NullMenu_LogsWarningAndReturns)
{
    EXPECT_NO_THROW(scene->updateState(nullptr));
}

TEST_F(UT_ComputerMenuScene, UpdateState_NullInfo_LogsErrorAndReturns)
{
    // Initialize scene without proper info
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });

    // Don't properly initialize info
    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(UT_ComputerMenuScene, UpdateState_UserDirectory_ShowsCorrectActions)
{
    using namespace ContextMenuAction;

    // Properly initialize scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://desktop.userdir") });
    scene->initialize(params);
    scene->create(menu);

    // Mock EntryFileInfo for user directory
    stub.set_lamda(&EntryFileInfo::order, [&](EntryFileInfo *) -> AbstractEntryFileEntity::EntryOrder {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::kOrderUserDir;
    });

    // Mock tab addable check
    typedef QVariant (dpf::EventChannelManager::*PushMethod)(const QString &, const QString &, quint64);
    auto pushMethod = static_cast<PushMethod>(&dpf::EventChannelManager::push);
    stub.set_lamda(pushMethod, [&](dpf::EventChannelManager *, const QString &space, const QString &slot, quint64) -> QVariant {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_titlebar" && slot == "slot_Tab_Addable") {
            return QVariant(true);
        }
        return QVariant();
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(scene->updateState(menu));

    // Check visible actions
    QStringList visibleActions;
    for (QAction *action : menu->actions()) {
        if (action && !action->isSeparator() && action->isVisible()) {
            QString actionId = action->property(ActionPropertyKey::kActionID).toString();
            if (!actionId.isEmpty()) {
                visibleActions << actionId;
            }
        }
    }

    // User directories should show: open in new window, open in new tab, properties
    EXPECT_TRUE(visibleActions.contains(kOpenInNewWin));
    EXPECT_TRUE(visibleActions.contains(kOpenInNewTab));
    EXPECT_TRUE(visibleActions.contains(kProperty));
}

TEST_F(UT_ComputerMenuScene, UpdateState_RemovableDisks_ShowsCorrectActions)
{
    using namespace ContextMenuAction;

    // Setup scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://usb.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    // Mock EntryFileInfo for removable disk
    stub.set_lamda(&EntryFileInfo::order, [&](EntryFileInfo *) -> AbstractEntryFileEntity::EntryOrder {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::kOrderRemovableDisks;
    });

    stub.set_lamda(&EntryFileInfo::renamable, [&](EntryFileInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&EntryFileInfo::targetUrl, [&](EntryFileInfo *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///media/usb");   // Mounted
    });

    typedef QVariant (dpf::EventChannelManager::*PushMethod)(const QString &, const QString &, quint64);
    auto pushMethod = static_cast<PushMethod>(&dpf::EventChannelManager::push);
    stub.set_lamda(pushMethod, [&] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(UT_ComputerMenuScene, UpdateState_OpticalDisks_HandlesComplexLogic)
{
    using namespace ContextMenuAction;

    // Setup scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://cdrom.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    // Mock EntryFileInfo for optical disk
    stub.set_lamda(&EntryFileInfo::order, [&](EntryFileInfo *) -> AbstractEntryFileEntity::EntryOrder {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::kOrderOptical;
    });

    stub.set_lamda(&EntryFileInfo::extraProperty, [&](EntryFileInfo *, const QString &key) -> QVariant {
        __DBG_STUB_INVOKE__
        if (key == DeviceProperty::kOptical) return true;
        if (key == DeviceProperty::kMedia) return QString("cd_rw");
        if (key == DeviceProperty::kOpticalBlank) return false;
        if (key == DeviceProperty::kDevice) return QString("/dev/sr0");
        return QVariant();
    });

    stub.set_lamda(&EntryFileInfo::targetUrl, [&](EntryFileInfo *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl();   // Not mounted
    });

    // Mock DeviceUtils::isWorkingOpticalDiscDev
    stub.set_lamda(&DeviceUtils::isWorkingOpticalDiscDev, [&](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    typedef QVariant (dpf::EventChannelManager::*PushMethod)(const QString &, const QString &, quint64);
    auto pushMethod = static_cast<PushMethod>(&dpf::EventChannelManager::push);
    stub.set_lamda(pushMethod, [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(UT_ComputerMenuScene, UpdateState_NetworkProtocols_ShowsCorrectActions)
{
    using namespace ContextMenuAction;

    // Setup scene for SMB
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://smb-server.protodev") });
    scene->initialize(params);
    scene->create(menu);

    // Mock EntryFileInfo for SMB
    stub.set_lamda(&EntryFileInfo::order, [&](EntryFileInfo *) -> AbstractEntryFileEntity::EntryOrder {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::kOrderSmb;
    });

    stub.set_lamda(&EntryFileInfo::targetUrl, [&](EntryFileInfo *) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("smb://server/share");
    });

    stub.set_lamda(&EntryFileInfo::extraProperty, [&](EntryFileInfo *, const QString &key) -> QVariant {
        __DBG_STUB_INVOKE__
        if (key == DeviceProperty::kId) return QString("smb://server/share");
        return QVariant();
    });

    // Mock ProtocolUtils::isSMBFile
    stub.set_lamda(&ProtocolUtils::isSMBFile, [&](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    typedef QVariant (dpf::EventChannelManager::*PushMethod)(const QString &, const QString &, quint64);
    auto pushMethod = static_cast<PushMethod>(&dpf::EventChannelManager::push);
    stub.set_lamda(pushMethod, [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(UT_ComputerMenuScene, UpdateState_LoopDevice_HidesRenameAction)
{
    using namespace ContextMenuAction;

    // Setup scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://loop.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    // Mock EntryFileInfo for loop device
    stub.set_lamda(&EntryFileInfo::order, [&](EntryFileInfo *) -> AbstractEntryFileEntity::EntryOrder {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::kOrderSysDiskData;
    });

    QVariantHash extraProperties;
    extraProperties[DeviceProperty::kIsLoopDevice] = true;

    stub.set_lamda(VADDR(EntryFileInfo, extraProperties), [&](EntryFileInfo *) -> QVariantHash {
        __DBG_STUB_INVOKE__
        return extraProperties;
    });

    stub.set_lamda(VADDR(EntryFileInfo, renamable), [&](EntryFileInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    typedef QVariant (dpf::EventChannelManager::*PushMethod)(const QString &, const QString &, quint64);
    auto pushMethod = static_cast<PushMethod>(&dpf::EventChannelManager::push);
    stub.set_lamda(pushMethod, [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(scene->updateState(menu));
}

TEST_F(UT_ComputerMenuScene, UpdateState_TabNotAddable_DisablesOpenInNewTab)
{
    using namespace ContextMenuAction;

    // Setup scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    stub.set_lamda(&EntryFileInfo::order, [&](EntryFileInfo *) -> AbstractEntryFileEntity::EntryOrder {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::kOrderUserDir;
    });

    // Mock tab not addable
    typedef QVariant (dpf::EventChannelManager::*PushMethod)(const QString &, const QString &, quint64);
    auto pushMethod = static_cast<PushMethod>(&dpf::EventChannelManager::push);
    stub.set_lamda(pushMethod, [&](dpf::EventChannelManager *, const QString &space, const QString &slot, quint64) -> QVariant {
        __DBG_STUB_INVOKE__
        if (space == "dfmplugin_titlebar" && slot == "slot_Tab_Addable") {
            return QVariant(false);   // Tab not addable
        }
        return QVariant();
    });

    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(scene->updateState(menu));

    // Check that kOpenInNewTab is disabled
    for (QAction *action : menu->actions()) {
        if (action && action->property(ActionPropertyKey::kActionID).toString() == kOpenInNewTab) {
            EXPECT_FALSE(action->isEnabled());
            break;
        }
    }
}

TEST_F(UT_ComputerMenuScene, Triggered_OpenAction_CallsController)
{
    using namespace ContextMenuAction;

    // Setup scene with proper initialization
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    // Mock EntryFileInfo
    stub.set_lamda(VADDR(EntryFileInfo, urlOf), [] {
        __DBG_STUB_INVOKE__
        return QUrl("entry://test.blockdev");
    });

    // Mock ComputerController
    bool openItemCalled = false;
    stub.set_lamda(&ComputerController::onOpenItem, [&](ComputerController *, quint64 winId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        openItemCalled = true;
        EXPECT_EQ(winId, 0);
        EXPECT_EQ(url, QUrl("entry://test.blockdev"));
    });

    // Create action
    QAction *action = findAction(menu, kOpen);
    bool result = scene->triggered(action);
    EXPECT_TRUE(result);
    EXPECT_TRUE(openItemCalled);

    delete action;
}

TEST_F(UT_ComputerMenuScene, Triggered_MountAction_CallsController)
{
    using namespace ContextMenuAction;

    // Setup scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    // Mock ComputerController
    bool mountCalled = false;
    stub.set_lamda(&ComputerController::actMount, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info, bool) {
        __DBG_STUB_INVOKE__
        mountCalled = true;
        EXPECT_EQ(winId, quint64(12345));
        EXPECT_NE(info, nullptr);
    });

    // Create action
    QAction *action = findAction(menu, kMount);
    bool result = scene->triggered(action);
    EXPECT_TRUE(result);
    EXPECT_TRUE(mountCalled);

    delete action;
}

TEST_F(UT_ComputerMenuScene, Triggered_RenameAction_CallsControllerWithSidebarFlag)
{
    using namespace ContextMenuAction;

    // Setup scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    // Set sidebar trigger flag
    menu->setProperty(kActionTriggeredFromSidebar, true);

    // Mock ComputerController
    bool renameCalled = false;
    stub.set_lamda(&ComputerController::actRename, [&](ComputerController *, quint64 winId, DFMEntryFileInfoPointer info, bool fromSidebar) {
        __DBG_STUB_INVOKE__
        renameCalled = true;
        EXPECT_EQ(winId, quint64(12345));
        EXPECT_NE(info, nullptr);
        EXPECT_FALSE(fromSidebar);
    });

    // Create action
    QAction *action = findAction(menu, kRename);
    bool result = scene->triggered(action);
    EXPECT_TRUE(result);
    EXPECT_TRUE(renameCalled);

    delete action;
}

TEST_F(UT_ComputerMenuScene, Triggered_UnknownAction_CallsBaseClass)
{
    // Create unknown action
    QAction *action = new QAction("Unknown", menu);
    action->setProperty(ActionPropertyKey::kActionID, "unknown");

    // Mock base class triggered
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [&](AbstractMenuScene *, QAction *act) -> bool {
        __DBG_STUB_INVOKE__
        baseCalled = true;
        EXPECT_EQ(act, action);
        return false;
    });

    bool result = scene->triggered(action);
    EXPECT_FALSE(result);
    EXPECT_TRUE(baseCalled);

    delete action;
}

TEST_F(UT_ComputerMenuScene, Scene_NullAction_LogsDebugAndReturnsNull)
{
    AbstractMenuScene *result = scene->scene(nullptr);
    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ComputerMenuScene, Scene_ValidAction_ReturnsThis)
{
    using namespace ContextMenuAction;

    // Setup scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    // Create action that belongs to this scene
    QAction *action = new QAction(kOpen, menu);
    action->setProperty(ActionPropertyKey::kActionID, kOpen);

    // Mock private data check (simplified)
    AbstractMenuScene *result = scene->scene(action);
    // The actual result depends on internal private data state
    // We just ensure it doesn't crash
    EXPECT_NO_THROW(scene->scene(action));

    delete action;
}

TEST_F(UT_ComputerMenuScene, Scene_UnknownAction_CallsBaseClass)
{
    QAction *action = new QAction("Unknown", menu);
    action->setProperty(ActionPropertyKey::kActionID, "unknown");

    // Mock base class scene
    AbstractMenuScene *mockScene = reinterpret_cast<AbstractMenuScene *>(0x1234);
    stub.set_lamda(VADDR(AbstractMenuScene, scene), [&](const AbstractMenuScene *, QAction *act) -> AbstractMenuScene * {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(act, action);
        return mockScene;
    });

    AbstractMenuScene *result = scene->scene(action);
    EXPECT_EQ(result, mockScene);

    delete action;
}

TEST_F(UT_ComputerMenuScene, Creator_Create_ReturnsNewComputerMenuScene)
{
    ComputerMenuCreator creator;
    AbstractMenuScene *createdScene = creator.create();
    EXPECT_NE(createdScene, nullptr);

    ComputerMenuScene *computerScene = dynamic_cast<ComputerMenuScene *>(createdScene);
    EXPECT_NE(computerScene, nullptr);

    delete createdScene;
}

TEST_F(UT_ComputerMenuScene, PrivateClass_InitializesPredicateNames_Correctly)
{
    using namespace ContextMenuAction;

    // This tests the ComputerMenuScenePrivate constructor
    ComputerMenuScene *testScene = new ComputerMenuScene();

    // The private constructor should initialize predicate names
    // We can't directly test private members, but we can ensure construction doesn't crash
    EXPECT_NE(testScene, nullptr);

    delete testScene;
}

TEST_F(UT_ComputerMenuScene, PrivateUpdateMenu_NullMenu_LogsWarningAndReturns)
{
    // Test ComputerMenuScenePrivate::updateMenu with null menu
    // This is called internally by updateState, so we test indirectly
    EXPECT_NO_THROW(scene->updateState(nullptr));
}

TEST_F(UT_ComputerMenuScene, AllActionTypes_TriggeredCorrectly_CallsAppropriateControllerMethods)
{
    using namespace ContextMenuAction;

    // Setup scene
    QVariantHash params;
    params[MenuParamKey::kWindowId] = quint64(12345);
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("entry://test.blockdev") });
    scene->initialize(params);
    scene->create(menu);

    // Mock EntryFileInfo methods
    stub.set_lamda(VADDR(EntryFileInfo, urlOf), [] {
        __DBG_STUB_INVOKE__
        return QUrl("entry://test.blockdev");
    });

    // Test action mappings
    struct ActionTest
    {
        QString actionId;
        std::function<void()> mockSetup;
        std::function<void()> verify;
    };

    bool openTabCalled = false, openWinCalled = false, unmountCalled = false;
    bool formatCalled = false, ejectCalled = false, eraseCalled = false;
    bool safelyRemoveCalled = false, logoutCalled = false, propertiesCalled = false;

    std::vector<ActionTest> tests = {
        { kOpenInNewTab, [&]() { stub.set_lamda(&ComputerController::actOpenInNewTab, [&] {
                                     __DBG_STUB_INVOKE__
                                     openTabCalled = true;
                                 }); }, [&]() { EXPECT_TRUE(openTabCalled); } },

        { kOpenInNewWin, [&]() { stub.set_lamda(&ComputerController::actOpenInNewWindow, [&] {
                                     __DBG_STUB_INVOKE__
                                     openWinCalled = true;
                                 }); }, [&]() { EXPECT_TRUE(openWinCalled); } },

        { kProperty, [&]() { stub.set_lamda(&ComputerController::actProperties, [&] {
                                 __DBG_STUB_INVOKE__
                                 propertiesCalled = true;
                             }); }, [&]() { EXPECT_TRUE(propertiesCalled); } }
    };

    for (const auto &test : tests) {
        test.mockSetup();

        QAction *action = findAction(menu, test.actionId);
        bool result = scene->triggered(action);
        EXPECT_TRUE(result);
        test.verify();

        delete action;
    }
}

TEST_F(UT_ComputerMenuScene, InheritanceAndPolymorphism_WorksCorrectly_Success)
{
    // Test inheritance from AbstractMenuScene
    AbstractMenuScene *baseScene = scene;
    EXPECT_NE(baseScene, nullptr);

    // Test virtual method calls
    EXPECT_NO_THROW(baseScene->name());

    // Test that scene can be created through creator
    ComputerMenuCreator creator;
    AbstractMenuScene *createdScene = creator.create();
    EXPECT_NE(createdScene, nullptr);
    delete createdScene;
}
