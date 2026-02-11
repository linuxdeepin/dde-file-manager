// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "displaycontrol/menu/virtualentrymenuscene.h"
#include "displaycontrol/menu/virtualentrymenuscene_p.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/interfaces/abstractmenuscene.h>

#include <dfm-framework/event/event.h>

#include <QMenu>
#include <QAction>
#include <QUrl>
#include <QVariantHash>
#include <QList>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_VirtualEntryMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new VirtualEntryMenuScene();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

private:
    stub_ext::StubExt stub;
    VirtualEntryMenuScene *scene { nullptr };
};

TEST_F(UT_VirtualEntryMenuScene, Name)
{
    EXPECT_EQ("VirtualEntry", scene->name());
    EXPECT_NE("XXX", scene->name());
}

TEST_F(UT_VirtualEntryMenuScene, Initialize)
{
    QVariantHash params;
    EXPECT_FALSE(scene->initialize(params));

    QList<QUrl> urls { QUrl::fromLocalFile("/home") };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue<QList<QUrl>>(urls));

    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto f = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(f, [] { __DBG_STUB_INVOKE__ return QVariant::fromValue<AbstractMenuScene *>(nullptr); });
    EXPECT_FALSE(scene->initialize(params));

    QUrl u;
    u.setScheme("entry");
    u.setPath("smb://1.2.3.4/share/.ventry");
    urls = { u };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue<QList<QUrl>>(urls));
    EXPECT_TRUE(scene->initialize(params));
    EXPECT_EQ("smb://1.2.3.4/share/", scene->d->stdSmb);
    EXPECT_TRUE(scene->d->seperatedEntrySelected);

    u.setPath("smb://1.2.3.4.ventry");
    urls = { u };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue<QList<QUrl>>(urls));
    EXPECT_TRUE(scene->initialize(params));
    EXPECT_EQ("smb://1.2.3.4", scene->d->stdSmb);
    EXPECT_TRUE(scene->d->aggregatedEntrySelected);

    u.setPath("/home.protodev");
    urls = { u };
    params.insert(MenuParamKey::kSelectFiles, QVariant::fromValue<QList<QUrl>>(urls));
    typedef QString (*GetPath)(const QString &);
    auto ff = static_cast<GetPath>(protocol_display_utilities::getStandardSmbPath);
    stub.set_lamda(ff, [] { __DBG_STUB_INVOKE__ return "hello"; });
    EXPECT_FALSE(scene->initialize(params));

    stub.set_lamda(ff, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4"; });
    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_VirtualEntryMenuScene, Create)
{
    EXPECT_FALSE(scene->create(nullptr));

    QMenu menu;
    scene->d->aggregatedEntrySelected = true;
    bool hasMounted = false;
    stub.set_lamda(protocol_display_utilities::hasMountedShareOf, [&] { __DBG_STUB_INVOKE__ return hasMounted; });
    stub.set_lamda(&VirtualEntryMenuScenePrivate::insertActionBefore, [] { __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(scene->create(&menu));

    scene->d->aggregatedEntrySelected = false;
    scene->d->seperatedEntrySelected = true;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(UT_VirtualEntryMenuScene, UpdateState)
{
    EXPECT_NO_FATAL_FAILURE(scene->updateState(nullptr));

    stub.set_lamda(&VirtualEntryMenuScenePrivate::setActionVisible, [] { __DBG_STUB_INVOKE__ });
    QMenu menu;
    scene->d->aggregatedEntrySelected = true;
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));

    scene->d->aggregatedEntrySelected = false;
    scene->d->seperatedEntrySelected = true;
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
}

TEST_F(UT_VirtualEntryMenuScene, Triggered)
{
    EXPECT_NO_FATAL_FAILURE(scene->triggered(nullptr));

    stub.set_lamda(&VirtualEntryMenuScenePrivate::actUnmountAggregatedItem, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&VirtualEntryMenuScenePrivate::actForgetAggregatedItem, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&VirtualEntryMenuScenePrivate::actRemoveVirtualEntry, [] { __DBG_STUB_INVOKE__ });

    QAction act;
    act.setProperty("actionID", "aggregated-unmount");
    EXPECT_NO_FATAL_FAILURE(scene->triggered(&act));

    act.setProperty("actionID", "aggregated-forget");
    EXPECT_NO_FATAL_FAILURE(scene->triggered(&act));

    act.setProperty("actionID", "virtual-entry-remove");
    EXPECT_NO_FATAL_FAILURE(scene->triggered(&act));
}

TEST_F(UT_VirtualEntryMenuScene, Scene)
{
    EXPECT_NO_FATAL_FAILURE(scene->scene(nullptr));
    EXPECT_EQ(nullptr, scene->scene(nullptr));

    QAction act;
    scene->d->predicateAction.insert("test", &act);
    EXPECT_EQ(scene, scene->scene(&act));

    QAction act2;
    EXPECT_NO_FATAL_FAILURE(scene->scene(&act2));
}

class UT_VirtualEntryMenuScenePrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new VirtualEntryMenuScene();
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

private:
    stub_ext::StubExt stub;
    VirtualEntryMenuScene *scene { nullptr };
    VirtualEntryMenuScenePrivate *d { nullptr };
};

TEST_F(UT_VirtualEntryMenuScenePrivate, SetActionVisiable)
{
    EXPECT_NO_FATAL_FAILURE(d->setActionVisible(QStringList(), nullptr));

    QMenu m;
    m.addAction("hello");
    EXPECT_NO_FATAL_FAILURE(d->setActionVisible(QStringList(), &m));
}

TEST_F(UT_VirtualEntryMenuScenePrivate, InsertActionBefore)
{
    QMenu m;
    auto act = m.addAction("hello");
    act->setProperty(ActionPropertyKey::kActionID, "hello");

    EXPECT_NO_FATAL_FAILURE(d->insertActionBefore("test", "hello", &m));
    EXPECT_TRUE(m.actions().count() == 2);

    EXPECT_NO_FATAL_FAILURE(d->insertActionBefore("test", "xxx", &m));
    EXPECT_TRUE(m.actions().count() == 3);
}

TEST_F(UT_VirtualEntryMenuScenePrivate, HookCptActions)
{
    stub.set_lamda(&VirtualEntryMenuScenePrivate::actCptForget, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&VirtualEntryMenuScenePrivate::actCptMount, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(d->hookCptActions(nullptr));

    QAction act;
    act.setProperty(ActionPropertyKey::kActionID, "computer-logout-and-forget-passwd");
    EXPECT_NO_FATAL_FAILURE(d->hookCptActions(&act));
    act.setProperty(ActionPropertyKey::kActionID, "computer-mount");
    EXPECT_NO_FATAL_FAILURE(d->hookCptActions(&act));
}

TEST_F(UT_VirtualEntryMenuScenePrivate, ActUnmountAggregatedItem)
{
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4" }; });
    typedef QString (*GetPath)(const QString &);
    auto f = static_cast<GetPath>(protocol_display_utilities::getStandardSmbPath);
    stub.set_lamda(f, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4"; });
    stub.set_lamda(&DialogManager::showErrorDialogWhenOperateDeviceFailed, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&VirtualEntryMenuScenePrivate::tryRemoveAggregatedEntry, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&VirtualEntryMenuScenePrivate::gotoDefaultPageOnUnmount, [] { __DBG_STUB_INVOKE__ });

    bool unmountResult = false;
    stub.set_lamda(&DeviceManager::unmountProtocolDevAsync, [&](void *, const QString &, const QVariantMap &, CallbackType2 cb) {
        __DBG_STUB_INVOKE__
        if (cb) cb(unmountResult, DFMMOUNT::OperationErrorInfo());
    });

    EXPECT_NO_FATAL_FAILURE(d->actUnmountAggregatedItem(false));
    unmountResult = true;
    EXPECT_NO_FATAL_FAILURE(d->actUnmountAggregatedItem(true));
}

TEST_F(UT_VirtualEntryMenuScenePrivate, ActForgetAggregatedItem)
{
    stub.set_lamda(computer_sidebar_event_calls::callForgetPasswd, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&VirtualEntryMenuScenePrivate::actUnmountAggregatedItem, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(d->actForgetAggregatedItem());
}

TEST_F(UT_VirtualEntryMenuScenePrivate, ActMountSeperatedItem)
{
    bool unmountResult = false;
    stub.set_lamda(&DeviceManager::mountNetworkDeviceAsync, [&](void *, const QString &, CallbackType1 cb, int) {
        __DBG_STUB_INVOKE__
        if (cb) cb(unmountResult, DFMMOUNT::OperationErrorInfo(), "");
    });
    stub.set_lamda(&DialogManager::showErrorDialogWhenOperateDeviceFailed, [] { __DBG_STUB_INVOKE__ });

    d->stdSmb = "smb://1.2.3.4/share///";
    EXPECT_NO_FATAL_FAILURE(d->actMountSeperatedItem());
}
TEST_F(UT_VirtualEntryMenuScenePrivate, ActRemoveVirtualEntry) { }

TEST_F(UT_VirtualEntryMenuScenePrivate, ActCptMount)
{
    stub.set_lamda(&VirtualEntryMenuScenePrivate::actMountSeperatedItem, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(d->actCptMount());
}

TEST_F(UT_VirtualEntryMenuScenePrivate, ActCptForget)
{
    stub.set_lamda(&VirtualEntryDbHandler::removeData, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(d->actCptForget());
}

TEST_F(UT_VirtualEntryMenuScenePrivate, GotoDefaultPageOnUnmount)
{
    stub.set_lamda(&Application::appAttribute, [] { __DBG_STUB_INVOKE__ return QUrl::fromLocalFile("/home"); });
    stub.set_lamda(&FileManagerWindowsManager::windowIdList, [] { __DBG_STUB_INVOKE__ return QList<quint64> { 1 }; });
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<FileManagerWindow *>(1); });
    stub.set_lamda(&FileManagerWindow::currentUrl, [] { __DBG_STUB_INVOKE__ return QUrl::fromLocalFile("/home"); });
    typedef bool (dpf::EventDispatcherManager::*Publish)(dpf::EventType, quint64, QUrl &);
    auto f = static_cast<Publish>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(f, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(d->gotoDefaultPageOnUnmount());
    d->stdSmb = "file:///home";
    EXPECT_NO_FATAL_FAILURE(d->gotoDefaultPageOnUnmount());
}

TEST_F(UT_VirtualEntryMenuScenePrivate, TryRemoveAggregatedEntry)
{
    stub.set_lamda(&VirtualEntryDbHandler::removeData, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(protocol_display_utilities::getMountedSmb, [] { __DBG_STUB_INVOKE__ return QStringList { "smb://1.2.3.4" }; });
    stub.set_lamda(protocol_display_utilities::makeVEntryUrl, [] { __DBG_STUB_INVOKE__ return QUrl(); });
    stub.set_lamda(computer_sidebar_event_calls::callItemRemove, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(VirtualEntryMenuScenePrivate::tryRemoveAggregatedEntry("smb://1.2.3.4", ""));
    EXPECT_NO_FATAL_FAILURE(VirtualEntryMenuScenePrivate::tryRemoveAggregatedEntry("smb://3.4.5.6", ""));
}
