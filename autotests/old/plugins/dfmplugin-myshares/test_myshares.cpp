// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include "addr_any.h"

#include "myshares.h"
#include "utils/shareutils.h"
#include "utils/sharefilehelper.h"
#include "events/shareeventhelper.h"
#include "dfmplugin_myshares_global.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QSignalSpy>
#include <QScopedPointer>
#include <QFileInfo>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_MyShares : public testing::Test
{
public:
    virtual void SetUp() override { }
    virtual void TearDown() override { }
    MyShares ins;
};

// NOTE! if you want to simplify your stub for events,
// just stub the 'QVariant EventChannel::send(const QVariantList &params)'
// but you will see a lots of error message in console.
// so if you do not want to see them, stub the `push` directly like me did.
// so does to 'follow/subscribe...'
TEST_F(UT_MyShares, Start)
{
    stub_ext::StubExt stub;

    typedef void (MyShares::*SubFunc1)(const QString &);   // type of MyShares::onShareAdded, MyShares::onShareRemoved
    typedef bool (EventDispatcherManager::*Subscribe)(const QString &, const QString &, MyShares *, SubFunc1);
    auto subscribe = static_cast<Subscribe>(&EventDispatcherManager::subscribe);
    stub.set_lamda(subscribe, [] { __DBG_STUB_INVOKE__ return true; });

    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, QString);
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, QString, QString &&);

    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    auto push2 = static_cast<Push2>(&EventChannelManager::push);
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    stub.set_lamda(&MyShares::followEvents, [] { __DBG_STUB_INVOKE__ });

    EXPECT_TRUE(ins.start());
}

static bool stubRegistScene(const QString &, DFMBASE_NAMESPACE::AbstractSceneCreator *creator)
{
    if (creator)
        delete creator;
    return true;
}

TEST_F(UT_MyShares, Initialize)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&MyShares::beMySubScene, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(ins.initialize());
}

TEST_F(UT_MyShares, Stop)
{
    EXPECT_NO_FATAL_FAILURE(ins.stop());
}

// complete protected/private functions' ut
TEST_F(UT_MyShares, OnWindowOpened)
{
    stub_ext::StubExt stub;

    DFMBASE_USE_NAMESPACE
    FileManagerWindow *win = new FileManagerWindow(QUrl());
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [win] { __DBG_STUB_INVOKE__ return win; });

    // run else block first.
    stub.set_lamda(&FileManagerWindow::sideBar, [] { __DBG_STUB_INVOKE__ return nullptr; });
    stub.set_lamda(&FileManagerWindow::titleBar, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(0));

    // run if true block
    stub.set_lamda(&FileManagerWindow::sideBar, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<AbstractFrame *>(1); });   // this lambda's  _  return  value is only used to work in 'if' block and will never and should never be used.
    stub.set_lamda(&FileManagerWindow::titleBar, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<AbstractFrame *>(1); });   // this lambda's  _  return  value is only used to work in 'if' block and will never and should never be used.
    stub.set_lamda(&MyShares::addToSidebar, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&MyShares::regMyShareToSearch, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(0));

    delete win;
}

TEST_F(UT_MyShares, OnShareAdded)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&MyShares::addToSidebar, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins.onShareAdded(""));
}

TEST_F(UT_MyShares, OnShareRemoved)
{
    typedef QVariant (EventChannelManager::*Push3)(const QString &, const QString &);
    typedef QVariant (EventChannelManager::*Push4)(const QString &, const QString &, QUrl);

    stub_ext::StubExt stub;
    auto push3 = static_cast<Push3>(&EventChannelManager::push);
    stub.set_lamda(push3, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    auto push4 = static_cast<Push4>(&EventChannelManager::push);
    stub.set_lamda(push4, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_NO_FATAL_FAILURE(ins.onShareRemoved(""));
    EXPECT_NO_FATAL_FAILURE(ins.onShareRemoved("whatever/path"));
    EXPECT_NO_FATAL_FAILURE(ins.onShareRemoved("what/a/good/day"));
}

TEST_F(UT_MyShares, AddToSideBar)
{
    stub_ext::StubExt stub;

    typedef QVariant (EventChannelManager::*Push3)(const QString &, const QString &);
    auto push3 = static_cast<Push3>(&EventChannelManager::push);
    stub.set_lamda(push3, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.addToSidebar());
    stub.clear();

    stub.set_lamda(push3, [] {
        ShareInfoList lst { ShareInfo() };
        __DBG_STUB_INVOKE__ return QVariant::fromValue<ShareInfoList>(lst);
    });

    typedef QVariant (EventChannelManager::*Push5)(const QString &, const QString &, QUrl, QVariantMap &);
    auto push5 = static_cast<Push5>(&EventChannelManager::push);
    stub.set_lamda(push5, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.addToSidebar());
}

TEST_F(UT_MyShares, RegMyShareToSearch)
{
    stub_ext::StubExt stub;

    typedef QVariant (EventChannelManager::*Push5)(const QString &, const QString &, QString, QVariantMap &);
    auto push5 = static_cast<Push5>(&EventChannelManager::push);
    stub.set_lamda(push5, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.regMyShareToSearch());
}

static bool stubContains(const QString &)
{
    return true;
}

static bool stubBind(const QString &, const QString &)
{
    return true;
}

TEST_F(UT_MyShares, BeMySubScene)
{
    stub_ext::StubExt stub;

    // test else branch first.
    typedef void (MyShares::*SubFunc1)(const QString &);   // type of MyShares::onShareAdded, MyShares::onShareRemoved
    typedef bool (EventDispatcherManager::*Subscribe)(const QString &, const QString &, MyShares *, SubFunc1);
    auto subscribe = static_cast<Subscribe>(&EventDispatcherManager::subscribe);
    stub.set_lamda(subscribe, [] { __DBG_STUB_INVOKE__ return true; });
    ins.eventSubscribed = false;
    EXPECT_NO_FATAL_FAILURE(ins.beMySubScene("hello"));

    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, QString);
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, QString, const QString &);
    auto pushContains = static_cast<Push1>(&EventChannelManager::push);
    auto pushBind = static_cast<Push2>(&EventChannelManager::push);
    stub.set_lamda(pushContains, [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(pushBind, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(ins.beMySubScene("hello"));
}

TEST_F(UT_MyShares, BeMySubOnAdded)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&MyShares::beMySubScene, [](void *, const QString &) { __DBG_STUB_INVOKE__ });

    typedef void (MyShares::*SubFunc1)(const QString &);   // type of MyShares::onShareAdded, MyShares::onShareRemoved
    typedef bool (EventDispatcherManager::*Subscribe)(const QString &, const QString &, MyShares *, SubFunc1);
    auto unsubscribe = static_cast<Subscribe>(&EventDispatcherManager::unsubscribe);
    stub.set_lamda(unsubscribe, [] { __DBG_STUB_INVOKE__ return true; });

    ins.waitToBind.clear();
    ins.waitToBind.insert("hello");
    EXPECT_NO_FATAL_FAILURE(ins.beMySubOnAdded("hello"));
}

Q_DECLARE_METATYPE(QList<QUrl> *);
TEST_F(UT_MyShares, HookEvent)
{
    stub_ext::StubExt stub;

    typedef bool (ShareEventHelper::*HookFunc1)(quint64, const QList<QUrl> &);   // type of ShareEventHelper::blockDelete/blockMoveToTrash
    typedef bool (EventSequenceManager::*HookType1)(const QString &, const QString &, ShareEventHelper *, HookFunc1);

    typedef bool (ShareEventHelper::*HookFunc2)(quint64, const QUrl &);   // type of ShareEventHelper::blockPaste, hookSendChangeCurrentUrl
    typedef bool (EventSequenceManager::*HookType2)(const QString &, const QString &, ShareEventHelper *, HookFunc2);

    typedef bool (ShareEventHelper::*HookFunc3)(const QList<QUrl> &);   // type of ShareEventHelper::hookSendOpenWindow
    typedef bool (EventSequenceManager::*HookType3)(const QString &, const QString &, ShareEventHelper *, HookFunc3);

    typedef bool (ShareUtils::*HookFunc4)(const QList<QUrl> &, QList<QUrl> *);   // type of ShareUtils::urlsToLocal
    typedef bool (EventSequenceManager::*HookType4)(const QString &, const QString &, ShareUtils *, HookFunc4);

    typedef bool (ShareFileHelper::*HookFunc5)(quint64, const QList<QUrl> &);
    typedef bool (EventSequenceManager::*HookType5)(const QString &, const QString &, ShareFileHelper *, HookFunc5);

    auto follow1 = static_cast<HookType1>(&EventSequenceManager::follow);
    stub.set_lamda(follow1, [] { __DBG_STUB_INVOKE__ return true; });

    auto follow2 = static_cast<HookType2>(&EventSequenceManager::follow);
    stub.set_lamda(follow2, [] { __DBG_STUB_INVOKE__ return true; });

    auto follow3 = static_cast<HookType3>(&EventSequenceManager::follow);
    stub.set_lamda(follow3, [] { __DBG_STUB_INVOKE__ return true; });

    auto follow4 = static_cast<HookType4>(&EventSequenceManager::follow);
    stub.set_lamda(follow4, [] { __DBG_STUB_INVOKE__ return true; });

    auto follow5 = static_cast<HookType5>(&EventSequenceManager::follow);
    stub.set_lamda(follow5, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_NO_FATAL_FAILURE(ins.followEvents());
}
