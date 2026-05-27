// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfm-base/base/application/application.h"
#include "search.h"
#include "stubext.h"
#include "utils/searchhelper.h"
#include "events/searcheventreceiver.h"
#include "utils/custommanager.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <gtest/gtest.h>

Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AbstractSceneCreator *)

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DPF_USE_NAMESPACE

TEST(SearchTest, ut_initialize)
{
    stub_ext::StubExt st;
    st.set_lamda(&Search::bindEvents, []() { return; });

    Search search;
    search.initialize();

    EXPECT_TRUE(UrlRoute::hasScheme("search"));
}

TEST(SearchTest, ut_start)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString, AbstractSceneCreator *&);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [](EventChannelManager *&, const QString &, const QString &, QString, AbstractSceneCreator *&creator) {
        delete creator;
        return QVariant();
    });

    st.set_lamda(&DConfigManager::addConfig, [] { return true; });
    st.set_lamda(&DConfigManager::value, [] { return true; });
    st.set_lamda(&DConfigManager::setValue, [] {});
    st.set_lamda(&SettingJsonGenerator::addGroup, [] { return true; });
    st.set_lamda(&SettingJsonGenerator::addConfig, [] { return true; });

    Application app;

    Search search;
    EXPECT_TRUE(search.start());
}

TEST(SearchTest, ut_stop)
{
    Search search;
    EXPECT_NO_FATAL_FAILURE(search.stop());
}

TEST(SearchTest, ut_onWindowOpened_1)
{
    stub_ext::StubExt st;
    FileManagerWindow w(QUrl::fromLocalFile("/home"));
    st.set_lamda(&FileManagerWindowsManager::findWindowById, [&w] { return &w; });
    st.set_lamda(&FileManagerWindow::workSpace, []() { return nullptr; });
    st.set_lamda(&FileManagerWindow::titleBar, []() { return nullptr; });
    st.set_lamda(&FileManagerWindow::detailView, []() { return nullptr; });

    Search search;
    EXPECT_NO_FATAL_FAILURE(search.onWindowOpened(0));
}

TEST(SearchTest, ut_onWindowOpened_2)
{
    stub_ext::StubExt st;
    FileManagerWindow w(QUrl::fromLocalFile("/home"));
    st.set_lamda(&FileManagerWindowsManager::findWindowById, [&w] { return &w; });
    st.set_lamda(&FileManagerWindow::workSpace, []() { return reinterpret_cast<AbstractFrame *>(1); });
    st.set_lamda(&FileManagerWindow::titleBar, []() { return reinterpret_cast<AbstractFrame *>(1); });
    st.set_lamda(&FileManagerWindow::detailView, []() { return reinterpret_cast<AbstractFrame *>(1); });

    st.set_lamda(&Search::regSearchToWorkspace, []() { return; });
    st.set_lamda(&Search::regSearchCrumbToTitleBar, []() { return; });

    Search search;
    EXPECT_NO_FATAL_FAILURE(search.onWindowOpened(0));
}

TEST(SearchTest, ut_regSearchCrumbToTitleBar)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, QString, QVariantMap &&);

    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });

    Search search;
    EXPECT_NO_FATAL_FAILURE(search.regSearchCrumbToTitleBar());
}

TEST(SearchTest, ut_regSearchToWorkspace)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, QString);
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, QString, QString &&);
    typedef QVariant (EventChannelManager::*Push3)(const QString &, const QString &, QString, Global::ViewMode &&);
    typedef QVariant (EventChannelManager::*Push4)(const QString &, const QString &, QVariantMap);

    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    st.set_lamda(push1, [] { return QVariant(); });

    auto push2 = static_cast<Push2>(&EventChannelManager::push);
    st.set_lamda(push2, [] { return QVariant(); });

    auto push3 = static_cast<Push3>(&EventChannelManager::push);
    st.set_lamda(push3, [] { return QVariant(); });

    auto push4 = static_cast<Push4>(&EventChannelManager::push);
    st.set_lamda(push4, [] { return QVariant(); });

    Search search;
    EXPECT_NO_FATAL_FAILURE(search.regSearchToWorkspace());
}

TEST(SearchTest, ut_bindEvents)
{
    stub_ext::StubExt st;
    // hook
    // type of SearchHelper::customColumnRole
    typedef bool (SearchHelper::*HookFunc1)(const QUrl &, QList<Global::ItemRoles> *);
    typedef bool (EventSequenceManager::*HookType1)(const QString &, const QString &, SearchHelper *, HookFunc1);

    // type of SearchHelper::customRoleDisplayName
    typedef bool (SearchHelper::*HookFunc2)(const QUrl &, const Global::ItemRoles, QString *);
    typedef bool (EventSequenceManager::*HookType2)(const QString &, const QString &, SearchHelper *, HookFunc2);

    // type of SearchHelper::customRoleData
    typedef bool (SearchHelper::*HookFunc3)(const QUrl &, const QUrl &, const Global::ItemRoles, QVariant *);
    typedef bool (EventSequenceManager::*HookType3)(const QString &, const QString &, SearchHelper *, HookFunc3);

    // type of SearchHelper::blockPaste
    typedef bool (SearchHelper::*HookFunc4)(quint64, const QUrl &);
    typedef bool (EventSequenceManager::*HookType4)(const QString &, const QString &, SearchHelper *, HookFunc4);

    auto follow1 = static_cast<HookType1>(&EventSequenceManager::follow);
    st.set_lamda(follow1, [] { return true; });
    auto follow2 = static_cast<HookType2>(&EventSequenceManager::follow);
    st.set_lamda(follow2, [] { return true; });
    auto follow3 = static_cast<HookType3>(&EventSequenceManager::follow);
    st.set_lamda(follow3, [] { return true; });
    auto follow4 = static_cast<HookType4>(&EventSequenceManager::follow);
    st.set_lamda(follow4, [] { return true; });

    // subscribe
    // type of SearchEventReceiver::handleSearch
    typedef void (SearchEventReceiver::*SubFunc1)(quint64, const QString &);
    typedef bool (EventDispatcherManager::*SubType1)(const QString &, const QString &, SearchEventReceiver *, SubFunc1);

    // type of SearchEventReceiver::handleStopSearch
    typedef void (SearchEventReceiver::*SubFunc2)(quint64);
    typedef bool (EventDispatcherManager::*SubType2)(const QString &, const QString &, SearchEventReceiver *, SubFunc2);

    // type of SearchEventReceiver::handleShowAdvanceSearchBar
    typedef void (SearchEventReceiver::*SubFunc3)(quint64, bool);
    typedef bool (EventDispatcherManager::*SubType3)(const QString &, const QString &, SearchEventReceiver *, SubFunc3);

    // type of SearchEventReceiver::handleUrlChanged
    typedef void (SearchEventReceiver::*SubFunc4)(quint64, const QUrl &);
    typedef bool (EventDispatcherManager::*SubType4)(const QString &, const QString &, SearchEventReceiver *, SubFunc4);

    auto subscribe1 = static_cast<SubType1>(&EventDispatcherManager::subscribe);
    st.set_lamda(subscribe1, [] { return true; });
    auto subscribe2 = static_cast<SubType2>(&EventDispatcherManager::subscribe);
    st.set_lamda(subscribe2, [] { return true; });
    auto subscribe3 = static_cast<SubType3>(&EventDispatcherManager::subscribe);
    st.set_lamda(subscribe3, [] { return true; });
    auto subscribe4 = static_cast<SubType4>(&EventDispatcherManager::subscribe);
    st.set_lamda(subscribe4, [] { return true; });

    // connect
    // type of CustomManager::registerCustomInfo
    typedef bool (CustomManager::*SigFunc1)(const QString &, const QVariantMap &);
    typedef bool (EventChannelManager::*SigType1)(const QString &, const QString &, CustomManager *, SigFunc1);

    // type of CustomManager::isDisableSearch
    typedef bool (CustomManager::*SigFunc2)(const QUrl &);
    typedef bool (EventChannelManager::*SigType2)(const QString &, const QString &, CustomManager *, SigFunc2);

    // type of CustomManager::redirectedPath
    typedef QString (CustomManager::*SigFunc3)(const QUrl &);
    typedef bool (EventChannelManager::*SigType3)(const QString &, const QString &, CustomManager *, SigFunc3);

    auto connect1 = static_cast<SigType1>(&EventChannelManager::connect);
    st.set_lamda(connect1, [] { return true; });
    auto connect2 = static_cast<SigType2>(&EventChannelManager::connect);
    st.set_lamda(connect2, [] { return true; });
    auto connect3 = static_cast<SigType3>(&EventChannelManager::connect);
    st.set_lamda(connect3, [] { return true; });

    Search search;
    EXPECT_NO_FATAL_FAILURE(search.bindEvents());
}
