// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "search.h"
#include "events/searcheventreceiver.h"
#include "utils/searchhelper.h"
#include "utils/custommanager.h"
#include "utils/textindexclient.h"
#include "fileinfo/searchfileinfo.h"
#include "iterator/searchdiriterator.h"
#include "watcher/searchfilewatcher.h"
#include "topwidget/advancesearchbar.h"
#include "menus/searchmenuscene.h"
#include "searchmanager/searchmanager.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/viewdefines.h>
#include <dfm-base/utils/sysinfoutils.h>

using CreateTopWidgetCallback = std::function<QWidget *()>;
using ShowTopWidgetCallback = std::function<bool(QWidget *, const QUrl &)>;
using ViewModeUrlCallback = std::function<QUrl(const QUrl)>;
Q_DECLARE_METATYPE(CreateTopWidgetCallback);
Q_DECLARE_METATYPE(ShowTopWidgetCallback);
Q_DECLARE_METATYPE(QList<QVariantMap> *);
Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(ViewModeUrlCallback)

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

namespace dfmplugin_search {
DFM_LOG_REGISTER_CATEGORY(DPSEARCH_NAMESPACE)

void Search::initialize()
{
    UrlRoute::regScheme(SearchHelper::scheme(), "/", {}, true, tr("Search"));
    // 注册Scheme为"search"的扩展的文件信息
    InfoFactory::regClass<SearchFileInfo>(SearchHelper::scheme());
    DirIteratorFactory::regClass<SearchDirIterator>(SearchHelper::scheme());
    WatcherFactory::regClass<SearchFileWatcher>(SearchHelper::scheme(),
                                                WatcherFactory::RegOpts::kNoCache);

    // must inited in main thread
    TextIndexClient::instance();

    bindEvents();
    bindWindows();

    connect(DConfigManager::instance(), &DConfigManager::valueChanged,
            SearchManager::instance(), &SearchManager::onDConfigValueChanged, Qt::DirectConnection);
}

bool Search::start()
{
    regSearchSettingConfig();
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
    ViewModeUrlCallback viewModelUrlCallback { SearchHelper::viewModelUrl };
    QVariantMap property;
    property["Property_Key_KeepAddressBar"] = true;
    property[ViewCustomKeys::kSupportTreeMode] = false;
    property[ViewCustomKeys::kAllowChangeListHeight] = false;
    property[ViewCustomKeys::kViewModeUrlCallback] = QVariant::fromValue(viewModelUrlCallback);
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", SearchHelper::scheme(), property);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_RedirectUrl",
                            SearchHelper::instance(), &SearchHelper::crumbRedirectUrl);

    QStringList &&filtes { "kFileSizeField", "kFileChangeTimeField", "kFileInterviewTimeField" };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicFiledFilter_Add",
                         SearchHelper::scheme(), filtes);
}

void Search::regSearchToWorkspace()
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", SearchHelper::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", SearchHelper::scheme(), SearchMenuCreator::name());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterLoadStrategy", SearchHelper::scheme(), DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy::kPreserve);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFocusFileViewDisabled", SearchHelper::scheme());

    ViewModeUrlCallback viewModelUrlCallback { SearchHelper::viewModelUrl };
    QVariantMap propertise {
        { DFMGLOBAL_NAMESPACE::ViewCustomKeys::kSupportTreeMode, false },
        { DFMGLOBAL_NAMESPACE::ViewCustomKeys::kDefaultViewMode, static_cast<int>(Global::ViewMode::kListMode) },
        { DFMGLOBAL_NAMESPACE::ViewCustomKeys::kAllowChangeListHeight, false },
        { DFMGLOBAL_NAMESPACE::ViewCustomKeys::kDefaultListHeight, ViewDefines().listHeightCount() - 1 },
        { DFMGLOBAL_NAMESPACE::ViewCustomKeys::kViewModeUrlCallback, QVariant::fromValue(viewModelUrlCallback) }
    };
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetCustomViewProperty", SearchHelper::scheme(), propertise);

    CreateTopWidgetCallback createCallback { []() { return new AdvanceSearchBar(); } };
    ShowTopWidgetCallback showCallback { SearchHelper::showTopWidget };

    QVariantMap map {
        { "Property_Key_Scheme", SearchHelper::scheme() },
        { "Property_Key_KeepShow", false },
        { "Property_Key_KeepTop", true },
        { "Property_Key_CreateTopWidgetCallback", QVariant::fromValue(createCallback) },
        { "Property_Key_ShowTopWidgetCallback", QVariant::fromValue(showCallback) }
    };

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterCustomTopWidget", map);
}

void Search::regSearchSettingConfig()
{
    if (SysInfoUtils::isOpenAsAdmin())
        return;

    QString err;
    auto ret = DConfigManager::instance()->addConfig(DConfig::kSearchCfgPath, &err);
    if (!ret)
        fmWarning() << "cannot regist dconfig of search plugin:" << err;

    SettingJsonGenerator::instance()->addGroup(SearchSettings::kGroupSearch, tr("Search"));

    QString textIndexKey { SearchSettings::kFulltextSearch };
    DialogManager::instance()->registerSettingWidget("checkBoxWidthTextIndex", &SearchHelper::createCheckBoxWidthTextIndex);
    SettingJsonGenerator::instance()->addConfig(SearchSettings::kFulltextSearch,
                                                { { "key", textIndexKey.mid(textIndexKey.lastIndexOf(".") + 1) },
                                                  { "text", tr("Full-Text search") },
                                                  { "type", "checkBoxWidthTextIndex" },
                                                  { "default", true } });

    SettingBackend::instance()->addSettingAccessor(
            SearchSettings::kFulltextSearch,
            []() {
                return DConfigManager::instance()->value(DConfig::kSearchCfgPath,
                                                         DConfig::kEnableFullTextSearch,
                                                         true);
            },
            [](const QVariant &val) {
                DConfigManager::instance()->setValue(DConfig::kSearchCfgPath,
                                                     DConfig::kEnableFullTextSearch,
                                                     val);
            });
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
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Allow_Repeat_Url",
                            SearchHelper::instance(), &SearchHelper::allowRepeatUrl);
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch",
                            SearchHelper::instance(), &SearchHelper::searchIconName);

    // subscribe signal events
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Search_Start",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleSearch);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_Search_Stop",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleStopSearch);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_FilterView_Show",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleShowAdvanceSearchBar);
    dpfSignalDispatcher->subscribe("dfmplugin_titlebar", "signal_InputAdddressStr_Check",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleAddressInputStr);

    dpfSignalDispatcher->subscribe("dfmplugin_fileoperations", "signal_File_Add",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleFileAdd);
    dpfSignalDispatcher->subscribe("dfmplugin_fileoperations", "signal_File_Delete",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleFileDelete);
    dpfSignalDispatcher->subscribe("dfmplugin_fileoperations", "signal_File_Rename",
                                   SearchEventReceiverIns, &SearchEventReceiver::handleFileRename);

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
