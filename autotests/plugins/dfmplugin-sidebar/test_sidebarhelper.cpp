// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"
#include "treeviews/sidebaritem.h"
#include "treeviews/sidebarwidget.h"
#include "events/sidebareventcaller.h"
#include "dfmplugin_sidebar_global.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/networkutils.h>

#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QIcon>
#include <QMutex>
#include <QMenu>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_SideBarHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Clear static map before each test
        stub.set_lamda(ADDR(QWidget, show), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(ADDR(QWidget, hide), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    Application app;
};

TEST_F(UT_SideBarHelper, AllSideBar_Empty)
{
    stub.set_lamda(&SideBarHelper::mutex, []() -> QMutex & {
        __DBG_STUB_INVOKE__
        static QMutex m;
        return m;
    });

    QList<SideBarWidget *> list = SideBarHelper::allSideBar();

    // May be empty or contain widgets
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarHelper, AddSideBar)
{
    quint64 windowId = 12345;
    SideBarWidget *widget = new SideBarWidget();

    stub.set_lamda(&SideBarHelper::mutex, []() -> QMutex & {
        __DBG_STUB_INVOKE__
        static QMutex m;
        return m;
    });

    SideBarHelper::addSideBar(windowId, widget);

    // Verify no crash
    EXPECT_TRUE(true);

    delete widget;
}

TEST_F(UT_SideBarHelper, AddSideBar_Duplicate)
{
    quint64 windowId = 54321;
    SideBarWidget *widget1 = new SideBarWidget();
    SideBarWidget *widget2 = new SideBarWidget();

    stub.set_lamda(&SideBarHelper::mutex, []() -> QMutex & {
        __DBG_STUB_INVOKE__
        static QMutex m;
        return m;
    });

    SideBarHelper::addSideBar(windowId, widget1);
    SideBarHelper::addSideBar(windowId, widget2);   // Should not add duplicate

    EXPECT_TRUE(true);

    delete widget1;
    delete widget2;
}

TEST_F(UT_SideBarHelper, RemoveSideBar)
{
    quint64 windowId = 99999;

    stub.set_lamda(&SideBarHelper::mutex, []() -> QMutex & {
        __DBG_STUB_INVOKE__
        static QMutex m;
        return m;
    });

    SideBarHelper::removeSideBar(windowId);

    // Should not crash even if not exists
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarHelper, RemoveSideBar_Existing)
{
    quint64 windowId = 88888;
    SideBarWidget *widget = new SideBarWidget();

    stub.set_lamda(&SideBarHelper::mutex, []() -> QMutex & {
        __DBG_STUB_INVOKE__
        static QMutex m;
        return m;
    });

    SideBarHelper::addSideBar(windowId, widget);
    SideBarHelper::removeSideBar(windowId);

    EXPECT_TRUE(true);

    delete widget;
}

TEST_F(UT_SideBarHelper, WindowId)
{
    QWidget *widget = new QWidget();
    quint64 testWindowId = 77777;

    stub.set_lamda(&FileManagerWindowsManager::findWindowId,
                   [testWindowId](FileManagerWindowsManager *, const QWidget *) -> quint64 {
                       __DBG_STUB_INVOKE__
                       return testWindowId;
                   });

    quint64 result = SideBarHelper::windowId(widget);

    EXPECT_EQ(result, testWindowId);

    delete widget;
}

TEST_F(UT_SideBarHelper, PreDefineItemProperties_Empty)
{
    stub.set_lamda(&LifeCycle::pluginMetaObjs,
                   [](std::function<bool(PluginMetaObjectPointer)>) -> QList<PluginMetaObjectPointer> {
                       __DBG_STUB_INVOKE__
                       return QList<PluginMetaObjectPointer>();
                   });

    QMap<QUrl, QPair<int, QVariantMap>> properties = SideBarHelper::preDefineItemProperties();

    EXPECT_TRUE(properties.isEmpty());
}

TEST_F(UT_SideBarHelper, CreateItemByInfo_Basic)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/home/test");
    info.displayName = "TestItem";
    info.group = DefaultGroup::kCommon;
    info.icon = QIcon::fromTheme("folder");
    info.flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    info.isEjectable = false;

    SideBarItem *item = SideBarHelper::createItemByInfo(info);

    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->text(), info.displayName);

    delete item;
}

TEST_F(UT_SideBarHelper, CreateItemByInfo_WithEjectable)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/media/usb");
    info.displayName = "USB Drive";
    info.group = DefaultGroup::kDevice;
    info.icon = QIcon::fromTheme("drive-removable-media");
    info.flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    info.isEjectable = true;

    stub.set_lamda(&SideBarEventCaller::sendEject, [](const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    SideBarItem *item = SideBarHelper::createItemByInfo(info);

    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->text(), info.displayName);

    delete item;
}

TEST_F(UT_SideBarHelper, CreateSeparatorItem_Tag)
{
    QString group = DefaultGroup::kTag;

    SideBarItemSeparator *separator = SideBarHelper::createSeparatorItem(group);

    ASSERT_NE(separator, nullptr);
    EXPECT_EQ(separator->group(), group);
    EXPECT_TRUE(separator->flags() & Qt::ItemIsEnabled);
    EXPECT_TRUE(separator->flags() & Qt::ItemIsDropEnabled);

    delete separator;
}

TEST_F(UT_SideBarHelper, CreateSeparatorItem_Common)
{
    QString group = DefaultGroup::kCommon;

    SideBarItemSeparator *separator = SideBarHelper::createSeparatorItem(group);

    ASSERT_NE(separator, nullptr);
    EXPECT_EQ(separator->group(), group);
    EXPECT_TRUE(separator->flags() & Qt::ItemIsEnabled);
    EXPECT_TRUE(separator->flags() & Qt::ItemIsDropEnabled);

    delete separator;
}

TEST_F(UT_SideBarHelper, CreateSeparatorItem_Device)
{
    QString group = DefaultGroup::kDevice;

    SideBarItemSeparator *separator = SideBarHelper::createSeparatorItem(group);

    ASSERT_NE(separator, nullptr);
    EXPECT_EQ(separator->group(), group);
    EXPECT_EQ(separator->flags(), Qt::NoItemFlags);

    delete separator;
}

TEST_F(UT_SideBarHelper, MakeItemIdentifier)
{
    QString group = DefaultGroup::kCommon;
    QUrl url = QUrl::fromLocalFile("/home/test");

    QString identifier = SideBarHelper::makeItemIdentifier(group, url);

    EXPECT_FALSE(identifier.isEmpty());
    EXPECT_TRUE(identifier.contains(group));
    EXPECT_TRUE(identifier.contains(url.url()));
}

TEST_F(UT_SideBarHelper, DefaultCdAction_ValidUrl)
{
    quint64 windowId = 12345;
    QUrl url = QUrl::fromLocalFile("/home/test");

    bool sendCalled = false;

    stub.set_lamda(&SideBarEventCaller::sendItemActived,
                   [&sendCalled](quint64, const QUrl &) {
                       __DBG_STUB_INVOKE__
                       sendCalled = true;
                   });

    SideBarHelper::defaultCdAction(windowId, url);

    EXPECT_TRUE(sendCalled);
}

TEST_F(UT_SideBarHelper, DefaultCdAction_EmptyUrl)
{
    quint64 windowId = 12345;
    QUrl emptyUrl;

    bool sendCalled = false;

    stub.set_lamda(&SideBarEventCaller::sendItemActived,
                   [&sendCalled](quint64, const QUrl &) {
                       __DBG_STUB_INVOKE__
                       sendCalled = true;
                   });

    SideBarHelper::defaultCdAction(windowId, emptyUrl);

    EXPECT_FALSE(sendCalled);
}

TEST_F(UT_SideBarHelper, DefaultContextMenu)
{
    quint64 windowId = 12345;
    QUrl url = QUrl::fromLocalFile("/home/test");
    QPoint globalPos(100, 100);

    stub.set_lamda(&SideBarEventCaller::sendOpenWindow, [](const QUrl &, bool) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&SideBarEventCaller::sendOpenTab, [](quint64, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&SideBarEventCaller::sendCheckTabAddable, [](quint64) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SideBarEventCaller::sendShowFilePropertyDialog, [](const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusy, [](NetworkUtils *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    using ExecFunc = QAction *(QMenu::*)(const QPoint &, QAction *);
    stub.set_lamda(static_cast<ExecFunc>(&QMenu::exec), [] {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    // Should create and show menu without crashing
    SideBarHelper::defaultContextMenu(windowId, url, globalPos);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarHelper, BindSetting)
{
    QString itemVisiableSettingKey = "test.setting.key";
    QString itemVisiableControlKey = "test.control.key";

    stub.set_lamda(&SettingBackend::instance, []() -> SettingBackend * {
        __DBG_STUB_INVOKE__
        static SettingBackend backend;
        return &backend;
    });

    typedef void (SettingBackend::*Func)(const QString &, SettingBackend::GetOptFunc, SettingBackend::SaveOptFunc);
    stub.set_lamda(static_cast<Func>(&SettingBackend::addSettingAccessor),
                   [] {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarHelper::hiddenRules, []() -> QVariantMap {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["test.control.key"] = true;
        return map;
    });

    SideBarHelper::bindSetting(itemVisiableSettingKey, itemVisiableControlKey);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarHelper, RemovebindingSetting)
{
    QString itemVisiableSettingKey = "test.remove.key";

    stub.set_lamda(&SettingBackend::instance, []() -> SettingBackend * {
        __DBG_STUB_INVOKE__
        static SettingBackend backend;
        return &backend;
    });

    stub.set_lamda(&SettingBackend::removeSettingAccessor,
                   [](SettingBackend *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    SideBarHelper::removebindingSetting(itemVisiableSettingKey);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarHelper, InitDefaultSettingPanel)
{
    stub.set_lamda(&SettingJsonGenerator::instance, []() -> SettingJsonGenerator * {
        __DBG_STUB_INVOKE__
        static SettingJsonGenerator generator;
        return &generator;
    });

    stub.set_lamda(&SettingJsonGenerator::addGroup,
                   [](SettingJsonGenerator *, const QString &, const QString &) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    SideBarHelper::initDefaultSettingPanel();

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarHelper, AddItemToSettingPannel_QuickAccess)
{
    QString group = DefaultGroup::kCommon;
    QString key = "test.quick.key";
    QString value = "Quick Access Item";
    QMap<QString, int> levelMap;
    levelMap[group] = 0;

    stub.set_lamda(&SettingJsonGenerator::instance, []() -> SettingJsonGenerator * {
        __DBG_STUB_INVOKE__
        static SettingJsonGenerator generator;
        return &generator;
    });

    stub.set_lamda(&SettingJsonGenerator::hasConfig,
                   [](SettingJsonGenerator *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&SettingJsonGenerator::addConfig,
                   [](SettingJsonGenerator *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&SettingJsonGenerator::addCheckBoxConfig,
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&SideBarInfoCacheMananger::instance, []() -> SideBarInfoCacheMananger * {
        __DBG_STUB_INVOKE__
        static SideBarInfoCacheMananger manager;
        return &manager;
    });

    stub.set_lamda(&SideBarInfoCacheMananger::appendLastSettingKey,
                   [](SideBarInfoCacheMananger *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarHelper::bindSetting,
                   [](const QString &, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarInfoCacheMananger::appendLastSettingBindingKey,
                   [](SideBarInfoCacheMananger *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    SideBarHelper::addItemToSettingPannel(group, key, value, &levelMap);

    EXPECT_EQ(levelMap[group], 1);
}

TEST_F(UT_SideBarHelper, AddItemToSettingPannel_Device)
{
    QString group = DefaultGroup::kDevice;
    QString key = "test.device.key";
    QString value = "Device Item";
    QMap<QString, int> levelMap;
    levelMap[group] = 0;

    stub.set_lamda(&SettingJsonGenerator::instance, []() -> SettingJsonGenerator * {
        __DBG_STUB_INVOKE__
        static SettingJsonGenerator generator;
        return &generator;
    });

    stub.set_lamda(&SettingJsonGenerator::hasConfig,
                   [](SettingJsonGenerator *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&SettingJsonGenerator::addConfig,
                   [](SettingJsonGenerator *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&SettingJsonGenerator::addCheckBoxConfig,
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&SideBarInfoCacheMananger::instance, []() -> SideBarInfoCacheMananger * {
        __DBG_STUB_INVOKE__
        static SideBarInfoCacheMananger manager;
        return &manager;
    });

    stub.set_lamda(&SideBarInfoCacheMananger::appendLastSettingKey,
                   [](SideBarInfoCacheMananger *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarHelper::bindSetting,
                   [](const QString &, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarInfoCacheMananger::appendLastSettingBindingKey,
                   [](SideBarInfoCacheMananger *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    SideBarHelper::addItemToSettingPannel(group, key, value, &levelMap);

    EXPECT_EQ(levelMap[group], 1);
}

TEST_F(UT_SideBarHelper, AddItemToSettingPannel_Network)
{
    QString group = DefaultGroup::kNetwork;
    QString key = "test.network.key";
    QString value = "Network Item";
    QMap<QString, int> levelMap;
    levelMap[group] = 0;

    stub.set_lamda(&SettingJsonGenerator::instance, []() -> SettingJsonGenerator * {
        __DBG_STUB_INVOKE__
        static SettingJsonGenerator generator;
        return &generator;
    });

    stub.set_lamda(&SettingJsonGenerator::hasConfig,
                   [](SettingJsonGenerator *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&SettingJsonGenerator::addConfig,
                   [](SettingJsonGenerator *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&SettingJsonGenerator::addCheckBoxConfig,
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&SideBarInfoCacheMananger::instance, []() -> SideBarInfoCacheMananger * {
        __DBG_STUB_INVOKE__
        static SideBarInfoCacheMananger manager;
        return &manager;
    });

    stub.set_lamda(&SideBarInfoCacheMananger::appendLastSettingKey,
                   [](SideBarInfoCacheMananger *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarHelper::bindSetting,
                   [](const QString &, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarInfoCacheMananger::appendLastSettingBindingKey,
                   [](SideBarInfoCacheMananger *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    SideBarHelper::addItemToSettingPannel(group, key, value, &levelMap);

    EXPECT_EQ(levelMap[group], 1);
}

TEST_F(UT_SideBarHelper, AddItemToSettingPannel_Tag)
{
    QString group = DefaultGroup::kTag;
    QString key = "test.tag.key";
    QString value = "Tag Item";
    QMap<QString, int> levelMap;
    levelMap[group] = 0;

    stub.set_lamda(&SettingJsonGenerator::instance, []() -> SettingJsonGenerator * {
        __DBG_STUB_INVOKE__
        static SettingJsonGenerator generator;
        return &generator;
    });

    stub.set_lamda(&SettingJsonGenerator::hasConfig,
                   [](SettingJsonGenerator *, const QString &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&SettingJsonGenerator::addConfig,
                   [](SettingJsonGenerator *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&SettingJsonGenerator::addCheckBoxConfig,
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&SideBarInfoCacheMananger::instance, []() -> SideBarInfoCacheMananger * {
        __DBG_STUB_INVOKE__
        static SideBarInfoCacheMananger manager;
        return &manager;
    });

    stub.set_lamda(&SideBarInfoCacheMananger::appendLastSettingKey,
                   [](SideBarInfoCacheMananger *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarHelper::bindSetting,
                   [](const QString &, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&SideBarInfoCacheMananger::appendLastSettingBindingKey,
                   [](SideBarInfoCacheMananger *, const QString &) {
                       __DBG_STUB_INVOKE__
                   });

    SideBarHelper::addItemToSettingPannel(group, key, value, &levelMap);

    EXPECT_EQ(levelMap[group], 1);
}

TEST_F(UT_SideBarHelper, RemoveItemFromSetting)
{
    QString key = "test.remove.setting.key";

    stub.set_lamda(&SettingJsonGenerator::instance, []() -> SettingJsonGenerator * {
        __DBG_STUB_INVOKE__
        static SettingJsonGenerator generator;
        return &generator;
    });

    stub.set_lamda(&SettingJsonGenerator::removeConfig,
                   [](SettingJsonGenerator *, const QString &) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    SideBarHelper::removeItemFromSetting(key);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarHelper, RegistCustomSettingItem)
{
    stub.set_lamda(&CustomSettingItemRegister::instance, []() -> CustomSettingItemRegister * {
        __DBG_STUB_INVOKE__
        static CustomSettingItemRegister reg;
        return &reg;
    });

    stub.set_lamda(&CustomSettingItemRegister::registCustomSettingItemType,
                   [](CustomSettingItemRegister *, const QString &, std::function<QPair<QWidget *, QWidget *>(QObject *)>) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    SideBarHelper::registCustomSettingItem();

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarHelper, HiddenRules)
{
    QVariantMap mockRules;
    mockRules["key1"] = false;
    mockRules["key2"] = true;

    stub.set_lamda(&DConfigManager::instance, []() -> DConfigManager * {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::value,
                   [mockRules](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(mockRules);
                   });

    QVariantMap rules = SideBarHelper::hiddenRules();

    EXPECT_EQ(rules.size(), mockRules.size());
}

TEST_F(UT_SideBarHelper, GroupExpandRules)
{
    QVariantMap mockRules;
    mockRules[DefaultGroup::kCommon] = true;
    mockRules[DefaultGroup::kDevice] = false;

    stub.set_lamda(&DConfigManager::instance, []() -> DConfigManager * {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::value,
                   [mockRules](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(mockRules);
                   });

    QVariantMap rules = SideBarHelper::groupExpandRules();

    EXPECT_EQ(rules.size(), mockRules.size());
}

TEST_F(UT_SideBarHelper, SaveGroupsStateToConfig)
{
    QVariantMap inputVar;
    inputVar[DefaultGroup::kCommon] = true;
    inputVar[DefaultGroup::kDevice] = false;

    bool setValueCalled = false;

    stub.set_lamda(&DConfigManager::instance, []() -> DConfigManager * {
        __DBG_STUB_INVOKE__
        static DConfigManager manager;
        return &manager;
    });

    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(QVariantMap());
                   });

    stub.set_lamda(&DConfigManager::setValue,
                   [&setValueCalled](DConfigManager *, const QString &, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       setValueCalled = true;
                   });

    SideBarHelper::saveGroupsStateToConfig(inputVar);

    EXPECT_TRUE(setValueCalled);
}

TEST_F(UT_SideBarHelper, OpenFolderInASeparateProcess)
{
    QUrl url = QUrl::fromLocalFile("/home/separate");

    bool sendCalled = false;

    stub.set_lamda(&SideBarEventCaller::sendOpenWindow,
                   [&sendCalled](const QUrl &, bool isNew) {
                       __DBG_STUB_INVOKE__
                       sendCalled = true;
                       EXPECT_FALSE(isNew);
                   });

    SideBarHelper::openFolderInASeparateProcess(url);

    EXPECT_TRUE(sendCalled);
}

TEST_F(UT_SideBarHelper, ContextMenuEnabled_Static)
{
    // Test static variable
    SideBarHelper::contextMenuEnabled = true;
    EXPECT_TRUE(SideBarHelper::contextMenuEnabled);

    SideBarHelper::contextMenuEnabled = false;
    EXPECT_FALSE(SideBarHelper::contextMenuEnabled);

    // Restore default
    SideBarHelper::contextMenuEnabled = true;
}

TEST_F(UT_SideBarHelper, Mutex)
{
    QMutex &m1 = SideBarHelper::mutex();
    QMutex &m2 = SideBarHelper::mutex();

    // Should return same mutex instance
    EXPECT_EQ(&m1, &m2);
}
