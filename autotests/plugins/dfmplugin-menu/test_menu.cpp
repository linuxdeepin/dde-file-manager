// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menu.h"
#include "utils/menuhelper.h"
#include "menuscene/menuutils.h"
#include "oemmenuscene/extensionmonitor.h"

#include <dfm-base/interfaces/abstractscenecreator.h>
#include <dfm-base/interfaces/abstractmenuscene.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QThread>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class TestSceneCreator : public AbstractSceneCreator
{
public:
    explicit TestSceneCreator() = default;
    QString name() const  { return "TestScene"; }
    AbstractMenuScene *create() override { return nullptr; }
};

class UT_MenuHandle : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        handle = new MenuHandle();
    }

    virtual void TearDown() override
    {
        delete handle;
        handle = nullptr;
        stub.clear();
    }

protected:
    MenuHandle *handle { nullptr };
    stub_ext::StubExt stub;
};

class UT_Menu : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        plugin = new Menu();
    }

    virtual void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

protected:
    Menu *plugin { nullptr };
    stub_ext::StubExt stub;
};

// MenuHandle 测试用例

TEST_F(UT_MenuHandle, Contains_EmptyName_ReturnsFalse)
{
    EXPECT_FALSE(handle->contains("NonExistent"));
}

TEST_F(UT_MenuHandle, RegisterScene_ValidScene_ReturnsTrue)
{
    bool signalEmitted = false;
    stub.set_lamda(&MenuHandle::publishSceneAdded, [&signalEmitted](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
        signalEmitted = true;
    });

    auto creator = new TestSceneCreator();
    EXPECT_TRUE(handle->registerScene("TestScene", creator));
    EXPECT_TRUE(handle->contains("TestScene"));
    EXPECT_TRUE(signalEmitted);
}

TEST_F(UT_MenuHandle, RegisterScene_NullCreator_ReturnsFalse)
{
    EXPECT_FALSE(handle->registerScene("TestScene", nullptr));
    EXPECT_FALSE(handle->contains("TestScene"));
}

TEST_F(UT_MenuHandle, RegisterScene_EmptyName_ReturnsFalse)
{
    auto creator = new TestSceneCreator();
    EXPECT_FALSE(handle->registerScene("", creator));
    delete creator;
}

TEST_F(UT_MenuHandle, RegisterScene_DuplicateName_ReturnsFalse)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    auto creator1 = new TestSceneCreator();
    auto creator2 = new TestSceneCreator();

    EXPECT_TRUE(handle->registerScene("TestScene", creator1));
    EXPECT_FALSE(handle->registerScene("TestScene", creator2));

    delete creator2;
}

TEST_F(UT_MenuHandle, UnregisterScene_ExistingScene_ReturnsCreator)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&MenuHandle::publishSceneRemoved, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    auto creator = new TestSceneCreator();
    handle->registerScene("TestScene", creator);

    auto result = handle->unregisterScene("TestScene");
    EXPECT_EQ(result, creator);
    EXPECT_FALSE(handle->contains("TestScene"));

    delete result;
}

TEST_F(UT_MenuHandle, UnregisterScene_NonExistentScene_ReturnsNull)
{
    auto result = handle->unregisterScene("NonExistent");
    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_MenuHandle, Bind_BothScenesExist_ReturnsTrue)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(VADDR(AbstractSceneCreator, addChild), [](AbstractSceneCreator *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    auto parentCreator = new TestSceneCreator();
    auto childCreator = new TestSceneCreator();

    handle->registerScene("ParentScene", parentCreator);
    handle->registerScene("ChildScene", childCreator);

    EXPECT_TRUE(handle->bind("ChildScene", "ParentScene"));
}

TEST_F(UT_MenuHandle, Bind_ChildNotExist_ReturnsFalse)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    auto parentCreator = new TestSceneCreator();
    handle->registerScene("ParentScene", parentCreator);

    EXPECT_FALSE(handle->bind("NonExistent", "ParentScene"));
}

TEST_F(UT_MenuHandle, Bind_ParentNotExist_ReturnsFalse)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    auto childCreator = new TestSceneCreator();
    handle->registerScene("ChildScene", childCreator);

    EXPECT_FALSE(handle->bind("ChildScene", "NonExistent"));
}

TEST_F(UT_MenuHandle, Unbind_EmptyName_DoesNothing)
{
    EXPECT_NO_FATAL_FAILURE(handle->unbind("", "ParentScene"));
}

TEST_F(UT_MenuHandle, Unbind_WithParent_RemovesFromParent)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    bool removeChildCalled = false;
    stub.set_lamda(VADDR(AbstractSceneCreator, removeChild), [&removeChildCalled](AbstractSceneCreator *, const QString &) {
        __DBG_STUB_INVOKE__
        removeChildCalled = true;
    });

    auto parentCreator = new TestSceneCreator();
    handle->registerScene("ParentScene", parentCreator);

    EXPECT_NO_FATAL_FAILURE(handle->unbind("ChildScene", "ParentScene"));
    EXPECT_TRUE(removeChildCalled);
}

TEST_F(UT_MenuHandle, Unbind_WithoutParent_RemovesFromAll)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    int removeChildCallCount = 0;
    stub.set_lamda(VADDR(AbstractSceneCreator, removeChild), [&removeChildCallCount](AbstractSceneCreator *, const QString &) {
        __DBG_STUB_INVOKE__
        removeChildCallCount++;
    });

    auto creator1 = new TestSceneCreator();
    auto creator2 = new TestSceneCreator();
    handle->registerScene("Scene1", creator1);
    handle->registerScene("Scene2", creator2);

    EXPECT_NO_FATAL_FAILURE(handle->unbind("ChildScene", ""));
    EXPECT_EQ(removeChildCallCount, 2);
}

TEST_F(UT_MenuHandle, CreateScene_NonExistentScene_ReturnsNull)
{
    auto scene = handle->createScene("NonExistent");
    EXPECT_EQ(scene, nullptr);
}

TEST_F(UT_MenuHandle, CreateScene_CreatorReturnsNull_ReturnsNull)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(VADDR(TestSceneCreator, create), [](AbstractSceneCreator *) -> AbstractMenuScene * {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    auto creator = new TestSceneCreator();
    handle->registerScene("TestScene", creator);

    auto scene = handle->createScene("TestScene");
    EXPECT_EQ(scene, nullptr);
}

TEST_F(UT_MenuHandle, CreateScene_ValidScene_CreatesSceneAndSubscenes)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    bool createCalled = false;
    bool createSubsceneCalled = false;

    stub.set_lamda(VADDR(TestSceneCreator, create), [&createCalled](AbstractSceneCreator *) -> AbstractMenuScene * {
        __DBG_STUB_INVOKE__
        createCalled = true;
        return reinterpret_cast<AbstractMenuScene *>(0x1);  // 返回非空指针
    });

    stub.set_lamda(&MenuHandle::createSubscene, [&createSubsceneCalled](MenuHandle *, AbstractSceneCreator *, AbstractMenuScene *) {
        __DBG_STUB_INVOKE__
        createSubsceneCalled = true;
    });

    auto creator = new TestSceneCreator();
    handle->registerScene("TestScene", creator);

    auto scene = handle->createScene("TestScene");
    EXPECT_NE(scene, nullptr);
    EXPECT_TRUE(createCalled);
    EXPECT_TRUE(createSubsceneCalled);
}

TEST_F(UT_MenuHandle, PerfectMenuParams_CallsMenuUtils)
{
    QVariantHash params;
    params["key"] = "value";

    bool menuUtilsCalled = false;
    stub.set_lamda(ADDR(MenuUtils, perfectMenuParams), [&menuUtilsCalled](const QVariantHash &) -> QVariantHash {
        __DBG_STUB_INVOKE__
        menuUtilsCalled = true;
        QVariantHash result;
        result["modified"] = true;
        return result;
    });

    auto result = handle->perfectMenuParams(params);
    EXPECT_TRUE(menuUtilsCalled);
}

TEST_F(UT_MenuHandle, IsMenuDisable_WithApplicationName_CallsHelper)
{
    QVariantHash params;
    params["ApplicationName"] = "testapp";

    bool helperCalled = false;
    stub.set_lamda(ADDR(Helper, isHiddenMenu), [&helperCalled](const QString &app) -> bool {
        __DBG_STUB_INVOKE__
        helperCalled = true;
        return app == "testapp";
    });

    bool result = handle->isMenuDisable(params);
    EXPECT_TRUE(helperCalled);
}

TEST_F(UT_MenuHandle, IsMenuDisable_WithoutApplicationName_UsesQAppName)
{
    QVariantHash params;

    bool helperCalled = false;
    QString capturedAppName;
    stub.set_lamda(ADDR(Helper, isHiddenMenu), [&helperCalled, &capturedAppName](const QString &app) -> bool {
        __DBG_STUB_INVOKE__
        helperCalled = true;
        capturedAppName = app;
        return false;
    });

    handle->isMenuDisable(params);
    EXPECT_TRUE(helperCalled);
    EXPECT_EQ(capturedAppName, qApp->applicationName());
}

TEST_F(UT_MenuHandle, Init_RegistersAllScenes)
{
    stub.set_lamda(&MenuHandle::publishSceneAdded, [](MenuHandle *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    EXPECT_TRUE(handle->init());
}

// Menu Plugin 测试用例

TEST_F(UT_Menu, Initialize_CreatesHandle)
{
    stub.set_lamda(&MenuHandle::init, [](MenuHandle *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_NO_FATAL_FAILURE(plugin->initialize());
}

TEST_F(UT_Menu, Start_DesktopApp_StartsExtensionMonitor)
{
    stub.set_lamda(&QApplication::applicationName, []() {
        __DBG_STUB_INVOKE__
        return QString("dde-desktop");
    });

    bool monitorStarted = false;
    stub.set_lamda(ADDR(ExtensionMonitor, start), [&monitorStarted](ExtensionMonitor *) {
        __DBG_STUB_INVOKE__
        monitorStarted = true;
    });

    EXPECT_TRUE(plugin->start());
    EXPECT_TRUE(monitorStarted);
}

TEST_F(UT_Menu, Start_ShellApp_StartsExtensionMonitor)
{
    stub.set_lamda(&QApplication::applicationName, []() {
        __DBG_STUB_INVOKE__
        return QString("org.deepin.dde-shell");
    });

    bool monitorStarted = false;
    stub.set_lamda(ADDR(ExtensionMonitor, start), [&monitorStarted](ExtensionMonitor *) {
        __DBG_STUB_INVOKE__
        monitorStarted = true;
    });

    EXPECT_TRUE(plugin->start());
    EXPECT_TRUE(monitorStarted);
}

TEST_F(UT_Menu, Start_OtherApp_DoesNotStartMonitor)
{
    stub.set_lamda(&QApplication::applicationName, []() {
        __DBG_STUB_INVOKE__
        return QString("dde-file-manager");
    });

    bool monitorStarted = false;
    stub.set_lamda(ADDR(ExtensionMonitor, start), [&monitorStarted](ExtensionMonitor *) {
        __DBG_STUB_INVOKE__
        monitorStarted = true;
    });

    EXPECT_TRUE(plugin->start());
    EXPECT_FALSE(monitorStarted);
}

TEST_F(UT_Menu, Stop_DeletesHandle)
{
    stub.set_lamda(&MenuHandle::init, [](MenuHandle *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    plugin->initialize();
    EXPECT_NO_FATAL_FAILURE(plugin->stop());
}
