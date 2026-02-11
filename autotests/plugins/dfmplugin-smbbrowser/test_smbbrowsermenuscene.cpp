// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "menu/smbbrowsermenuscene.h"
#include "private/smbbrowsermenuscene_p.h"
#include "actioniddefines.h"
#include "events/smbbrowsereventcaller.h"
#include "utils/smbbrowserutils.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_menu_defines.h>
#include <plugins/common/dfmplugin-menu/menu_eventinterface_helper.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QAction>
#include <QVariantHash>
#include <QDir>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_smbbrowser;

class UT_SmbBrowserMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new SmbBrowserMenuScene();
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

private:
    stub_ext::StubExt stub;
    SmbBrowserMenuScene *scene { nullptr };
    SmbBrowserMenuScenePrivate *d { nullptr };
};

TEST_F(UT_SmbBrowserMenuScene, Name)
{
    EXPECT_TRUE(scene->name() == "SmbBrowserMenu");
}

TEST_F(UT_SmbBrowserMenuScene, Initialize)
{
    QList<QUrl> urls { QUrl::fromLocalFile("/hello/world"), QUrl::fromLocalFile("/i/can/eat/glass/without/hurt") };
    EXPECT_FALSE(scene->initialize({ { "isEmptyArea", true } }));
    EXPECT_FALSE(scene->initialize({ { "selectFiles", QVariant::fromValue<QList<QUrl>>(urls) } }));
    urls.takeFirst();
    EXPECT_NO_FATAL_FAILURE(scene->initialize({ { "isEmptyArea", false },
                                                { "selectFiles", QVariant::fromValue<QList<QUrl>>(urls) } }));
}

TEST_F(UT_SmbBrowserMenuScene, Create)
{
    EXPECT_FALSE(scene->create(nullptr));

    QMenu menu;
    EXPECT_NO_FATAL_FAILURE(scene->create(&menu));
}

TEST_F(UT_SmbBrowserMenuScene, UpdateState)
{
    EXPECT_NO_FATAL_FAILURE(scene->updateState(nullptr));
    QMenu menu;
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
}

TEST_F(UT_SmbBrowserMenuScene, Triggered)
{
    DFMBASE_USE_NAMESPACE

    EXPECT_FALSE(scene->triggered(nullptr));

    auto act = new QAction("hello");
    act->setProperty(ActionPropertyKey::kActionID, "hello");
    d->predicateAction.insert("hello", act);
    EXPECT_FALSE(scene->triggered(act));

    d->selectFiles.append(QUrl::fromLocalFile("/hello/world"));

    QList<bool> arg1 { false, true };
    QList<dfmmount::DeviceError> arg2 { dfmmount::DeviceError::kNoError, dfmmount::DeviceError::kGIOErrorAlreadyMounted };
    QList<QString> arg3 { "/hello/world", "" };
    stub.set_lamda(&DeviceManager::mountNetworkDeviceAsync, [&](void *, const QString &, CallbackType1 cb, int) {
        __DBG_STUB_INVOKE__
        DFMMOUNT::OperationErrorInfo errInfo { arg2.first() };
        cb(arg1.first(), errInfo, arg3.first());
    });
    stub.set_lamda(&DialogManager::showErrorDialogWhenOperateDeviceFailed, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(scene->triggered(act));

    arg1.takeFirst();
    act->setProperty(ActionPropertyKey::kActionID, "open-smb");
    d->predicateAction.insert("open-smb", act);
    typedef bool (dpf::EventDispatcherManager::*Publish1)(int, quint64, QUrl &);
    auto publish1 = static_cast<Publish1>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish1, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(scene->triggered(act));

    act->setProperty(ActionPropertyKey::kActionID, "open-smb-in-new-win");
    d->predicateAction.insert("open-smb-in-new-win", act);
    typedef bool (dpf::EventDispatcherManager::*Publish2)(int, QUrl);
    auto publish2 = static_cast<Publish2>(&dpf::EventDispatcherManager::publish);
    stub.set_lamda(publish2, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(scene->triggered(act));
    delete act;
}

TEST_F(UT_SmbBrowserMenuScene, Scene)
{
    EXPECT_EQ(nullptr, scene->scene(nullptr));

    auto act = new QAction("hello");
    d->predicateAction.insert("hello", act);
    EXPECT_STREQ(scene->scene(act)->metaObject()->className(), "dfmbase::AbstractMenuScene");

    d->predicateAction.clear();
    EXPECT_NO_FATAL_FAILURE(scene->scene(act));
    delete act;
    act = nullptr;
}

class UT_SmbBrowserMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    SmbBrowserMenuCreator creator;
};

TEST_F(UT_SmbBrowserMenuCreator, Name)
{
    EXPECT_TRUE(SmbBrowserMenuCreator::name() == "SmbBrowserMenu");
}

TEST_F(UT_SmbBrowserMenuCreator, Create)
{
    auto scene = creator.create();
    EXPECT_TRUE(scene != nullptr);
    EXPECT_STREQ("dfmbase::AbstractMenuScene", scene->metaObject()->className());
    EXPECT_NO_FATAL_FAILURE(delete scene);
}

class UT_SmbBrowserMenuScenePrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new SmbBrowserMenuScene();
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

private:
    stub_ext::StubExt stub;
    SmbBrowserMenuScene *scene { nullptr };
    SmbBrowserMenuScenePrivate *d { nullptr };
};

TEST_F(UT_SmbBrowserMenuScenePrivate, ActUnmount)
{
    d->url = QUrl("smb://1.2.3.4/hello");
    stub.set_lamda(smb_browser_utils::getDeviceIdByStdSmb, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4/hello"; });
    stub.set_lamda(&dfmbase::DeviceManager::unmountProtocolDevAsync,
                   [](void *, const QString &, const QVariantMap &, dfmbase::CallbackType2 cb) {
                       __DBG_STUB_INVOKE__
                       DFMMOUNT::OperationErrorInfo info;
                       cb(false, info);
                   });
    stub.set_lamda(&dfmbase::DialogManager::showErrorDialogWhenOperateDeviceFailed, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(d->actUnmount());
}

TEST_F(UT_SmbBrowserMenuScenePrivate, ActMount)
{
    d->url = QUrl("smb://1.2.3.4/hello");
    stub.set_lamda(&dfmbase::DeviceManager::mountNetworkDeviceAsync,
                   [](void *, const QString &, dfmbase::CallbackType1 cb, int) {
                       __DBG_STUB_INVOKE__
                       cb(false, DFMMOUNT::OperationErrorInfo(), "");
                   });
    stub.set_lamda(&dfmbase::DialogManager::showErrorDialogWhenOperateDeviceFailed, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(d->actMount());
}

TEST_F(UT_SmbBrowserMenuScenePrivate, ActProperties)
{
    d->url = QUrl("smb://1.2.3.4/hello");
    stub.set_lamda(smb_browser_utils::getDeviceIdByStdSmb, [] { __DBG_STUB_INVOKE__ return "smb://1.2.3.4/hello"; });

    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QList<QUrl>, QVariantHash &&);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(d->actProperties());
}
