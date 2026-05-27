// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "recent.h"
#include "utils/recentmanager.h"
#include "utils/recentfilehelper.h"
#include "events/recenteventreceiver.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;
using namespace dpf;

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QVariantMap> *)
Q_DECLARE_METATYPE(QFlags<QFileDevice::Permission>)

class RecentTest : public testing::Test
{

protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    Recent ins;
};

TEST_F(RecentTest, Initialize)
{
    stub.set_lamda(&RecentManager::init, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&Recent::bindWindows, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&Recent::followEvents, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins.initialize());
}

TEST_F(RecentTest, Start)
{
    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, QString);
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, QString, QString &);
    typedef QVariant (EventChannelManager::*Push3)(const QString &, const QString &, QString, QStringList &);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    auto push2 = static_cast<Push2>(&EventChannelManager::push);
    auto push3 = static_cast<Push3>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push3, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    stub.set_lamda(&RecentManager::init, []() {});
    // type of RecentFileHelper::setPermissionHandle
    typedef bool (RecentFileHelper::*HookFunc1)(const quint64, const QUrl, QFileDevice::Permissions, bool *, QString *);
    typedef bool (EventSequenceManager::*HookType1)(const QString &, const QString &, RecentFileHelper *, HookFunc1);

    auto follow1 = static_cast<HookType1>(&EventSequenceManager::follow);
    stub.set_lamda(follow1, [] { return true; });

    EXPECT_TRUE(ins.start());
}

TEST_F(RecentTest, addRecentItem)
{
    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, const QVariantMap &);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, const QUrl &, const QVariantMap &);
    auto push2 = static_cast<Push2>(&EventChannelManager::push);
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.updateRecentItemToSideBar());
}

TEST_F(RecentTest, onWindowOpened)
{
    DFMBASE_USE_NAMESPACE
    FileManagerWindow *win = new FileManagerWindow(QUrl::fromLocalFile("/hello/world"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [win] { __DBG_STUB_INVOKE__ return win; });
    stub.set_lamda(&FileManagerWindow::sideBar, [] { __DBG_STUB_INVOKE__ return reinterpret_cast<AbstractFrame *>(1); });

    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, const QString &, const QVariantMap &);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins.regRecentCrumbToTitleBar());

    stub.set_lamda(&Recent::updateRecentItemToSideBar, [] {});
    stub.set_lamda(&Application::genericAttribute, []() -> bool {
        return true;
    });

    stub.set_lamda(&Recent::regRecentCrumbToTitleBar, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(1));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(2));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(3));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(-10086));

    stub.set_lamda(&FileManagerWindow::sideBar, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(1));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(2));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(3));
    EXPECT_NO_FATAL_FAILURE(ins.onWindowOpened(-10086));
    delete win;
}

TEST_F(RecentTest, FollowEvents)
{
    stub_ext::StubExt st;
    st.set_lamda(&RecentManager::init, []() {});
    // type of RecentManager::customColumnRole
    typedef bool (RecentManager::*HookFunc1)(const QUrl &, QList<Global::ItemRoles> *);
    typedef bool (EventSequenceManager::*HookType1)(const QString &, const QString &, RecentManager *, HookFunc1);

    // type of RecentManager::customRoleDisplayName
    typedef bool (RecentManager::*HookFunc2)(const QUrl &, const Global::ItemRoles, QString *);
    typedef bool (EventSequenceManager::*HookType2)(const QString &, const QString &, RecentManager *, HookFunc2);

    // type of RecentManager::isTransparent
    typedef bool (RecentManager::*HookFunc3)(const QUrl &, DFMGLOBAL_NAMESPACE::TransparentStatus *);
    typedef bool (EventSequenceManager::*HookType3)(const QString &, const QString &, RecentManager *, HookFunc3);

    // type of RecentManager::checkDragDropAction
    typedef bool (RecentManager::*HookFunc4)(const QList<QUrl> &, const QUrl &, Qt::DropAction *);
    typedef bool (EventSequenceManager::*HookType4)(const QString &, const QString &, RecentManager *, HookFunc4);

    // type of RecentManager::handleDropFiles
    typedef bool (RecentManager::*HookFunc5)(const QList<QUrl> &, const QUrl &);
    typedef bool (EventSequenceManager::*HookType5)(const QString &, const QString &, RecentManager *, HookFunc5);

    // type of RecentManager::detailViewIcon
    typedef bool (RecentManager::*HookFunc6)(const QUrl &, QString *);
    typedef bool (EventSequenceManager::*HookType6)(const QString &, const QString &, RecentManager *, HookFunc6);

    // type of RecentManager::sepateTitlebarCrumb
    typedef bool (RecentManager::*HookFunc7)(const QUrl &, QList<QVariantMap> *);
    typedef bool (EventSequenceManager::*HookType7)(const QString &, const QString &, RecentManager *, HookFunc7);

    // type of RecentFileHelper::cutFile
    typedef bool (RecentFileHelper::*HookFunc8)(const quint64, const QList<QUrl>,
                                                const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags);
    typedef bool (EventSequenceManager::*HookType8)(const QString &, const QString &, RecentFileHelper *, HookFunc8);

    // type of RecentFileHelper::copyFile
    typedef bool (RecentFileHelper::*HookFunc9)(const quint64, const QList<QUrl>,
                                                const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags);
    typedef bool (EventSequenceManager::*HookType9)(const QString &, const QString &, RecentFileHelper *, HookFunc9);

    // type of RecentFileHelper::moveToTrash
    typedef bool (RecentFileHelper::*HookFunc10)(const quint64, const QList<QUrl>,
                                                 const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags);
    typedef bool (EventSequenceManager::*HookType10)(const QString &, const QString &, RecentFileHelper *, HookFunc10);

    // type of RecentFileHelper::moveToTrash
    typedef bool (EventSequenceManager::*HookType11)(const QString &, const QString &, RecentFileHelper *, HookFunc10);

    // type of RecentFileHelper::openFileInPlugin
    typedef bool (RecentFileHelper::*HookFunc12)(quint64, QList<QUrl>);
    typedef bool (EventSequenceManager::*HookType12)(const QString &, const QString &, RecentFileHelper *, HookFunc12);

    // type of RecentFileHelper::linkFile
    typedef bool (RecentFileHelper::*HookFunc13)(const quint64, const QUrl, const QUrl, const bool, const bool);
    typedef bool (EventSequenceManager::*HookType13)(const QString &, const QString &, RecentFileHelper *, HookFunc13);

    // type of RecentFileHelper::writeUrlsToClipboard
    typedef bool (RecentFileHelper::*HookFunc14)(const quint64, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction,
                                                 const QList<QUrl>);
    typedef bool (EventSequenceManager::*HookType14)(const QString &, const QString &, RecentFileHelper *, HookFunc14);

    // type of RecentFileHelper::openFileInTerminal
    typedef bool (RecentFileHelper::*HookFunc15)(const quint64, const QList<QUrl>);
    typedef bool (EventSequenceManager::*HookType15)(const QString &, const QString &, RecentFileHelper *, HookFunc15);

    auto follow1 = static_cast<HookType1>(&EventSequenceManager::follow);
    st.set_lamda(follow1, [] { return true; });
    auto follow2 = static_cast<HookType2>(&EventSequenceManager::follow);
    st.set_lamda(follow2, [] { return true; });
    auto follow3 = static_cast<HookType3>(&EventSequenceManager::follow);
    st.set_lamda(follow3, [] { return true; });
    auto follow4 = static_cast<HookType4>(&EventSequenceManager::follow);
    st.set_lamda(follow4, [] { return true; });
    auto follow5 = static_cast<HookType5>(&EventSequenceManager::follow);
    st.set_lamda(follow5, [] { return true; });
    auto follow6 = static_cast<HookType6>(&EventSequenceManager::follow);
    st.set_lamda(follow6, [] { return true; });
    auto follow7 = static_cast<HookType7>(&EventSequenceManager::follow);
    st.set_lamda(follow7, [] { return true; });
    auto follow8 = static_cast<HookType8>(&EventSequenceManager::follow);
    st.set_lamda(follow8, [] { return true; });
    auto follow9 = static_cast<HookType9>(&EventSequenceManager::follow);
    st.set_lamda(follow9, [] { return true; });
    auto follow10 = static_cast<HookType10>(&EventSequenceManager::follow);
    st.set_lamda(follow10, [] { return true; });
    auto follow11 = static_cast<HookType11>(&EventSequenceManager::follow);
    st.set_lamda(follow11, [] { return true; });
    auto follow12 = static_cast<HookType12>(&EventSequenceManager::follow);
    st.set_lamda(follow12, [] { return true; });
    auto follow13 = static_cast<HookType13>(&EventSequenceManager::follow);
    st.set_lamda(follow13, [] { return true; });
    auto follow14 = static_cast<HookType14>(&EventSequenceManager::follow);
    st.set_lamda(follow14, [] { return true; });
    auto follow15 = static_cast<HookType15>(&EventSequenceManager::follow);
    st.set_lamda(follow15, [] { return true; });

    EXPECT_NO_FATAL_FAILURE(ins.followEvents());
}

TEST_F(RecentTest, BindWindows)
{
    stub.set_lamda(&Recent::onWindowOpened, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&FileManagerWindowsManager::windowIdList, [] { __DBG_STUB_INVOKE__ return QList<quint64> { 1, 2, 3 }; });
    EXPECT_NO_FATAL_FAILURE(ins.bindWindows());
}
