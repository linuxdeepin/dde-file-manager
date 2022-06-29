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
#include "views/computerview.h"
#include "fileentity/entryfileentities.h"
#include "events/computerunicastreceiver.h"
#include "events/computereventreceiver.h"
#include "watcher/computeritemwatcher.h"
#include "menu/computermenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/propertydialog/propertydialogservice.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

Q_DECLARE_METATYPE(QList<QVariantMap> *);

DSB_FM_USE_NAMESPACE
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
    DSC_USE_NAMESPACE

    UrlRoute::regScheme(ComputerUtils::scheme(), "/", ComputerUtils::icon(), true, tr("Computer"));
    ViewFactory::regClass<ComputerView>(ComputerUtils::scheme());
    UrlRoute::regScheme(Global::Scheme::kEntry, "/", QIcon(), true);
    InfoFactory::regClass<EntryFileInfo>(Global::Scheme::kEntry);

    EntryEntityFactor::registCreator<UserEntryFileEntity>(SuffixInfo::kUserDir);
    EntryEntityFactor::registCreator<BlockEntryFileEntity>(SuffixInfo::kBlock);
    EntryEntityFactor::registCreator<ProtocolEntryFileEntity>(SuffixInfo::kProtocol);
    EntryEntityFactor::registCreator<StashedProtocolEntryFileEntity>(SuffixInfo::kStashedProtocol);
    EntryEntityFactor::registCreator<AppEntryFileEntity>(SuffixInfo::kAppEntry);

    ComputerUnicastReceiver::instance()->connectService();

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this, &Computer::onWindowCreated, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Computer::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &Computer::onWindowClosed, Qt::DirectConnection);

    bindEvents();
    followEvents();
}

bool Computer::start()
{
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_Item_EjectClicked", ComputerEventReceiverIns, &ComputerEventReceiver::handleItemEject);

    dfmplugin_menu_util::menuSceneRegisterScene(ComputerMenuCreator::name(), new ComputerMenuCreator());

    DSB_FM_USE_NAMESPACE
    WorkspaceService::service()->addScheme(ComputerUtils::scheme());
    WorkspaceService::service()->setWorkspaceMenuScene(ComputerUtils::scheme(), ComputerMenuCreator::name());

    return true;
}

dpf::Plugin::ShutdownFlag Computer::stop()
{
    return kSync;
}

void Computer::onWindowCreated(quint64 winId)
{
    Q_UNUSED(winId);
    regComputerCrumbToTitleBar();
}

void Computer::onWindowOpened(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    Q_ASSERT_X(window, "Computer", "Cannot find window by id");

    if (window->workSpace())
        ComputerItemWatcherInstance->startQueryItems();
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, [] { ComputerItemWatcherInstance->startQueryItems(); }, Qt::DirectConnection);

    if (window->sideBar())
        addComputerToSidebar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { addComputerToSidebar(); }, Qt::DirectConnection);

    if (window->titleBar())
        regComputerToSearch();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, [this] { regComputerToSearch(); }, Qt::DirectConnection);

    propertyServIns->registerCustomizePropertyView(ComputerUtils::devicePropertyDialog, DFMBASE_NAMESPACE::Global::Scheme::kEntry);
}

void Computer::onWindowClosed(quint64 winId)
{
    Q_UNUSED(winId);
}

void Computer::addComputerToSidebar()
{
    Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable };
    QVariantMap map {
        { "Property_Key_Group", "Group_Device" },
        { "Property_Key_DisplayName", tr("Computer") },
        { "Property_Key_Icon", ComputerUtils::icon() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) }
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
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_ContextMenu_SetEnable", ComputerUnicastReceiver::instance(), &ComputerUnicastReceiver::setContextMenuEnable);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_AddDevice", ComputerUnicastReceiver::instance(), &ComputerUnicastReceiver::doAddDevice);
    dpfSlotChannel->connect(EventNameSpace::kComputerEventSpace, "slot_RemoveDevice", ComputerUnicastReceiver::instance(), &ComputerUnicastReceiver::doRemoveDevice);
}

void Computer::followEvents()
{
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", ComputerEventReceiver::instance(), &ComputerEventReceiver::handleSepateTitlebarCrumb);
    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Group_Sort", ComputerEventReceiver::instance(), &ComputerEventReceiver::handleSortItem);
}

}   // namespace dfmplugin_computer
