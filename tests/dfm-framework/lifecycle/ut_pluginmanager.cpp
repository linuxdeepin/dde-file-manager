// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "stubext.h"
#include "testplugin.h"

#include <dfm-framework/lifecycle/pluginmanager.h>
#include <dfm-framework/lifecycle/private/pluginmanager_p.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>
#include <dfm-framework/lifecycle/private/pluginmetaobject_p.h>
#include <dfm-framework/listener/listener.h>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE

class UT_PluginManager : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_PluginManager, test_pluginIIDs)
{
    PluginManager manager;
    EXPECT_TRUE(manager.pluginIIDs().isEmpty());
    manager.addPluginIID("org.deepin.dde-file-manager");
    manager.addPluginIID("org.deepin.dde-desktop");
    manager.addPluginIID("org.deepin.dde-desktop");

    EXPECT_EQ(2, manager.pluginIIDs().size());
    EXPECT_TRUE(manager.pluginIIDs().contains("org.deepin.dde-file-manager"));
    EXPECT_TRUE(manager.pluginIIDs().contains("org.deepin.dde-desktop"));
}

TEST_F(UT_PluginManager, test_pluginPaths)
{
    PluginManager manager;
    EXPECT_TRUE(manager.pluginPaths().isEmpty());
    QStringList paths { "/a", "/b", "c" };
    manager.setPluginPaths(paths);
    EXPECT_EQ(paths, manager.pluginPaths());
}

TEST_F(UT_PluginManager, test_blackList)
{
    PluginManager manager;
    EXPECT_TRUE(manager.blackList().isEmpty());
    manager.addBlackPluginName("dfmplugin-vault");
    manager.addBlackPluginName("dfmplugin-vault");
    manager.addBlackPluginName("dfmplugin-burn");
    EXPECT_EQ(manager.blackList().size(), 2);
    QStringList ret { "dfmplugin-vault", "dfmplugin-burn" };
    EXPECT_EQ(manager.blackList(), ret);
}

TEST_F(UT_PluginManager, test_lazyLoadList)
{
    PluginManager manager;
    EXPECT_TRUE(manager.lazyLoadList().isEmpty());
    manager.addLazyLoadPluginName("dfmplugin-vault");
    manager.addLazyLoadPluginName("dfmplugin-burn");
    manager.addLazyLoadPluginName("dfmplugin-burn");
    EXPECT_EQ(manager.lazyLoadList().size(), 2);
    QStringList ret { "dfmplugin-vault", "dfmplugin-burn" };
    EXPECT_EQ(manager.lazyLoadList(), ret);
}

TEST_F(UT_PluginManager, test_pluginMetaObj)
{
    PluginManager manager;
    EXPECT_FALSE(manager.pluginMetaObj("dfm-burn"));

    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    objPtr->d->name = "dfm-burn";
    manager.d->readQueue.append(objPtr);
    auto obj { manager.pluginMetaObj("dfm-burn") };
    EXPECT_TRUE(obj);
    EXPECT_EQ("dfm-burn", obj->name());
}

TEST_F(UT_PluginManager, test_loadplugin)
{
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };

    objPtr->d->state = PluginMetaObject::State::kLoaded;
    EXPECT_TRUE(manager.loadPlugin(objPtr));

    objPtr->d->state = PluginMetaObject::State::kReading;
    EXPECT_FALSE(manager.loadPlugin(objPtr));

    // Failed: load faield
    objPtr->d->state = PluginMetaObject::State::kReaded;
    stub.set_lamda(&QPluginLoader::load,
                   []() { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(manager.loadPlugin(objPtr));

    // Failed: not instance
    stub.set_lamda(&QPluginLoader::load,
                   []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_FALSE(manager.loadPlugin(objPtr));
}

TEST_F(UT_PluginManager, test_loadplugin_success)
{
    // success
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    stub.set_lamda(&QPluginLoader::load,
                   []() { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&QPluginLoader::instance,
                   []() { __DBG_STUB_INVOKE__ return new TestPlugin; });

    objPtr->d->state = PluginMetaObject::State::kReaded;
    EXPECT_TRUE(manager.loadPlugin(objPtr));
    EXPECT_EQ(objPtr->d->state, PluginMetaObject::State::kLoaded);
}

TEST_F(UT_PluginManager, test_loadplugin_virtual)
{
    PluginManager manager;

    // loaded
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    objPtr->d->isVirtual = true;
    objPtr->d->realName = "virtual";
    objPtr->d->state = PluginMetaObject::State::kReaded;
    manager.d->loadedVirtualPlugins.append("virtual");
    EXPECT_TRUE(manager.loadPlugin(objPtr));
}

TEST_F(UT_PluginManager, test_initPlugin)
{
    PluginManager manager;
    // Initialized
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    objPtr->d->state = PluginMetaObject::State::kInitialized;
    EXPECT_TRUE(manager.initPlugin(objPtr));

    // Failed: not loaded
    objPtr->d->state = PluginMetaObject::State::kLoading;
    EXPECT_FALSE(manager.initPlugin(objPtr));

    // Failed: not instance
    objPtr->d->state = PluginMetaObject::State::kLoaded;
    EXPECT_FALSE(manager.initPlugin(objPtr));
}

TEST_F(UT_PluginManager, test_initPlugin_success)
{
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    objPtr->d->iid = "test.iid";
    objPtr->d->name = "dfmplugin-test";
    objPtr->d->state = PluginMetaObject::State::kLoaded;
    objPtr->d->plugin.reset(new TestPlugin);

    bool inited { false };
    QObject::connect(Listener::instance(), &Listener::pluginInitialized, [&inited](const QString &iid, const QString &name) {
        inited = true;
        EXPECT_EQ(iid, "test.iid");
        EXPECT_EQ(name, "dfmplugin-test");
    });

    EXPECT_TRUE(manager.initPlugin(objPtr));
    EXPECT_TRUE(inited);
}

TEST_F(UT_PluginManager, test_startPlugin)
{
    PluginManager manager;
    // started
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    objPtr->d->state = PluginMetaObject::State::kStarted;
    EXPECT_TRUE(manager.startPlugin(objPtr));

    // Failed: not inited
    objPtr->d->state = PluginMetaObject::State::kLoaded;
    EXPECT_FALSE(manager.startPlugin(objPtr));

    // Failed: not instance
    objPtr->d->state = PluginMetaObject::State::kInitialized;
    EXPECT_FALSE(manager.startPlugin(objPtr));

    // Failed: cannot start
    QSharedPointer<TestPlugin> plugin { new TestPlugin };
    plugin->startResult = false;
    objPtr->d->plugin = plugin;
    EXPECT_FALSE(manager.startPlugin(objPtr));
}

TEST_F(UT_PluginManager, test_startPlugin_success)
{
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    bool started { false };
    QObject::connect(Listener::instance(), &Listener::pluginStarted, [&started](const QString &iid, const QString &name) {
        started = true;
        EXPECT_EQ(iid, "test.iid");
        EXPECT_EQ(name, "dfmplugin-test");
    });

    objPtr->d->iid = "test.iid";
    objPtr->d->name = "dfmplugin-test";
    objPtr->d->state = PluginMetaObject::State::kInitialized;
    objPtr->d->plugin.reset(new TestPlugin);
    EXPECT_TRUE(manager.startPlugin(objPtr));
    EXPECT_TRUE(started);
}

TEST_F(UT_PluginManager, test_stopPlugin)
{
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };

    // stoped
    objPtr->d->state = PluginMetaObject::State::kStoped;
    EXPECT_TRUE(manager.stopPlugin(objPtr));

    // failed
    objPtr->d->state = PluginMetaObject::State::kInitialized;
    EXPECT_FALSE(manager.stopPlugin(objPtr));

    // failed
    objPtr->d->state = PluginMetaObject::State::kStarted;
    EXPECT_FALSE(manager.stopPlugin(objPtr));

    // failed
    objPtr->d->plugin.reset(new TestPlugin);
    stub.set_lamda(&QPluginLoader::unload,
                   []() { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(manager.stopPlugin(objPtr));
    EXPECT_EQ(objPtr->d->state, PluginMetaObject::State::kStoped);
}

TEST_F(UT_PluginManager, test_stopPlugin_success)
{
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    objPtr->d->state = PluginMetaObject::State::kStarted;
    QSharedPointer<TestPlugin> plugin { new TestPlugin };
    EXPECT_FALSE(plugin->stopped);
    objPtr->d->plugin = plugin;
    stub.set_lamda(&QPluginLoader::unload,
                   []() { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(manager.stopPlugin(objPtr));
    EXPECT_EQ(objPtr->d->state, PluginMetaObject::State::kShutdown);
    EXPECT_TRUE(plugin->stopped);
}

TEST_F(UT_PluginManager, loadPlugins)
{
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    manager.d->loadQueue.append(objPtr);

    stub.set_lamda(&PluginManagerPrivate::dependsSort, []() {
        __DBG_STUB_INVOKE__
        return;
    });
    // failed
    stub.set_lamda(&PluginManagerPrivate::doLoadPlugin, []() {
        __DBG_STUB_INVOKE__ return false;
    });
    EXPECT_FALSE(manager.loadPlugins());

    // success
    stub.set_lamda(&PluginManagerPrivate::doLoadPlugin, []() {
        __DBG_STUB_INVOKE__ return true;
    });
    EXPECT_TRUE(manager.loadPlugins());
}

TEST_F(UT_PluginManager, initPlugins)
{
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    manager.d->loadQueue.append(objPtr);
    EXPECT_FALSE(manager.d->allPluginsInitialized);
    // failed
    stub.set_lamda(&PluginManagerPrivate::doInitPlugin, []() {
        __DBG_STUB_INVOKE__ return false;
    });
    EXPECT_FALSE(manager.initPlugins());

    // success
    stub.set_lamda(&PluginManagerPrivate::doInitPlugin, []() {
        __DBG_STUB_INVOKE__ return true;
    });

    bool inited { false };
    QObject::connect(Listener::instance(), &Listener::pluginsInitialized, [&inited]() {
        inited = true;
    });

    EXPECT_TRUE(manager.initPlugins());
    EXPECT_TRUE(inited);
    EXPECT_TRUE(manager.d->allPluginsInitialized);
}

TEST_F(UT_PluginManager, startPlugins)
{
    PluginManager manager;
    PluginMetaObjectPointer objPtr { new PluginMetaObject };
    manager.d->loadQueue.append(objPtr);
    EXPECT_FALSE(manager.d->allPluginsStarted);

    // failed
    stub.set_lamda(&PluginManagerPrivate::doStartPlugin, []() {
        __DBG_STUB_INVOKE__ return false;
    });
    EXPECT_FALSE(manager.startPlugins());

    // success
    stub.set_lamda(&PluginManagerPrivate::doStartPlugin, []() {
        __DBG_STUB_INVOKE__ return true;
    });

    bool started { false };
    QObject::connect(Listener::instance(), &Listener::pluginsStarted, [&started]() {
        started = true;
    });

    EXPECT_TRUE(manager.startPlugins());
    EXPECT_TRUE(started);
    EXPECT_TRUE(manager.d->allPluginsStarted);
}
