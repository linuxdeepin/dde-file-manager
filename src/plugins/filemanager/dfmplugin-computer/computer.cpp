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
#include "fileentity/appentryfileentity.h"
#include "events/computerunicastreceiver.h"
#include "events/computereventreceiver.h"
#include "watcher/computeritemwatcher.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/entry/entities/entryentities.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/utils/devicemanager.h"

#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/windows/windowsservice.h"

DSB_FM_USE_NAMESPACE
namespace GlobalPrivate {
static WindowsService *winServ { nullptr };
}   // namespace GlobalPrivate

DPCOMPUTER_BEGIN_NAMESPACE
/*!
 * \class Computer
 * \brief the plugin initializer
 */
void Computer::initialize()
{
    DFMBASE_USE_NAMESPACE
    DPCOMPUTER_USE_NAMESPACE
    UrlRoute::regScheme(ComputerUtils::scheme(), "/", ComputerUtils::icon(), true, tr("Computer"));
    ViewFactory::regClass<ComputerView>(ComputerUtils::scheme());
    UrlRoute::regScheme(SchemeTypes::kEntry, "/", QIcon(), true);
    InfoFactory::regClass<EntryFileInfo>(SchemeTypes::kEntry);

    EntryEntityFactor::registCreator<UserEntryFileEntity>(SuffixInfo::kUserDir);
    EntryEntityFactor::registCreator<BlockEntryFileEntity>(SuffixInfo::kBlock);
    EntryEntityFactor::registCreator<ProtocolEntryFileEntity>(SuffixInfo::kProtocol);
    EntryEntityFactor::registCreator<StashedProtocolEntryFileEntity>(SuffixInfo::kStashedRemote);
    EntryEntityFactor::registCreator<AppEntryFileEntity>(SuffixInfo::kAppEntry);

    ComputerUnicastReceiver::instance()->connectService();
    bool ret = DeviceManagerInstance.connectToServer();
    if (!ret)
        qCritical() << "device manager cannot connect to server!";

    auto &ctx = dpfInstance.serviceContext();
    DSB_FM_USE_NAMESPACE
    Q_ASSERT_X(ctx.loaded(WindowsService::name()), "Computer", "WindowService not loaded");
    GlobalPrivate::winServ = ctx.service<WindowsService>(WindowsService::name());
    connect(GlobalPrivate::winServ, &WindowsService::windowOpened, this, &Computer::onWindowOpened, Qt::DirectConnection);
    connect(GlobalPrivate::winServ, &WindowsService::windowClosed, this, &Computer::onWindowClosed, Qt::DirectConnection);
}

bool Computer::start()
{
    dpfInstance.eventDispatcher().subscribe(SideBar::EventType::kEjectAction, ComputerEventReceiverIns, &ComputerEventReceiver::handleItemEject);
    return true;
}

dpf::Plugin::ShutdownFlag Computer::stop()
{
    return kSync;
}

void Computer::onWindowOpened(quint64 windId)
{
    auto window = GlobalPrivate::winServ->findWindowById(windId);
    Q_ASSERT_X(window, "Computer", "Cannot find window by id");
    connect(window, &FileManagerWindow::workspaceInstallFinished, this, [] { ComputerItemWatcherInstance->startQueryItems(); }, Qt::DirectConnection);
    connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { this->addComputerToSidebar(); }, Qt::DirectConnection);
}

void Computer::onWindowClosed(quint64 winId)
{
    // TODO(xust)
}

void Computer::addComputerToSidebar()
{
    auto &ctx = dpfInstance.serviceContext();

    DSB_FM_USE_NAMESPACE
    if (!ctx.load(SideBarService::name()))
        abort();
    auto sidebarServ = ctx.service<SideBarService>(SideBarService::name());

    SideBar::ItemInfo entry;
    entry.group = SideBar::DefaultGroup::kDevice;
    entry.iconName = ComputerUtils::icon().name();
    entry.text = tr("Computer");
    entry.url = ComputerUtils::rootUrl();
    entry.flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    sidebarServ->addItem(entry, nullptr, nullptr);
}

DPCOMPUTER_END_NAMESPACE
