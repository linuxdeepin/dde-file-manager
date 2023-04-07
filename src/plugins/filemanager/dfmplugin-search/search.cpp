// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "search.h"
#include "events/searcheventreceiver.h"
#include "utils/searchhelper.h"
#include "utils/custommanager.h"
#include "fileinfo/searchfileinfo.h"
#include "iterator/searchdiriterator.h"
#include "watcher/searchfilewatcher.h"
#include "topwidget/advancesearchbar.h"
#include "menus/searchmenuscene.h"
#include "searchmanager/searchmanager.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/application/application.h>

using CreateTopWidgetCallback = std::function<QWidget *()>;
using ShowTopWidgetCallback = std::function<bool(QWidget *, const QUrl &)>;
Q_DECLARE_METATYPE(CreateTopWidgetCallback);
Q_DECLARE_METATYPE(ShowTopWidgetCallback);
Q_DECLARE_METATYPE(QList<QVariantMap> *);
Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(QVariant *)

DFMBASE_USE_NAMESPACE
namespace dfmplugin_search {

void Search::initialize()
{
    UrlRoute::regScheme(SearchHelper::scheme(), "/", {}, true, tr("Search"));
    //注册Scheme为"search"的扩展的文件信息
    InfoFactory::regClass<SearchFileInfo>(SearchHelper::scheme());
    DirIteratorFactory::regClass<SearchDirIterator>(SearchHelper::scheme());
    WatcherFactory::regClass<SearchFileWatcher>(SearchHelper::scheme());

    bindEvents();
    bindWindows();

    connect(Application::instance(), &Application::indexFullTextSearchChanged,
            SearchManager::instance(), &SearchManager::onIndexFullTextConfigChanged, Qt::DirectConnection);
}

bool Search::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(SearchMenuCreator::name(), new SearchMenuCreator());
    return true;
}

void Search::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);

    if (window->workSpace())
        regSearchToWorkspace();
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, &Search::regSearchToWorkspace, Qt::DirectConnection);

    if (window->titleBar())
        regSearchCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Search::regSearchCrumbToTitleBar, Qt::DirectConnection);
}

void Search::regSearchCrumbToTitleBar()
{
    QVariantMap property;
    property["Property_Key_KeepAddressBar"] = true;
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", SearchHelper::scheme(), property);

    QStringList &&filtes { "kFileSizeField", "kFileChangeTimeField", "kFileInterviewTimeField" };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicFiledFilter_Add",
                         SearchHelper::scheme(), filtes);
}

void Search::regSearchToWorkspace()
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", SearchHelper::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", SearchHelper::scheme(), SearchMenuCreator::name());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetDefaultViewMode", SearchHelper::scheme(), Global::ViewMode::kListMode);

    CreateTopWidgetCallback createCallback { []() { return new AdvanceSearchBar(); } };
    ShowTopWidgetCallback showCallback { SearchHelper::showTopWidget };

    QVariantMap map {
        { "Property_Key_Scheme", SearchHelper::scheme() },
        { "Property_Key_KeepShow", false },
        { "Property_Key_CreateTopWidgetCallback", QVariant::fromValue(createCallback) },
        { "Property_Key_ShowTopWidgetCallback", QVariant::fromValue(showCallback) }
    };

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterCustomTopWidget", map);
}

void Search::bindEvents()
{
    // hook events
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles",
                            SearchHelper::instance(), &SearchHelper::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName",
                            SearchHelper::instance(), &SearchHelper::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                            SearchHelper::instance(), &SearchHelper::blockPaste);
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch",
                            SearchHelper::instance(), &SearchHelper::searchIconName);

    // subscribe signal events
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Search_Start",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleSearch);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Search_Stop",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleStopSearch);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_FilterView_Show",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleShowAdvanceSearchBar);
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl,
                                   SearchEventReceiverIns, &SearchEventReceiver::handleUrlChanged);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_InputAdddressStr_Check",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleAddressInputStr);

    // connect self slot events
    static constexpr auto selfSpace { DPF_MACRO_TO_STR(DPSEARCH_NAMESPACE) };
    dpfSlotChannel->connect(selfSpace, "slot_Custom_Register",
                            CustomManager::instance(), &CustomManager::registerCustomInfo);
    dpfSlotChannel->connect(selfSpace, "slot_Custom_IsDisableSearch",
                            CustomManager::instance(), &CustomManager::isDisableSearch);
    dpfSlotChannel->connect(selfSpace, "slot_Custom_RedirectedPath",
                            CustomManager::instance(), &CustomManager::redirectedPath);
}

void Search::bindWindows()
{
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Search::onWindowOpened, Qt::DirectConnection);
}

}   // namespace Search
