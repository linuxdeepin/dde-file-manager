// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computer.h"
#include "utils/computerutils.h"
#include "utils/remotepasswdmanager.h"
#include "views/computerviewcontainer.h"
#include "fileentity/entryfileentities.h"
#include "events/computereventreceiver.h"
#include "watcher/computeritemwatcher.h"
#include "menu/computermenuscene.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)
Q_DECLARE_METATYPE(QList<QVariantMap> *)
Q_DECLARE_METATYPE(QString *)

using DirAccessPrehandlerType = std::function<void(quint64 winId, const QUrl &url, std::function<void()> after)>;
Q_DECLARE_METATYPE(DirAccessPrehandlerType)

inline constexpr char kComputerDConfigName[] { "org.deepin.dde.file-manager.computer" };
inline constexpr char kComputerDConfHideMyDirs[] { "hideMyDirectories" };
inline constexpr char kComputerDConfHide3rdEntries[] { "hide3rdEntries" };

#define COMPUTER_SETTING_GROUP "02_workspace.02_computer"
inline constexpr char kComputerSettingGroup[] { COMPUTER_SETTING_GROUP };
inline constexpr char kComputerSettingHideMyDirs[] { COMPUTER_SETTING_GROUP ".00_hide_my_directories" };
inline constexpr char kComputerSettingHideBuiltin[] { COMPUTER_SETTING_GROUP ".01_hide_builtin_partition" };
inline constexpr char kComputerSettingHideLoop[] { COMPUTER_SETTING_GROUP ".02_hide_loop_partitions" };
inline constexpr char kComputerSettingHide3rdEntries[] { COMPUTER_SETTING_GROUP ".03_hide_3rd_entryies" };
inline constexpr char kComputerSettingShowFsTag[] { COMPUTER_SETTING_GROUP ".04_show_filesystemtag_on_diskicon" };

DFMBASE_USE_NAMESPACE

namespace dfmplugin_computer {
DFM_LOG_REISGER_CATEGORY(DPCOMPUTER_NAMESPACE)

/*!
 * \class Computer
 * \brief the plugin initializer
 */
void Computer::initialize()
{
    DFMBASE_USE_NAMESPACE
    using namespace dfmplugin_computer;

    UrlRoute::regScheme(ComputerUtils::scheme(), "/", ComputerUtils::icon(), true, tr("Computer"));
    ViewFactory::regClass<ComputerViewContainer>(ComputerUtils::scheme());
    UrlRoute::regScheme(Global::Scheme::kEntry, "/", QIcon(), true);
    InfoFactory::regClass<EntryFileInfo>(Global::Scheme::kEntry);

    EntryEntityFactor::registCreator<CommonEntryFileEntity>(SuffixInfo::kCommon);
    EntryEntityFactor::registCreator<UserEntryFileEntity>(SuffixInfo::kUserDir);
    EntryEntityFactor::registCreator<BlockEntryFileEntity>(SuffixInfo::kBlock);
    EntryEntityFactor::registCreator<ProtocolEntryFileEntity>(SuffixInfo::kProtocol);
    EntryEntityFactor::registCreator<AppEntryFileEntity>(SuffixInfo::kAppEntry);

    bindEvents();
    followEvents();
    bindWindows();
}

bool Computer::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(ComputerMenuCreator::name(), new ComputerMenuCreator());

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", ComputerUtils::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", ComputerUtils::scheme(), ComputerMenuCreator::name());

    DirAccessPrehandlerType filePrehandler { ComputerEventReceiver::dirAccessPrehandler };
    if (!dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterRoutePrehandle", QString(Global::Scheme::kFile), filePrehandler).toBool())
        fmWarning() << "file's prehandler has been registered";

    addComputerSettingItem();
    return true;
}

void Computer::onWindowOpened(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    Q_ASSERT_X(window, "Computer", "Cannot find window by id");
    // register to computer
    regComputerCrumbToTitleBar();

    // init computer items
    if (window->workSpace())
        initComputerItems();
    else
        connect(
                window, &FileManagerWindow::workspaceInstallFinished,
                this, [this] { initComputerItems(); }, Qt::DirectConnection);

    if (window->sideBar())
        updateComputerToSidebar();
    else
        connect(
                window, &FileManagerWindow::sideBarInstallFinished,
                this, [this] { updateComputerToSidebar(); }, Qt::DirectConnection);

    // register to search
    auto searchPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-search") };
    if (searchPlugin && searchPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        regComputerToSearch();
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-search")
                        regComputerToSearch();
                },
                Qt::DirectConnection);
    }

    // register to property
    CustomViewExtensionView func { ComputerUtils::devicePropertyDialog };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_CustomView_Register",
                         func, QString(DFMBASE_NAMESPACE::Global::Scheme::kEntry));
}

void Computer::updateComputerToSidebar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable };
        QVariantMap map {
            { "Property_Key_QtItemFlags", QVariant::fromValue(flags) }
        };

        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", ComputerUtils::rootUrl(), map);
    });
}

void Computer::initComputerItems()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        ComputerItemWatcherInstance->startQueryItems();
    });
}

void Computer::regComputerCrumbToTitleBar()
{
    QVariantMap property;
    property["Property_Key_HideIconViewBtn"] = true;
    property["Property_Key_HideListViewBtn"] = true;
    property["Property_Key_HideTreeViewBtn"] = true;
    property["Property_Key_HideDetailSpaceBtn"] = true;
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", ComputerUtils::scheme(), property);
}

void Computer::regComputerToSearch()
{
    QVariantMap property;
    property["Property_Key_RedirectedPath"] = "/";
    property["Property_Key_UseNormalMenu"] = true;
    dpfSlotChannel->push("dfmplugin_search", "slot_Custom_Register", ComputerUtils::scheme(), property);
}

void Computer::addComputerSettingItem()
{
    QString err;
    auto ret = DConfigManager::instance()->addConfig(kComputerDConfigName, &err);
    if (!ret)
        fmWarning() << "cannot regist dconfig of computer plugin:" << err;

    SettingJsonGenerator::instance()->addGroup(kComputerSettingGroup, tr("Computer display items"));
    SettingJsonGenerator::instance()->addCheckBoxConfig(kComputerSettingHideBuiltin,
                                                        tr("Hide built-in disks on the Computer page"),
                                                        false);
    SettingJsonGenerator::instance()->addCheckBoxConfig(kComputerSettingHideLoop,
                                                        tr("Hide loop partitions on the Computer page"));
    SettingJsonGenerator::instance()->addCheckBoxConfig(kComputerSettingShowFsTag,
                                                        tr("Show file system on disk icon"),
                                                        false);

    SettingJsonGenerator::instance()->addCheckBoxConfig(kComputerSettingHideMyDirs,
                                                        tr("Hide My Directories on the Computer page"),
                                                        false);
    SettingBackend::instance()->addSettingAccessor(
            kComputerSettingHideMyDirs,
            []() {
                return DConfigManager::instance()->value(kComputerDConfigName,
                                                         kComputerDConfHideMyDirs,
                                                         false);
            },
            [](const QVariant &val) {
                DConfigManager::instance()->setValue(kComputerDConfigName,
                                                     kComputerDConfHideMyDirs,
                                                     val);
            });

    SettingJsonGenerator::instance()->addCheckBoxConfig(kComputerSettingHide3rdEntries,
                                                        tr("Hide 3rd party entries on the Computer page"),
                                                        false);
    SettingBackend::instance()->addSettingAccessor(
            kComputerSettingHide3rdEntries,
            []() {
                return DConfigManager::instance()->value(kComputerDConfigName,
                                                         kComputerDConfHide3rdEntries,
                                                         false);
            },
            [](const QVariant &val) {
                DConfigManager::instance()->setValue(kComputerDConfigName,
                                                     kComputerDConfHide3rdEntries,
                                                     val);
            });
}

void Computer::bindEvents()
{
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_Item_EjectClicked", ComputerEventReceiverIns, &ComputerEventReceiver::handleItemEject);

    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_ContextMenu_SetEnable", ComputerEventReceiver::instance(), &ComputerEventReceiver::setContextMenuEnable);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_Item_Add", ComputerItemWatcherInstance, &ComputerItemWatcher::addDevice);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_Item_Remove", ComputerItemWatcherInstance, &ComputerItemWatcher::removeDevice);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_View_Refresh", ComputerItemWatcherInstance, &ComputerItemWatcher::onViewRefresh);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_Passwd_Clear", RemotePasswdManagerInstance, &RemotePasswdManager::clearPasswd);
}

void Computer::followEvents()
{
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", ComputerEventReceiver::instance(), &ComputerEventReceiver::handleSepateTitlebarCrumb);
    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Group_Sort", ComputerEventReceiver::instance(), &ComputerEventReceiver::handleSortItem);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Tab_SetTabName", ComputerEventReceiver::instance(), &ComputerEventReceiver::handleSetTabName);
}

void Computer::bindWindows()
{
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Computer::onWindowOpened, Qt::DirectConnection);
}

}   // namespace dfmplugin_computer
