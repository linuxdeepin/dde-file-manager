// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "tag.h"
#include "utils/tagmanager.h"
#include "utils/taghelper.h"
#include "utils/filetagcache.h"
#include "widgets/tagwidget.h"
#include "widgets/private/tagwidget_p.h"
#include "data/tagproxyhandle.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>
#include "qdbusabstractinterface.h"
#include <QDBusPendingCall>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;
using namespace dpf;
using namespace QDBus;
Q_DECLARE_METATYPE(CustomViewExtensionView)

class TagTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&TagManager::getAllTags, []() {
            __DBG_STUB_INVOKE__
            return QMap<QString, QColor>();
        });
        stub.set_lamda(&QDBusAbstractInterface::asyncCallWithArgumentList, []() {
            return QDBusPendingCall::fromError(QDBusError());
        });
    }
    virtual void TearDown() override { stub.clear(); }

protected:
    stub_ext::StubExt stub;
    Tag ins;
};

TEST_F(TagTest, Start)
{
    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, CustomViewExtensionView, int &);
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, CustomViewExtensionView, QString &, int &);
    typedef QVariant (EventChannelManager::*Push3)(const QString &, const QString &, QString, QStringList &);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    auto push2 = static_cast<Push2>(&EventChannelManager::push);
    auto push3 = static_cast<Push3>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push3, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(&FileTagCacheController::initLoadTagInfos, []() {
        __DBG_STUB_INVOKE__
    });

    EXPECT_TRUE(ins.start());
}

TEST_F(TagTest, createTagWidget)
{
    auto func = static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() -> bool { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagManager::getTagsByUrls, []() {
        __DBG_STUB_INVOKE__
        return QStringList();
    });
    EXPECT_TRUE(ins.createTagWidgetForPropertyDialog(QUrl("file://hello/world")));
}

TEST_F(TagTest, initialize)
{
    bool isRegister = false;
    stub.set_lamda(&TagProxyHandle::connectToService, [&isRegister]() {
        __DBG_STUB_INVOKE__
        isRegister = true;
        return true;
    });
    ins.initialize();
    EXPECT_TRUE(isRegister);
}

TEST_F(TagTest, onWindowOpened)
{
    bool isRun = false;
    FileManagerWindow w(QUrl::fromLocalFile("/home"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&w, &isRun] {
        isRun = true;
        return &w;
    });
    stub.set_lamda(&Tag::installToSideBar, [] {});
    ins.onWindowOpened(1);
    EXPECT_TRUE(isRun);
}

TEST_F(TagTest, onAllPluginsStarted)
{
    bool isRun = false;
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QString);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [&isRun] {
        isRun = true;
        return QVariant();
    });

    ins.onAllPluginsStarted();
    EXPECT_TRUE(isRun);
}

TEST_F(TagTest, installToSideBar)
{
    bool isRun = false;
    stub.set_lamda(&TagManager::getAllTags, [&isRun]() {
        __DBG_STUB_INVOKE__
        isRun = true;
        QMap<QString, QColor> map;
        map.insert("red", QColor("red"));
        return map;
    });

    stub.set_lamda(&TagHelper::createSidebarItemInfo, []() { return QVariantMap(); });
    ins.installToSideBar();
    EXPECT_TRUE(isRun);
}

TEST_F(TagTest, onMenuSceneAdded)
{
    stub.set_lamda(&TagManager::getAllTags, []() {
        __DBG_STUB_INVOKE__
        return QMap<QString, QColor>();
    });
    ins.menuScenes.insert("TagMenu");
    ins.onMenuSceneAdded("TagMenu");
    EXPECT_TRUE(!ins.subscribedEvent);
}

TEST_F(TagTest, regTagCrumbToTitleBar)
{
    bool isCall { false };

    typedef QVariant (EventChannelManager::*FuncType)(const QString &, const QString &, QString, QVariantMap &&);
    stub.set_lamda(static_cast<FuncType>(&EventChannelManager::push), [&isCall] {
        isCall = true;
        return true;
    });

    ins.regTagCrumbToTitleBar();

    EXPECT_TRUE(isCall);
}
