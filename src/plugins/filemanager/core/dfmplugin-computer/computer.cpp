/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "computer.h"
#include "utils/computerutils.h"
#include "utils/stashmountsutils.h"
#include "views/computerviewcontainer.h"
#include "fileentity/entryfileentities.h"
#include "events/computereventreceiver.h"
#include "watcher/computeritemwatcher.h"
#include "menu/computermenuscene.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)
Q_DECLARE_METATYPE(QList<QVariantMap> *)
Q_DECLARE_METATYPE(QString *)

DFMBASE_USE_NAMESPACE

namespace dfmplugin_computer {
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

    EntryEntityFactor::registCreator<UserEntryFileEntity>(SuffixInfo::kUserDir);
    EntryEntityFactor::registCreator<BlockEntryFileEntity>(SuffixInfo::kBlock);
    EntryEntityFactor::registCreator<ProtocolEntryFileEntity>(SuffixInfo::kProtocol);
    EntryEntityFactor::registCreator<StashedProtocolEntryFileEntity>(SuffixInfo::kStashedProtocol);
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

    StashMountsUtils::bindStashEnableConf();
    return true;
}

void Computer::onWindowOpened(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    Q_ASSERT_X(window, "Computer", "Cannot find window by id");
    regComputerCrumbToTitleBar();
    if (window->workSpace())
        ComputerItemWatcherInstance->startQueryItems();
    else
        connect(
                window, &FileManagerWindow::workspaceInstallFinished,
                this, [] { ComputerItemWatcherInstance->startQueryItems(); }, Qt::DirectConnection);

    if (window->sideBar())
        addComputerToSidebar();
    else
        connect(
                window, &FileManagerWindow::sideBarInstallFinished,
                this, [this] { addComputerToSidebar(); }, Qt::DirectConnection);

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

    CustomViewExtensionView func { ComputerUtils::devicePropertyDialog };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_CustomView_Register",
                         func, QString(DFMBASE_NAMESPACE::Global::Scheme::kEntry));
}

void Computer::addComputerToSidebar()
{
    Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable };
    QVariantMap map {
        { "Property_Key_Group", "Group_Device" },
        { "Property_Key_DisplayName", tr("Computer") },
        { "Property_Key_Icon", ComputerUtils::icon() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
        { "Property_Key_VisiableControl", "computer" },
        { "Property_Key_ReportName", "Computer" }
    };

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Insert", 0, ComputerUtils::rootUrl(), map);
}

void Computer::regComputerCrumbToTitleBar()
{
    QVariantMap property;
    property["Property_Key_HideIconViewBtn"] = true;
    property["Property_Key_HideListViewBtn"] = true;
    property["Property_Key_HideDetailSpaceBtn"] = true;
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", ComputerUtils::scheme(), property);
}

void Computer::regComputerToSearch()
{
    QVariantMap property;
    property["Property_Key_RedirectedPath"] = "/";
    dpfSlotChannel->push("dfmplugin_search", "slot_Custom_Register", ComputerUtils::scheme(), property);
}

void Computer::bindEvents()
{
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_Item_EjectClicked", ComputerEventReceiverIns, &ComputerEventReceiver::handleItemEject);

    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_ContextMenu_SetEnable", ComputerEventReceiver::instance(), &ComputerEventReceiver::setContextMenuEnable);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_AddDevice", ComputerItemWatcherInstance, &ComputerItemWatcher::addDevice);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_RemoveDevice", ComputerItemWatcherInstance, &ComputerItemWatcher::removeDevice);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_ComputerView_Refresh", ComputerItemWatcherInstance, &ComputerItemWatcher::startQueryItems);
}

void Computer::followEvents()
{
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", ComputerEventReceiver::instance(), &ComputerEventReceiver::handleSepateTitlebarCrumb);
    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Group_Sort", ComputerEventReceiver::instance(), &ComputerEventReceiver::handleSortItem);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Tab_SetTabName", ComputerEventReceiver::instance(), &ComputerEventReceiver::handleSetTabName);
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
