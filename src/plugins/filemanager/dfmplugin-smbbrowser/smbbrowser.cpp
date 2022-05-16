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

#include "smbbrowser.h"
#include "utils/smbbrowserutils.h"
#include "fileinfo/smbsharefileinfo.h"
#include "iterator/smbshareiterator.h"
#include "menu/smbbrowsermenuscene.h"

#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/common/menu/menuservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/dialogmanager.h"

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void SmbBrowser::initialize()
{
    UrlRoute::regScheme(Global::kSmb, "/", SmbBrowserUtils::icon(), true);
    UrlRoute::regScheme(SmbBrowserUtils::networkScheme(), "/", SmbBrowserUtils::icon(), true);

    InfoFactory::regClass<SmbShareFileInfo>(Global::kSmb);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::kSmb);

    InfoFactory::regClass<SmbShareFileInfo>(Global::kFtp);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::kFtp);

    InfoFactory::regClass<SmbShareFileInfo>(Global::kSFtp);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::kSFtp);

    InfoFactory::regClass<SmbShareFileInfo>(SmbBrowserUtils::networkScheme());
    DirIteratorFactory::regClass<SmbShareIterator>(SmbBrowserUtils::networkScheme());

    DSC_NAMESPACE::MenuService::service()->registerScene(SmbBrowserMenuCreator::name(), new SmbBrowserMenuCreator());

    DSB_FM_USE_NAMESPACE
    connect(WindowsService::service(), &WindowsService::windowOpened, this, &SmbBrowser::onWindowOpened, Qt::DirectConnection);

    connect(&dpfInstance.listener(), &dpf::Listener::pluginsStarted, this, &SmbBrowser::registerSambaPrehandler, Qt::DirectConnection);
}

bool SmbBrowser::start()
{
    DSB_FM_USE_NAMESPACE
    WorkspaceService::service()->addScheme(Global::kSmb);
    WorkspaceService::service()->setWorkspaceMenuScene(Global::kSmb, SmbBrowserMenuCreator::name());

    WorkspaceService::service()->addScheme(SmbBrowserUtils::networkScheme());
    WorkspaceService::service()->setWorkspaceMenuScene(SmbBrowserUtils::networkScheme(), SmbBrowserMenuCreator::name());
    initOperations();
    return true;
}

dpf::Plugin::ShutdownFlag SmbBrowser::stop()
{
    return dpf::Plugin::ShutdownFlag::kSync;
}

void SmbBrowser::onWindowCreated(quint64 winId)
{
}

void SmbBrowser::onWindowOpened(quint64 winId)
{
    DSB_FM_USE_NAMESPACE
    auto window = WindowsService::service()->findWindowById(winId);
    if (window->sideBar())
        addNeighborToSidebar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { addNeighborToSidebar(); }, Qt::DirectConnection);
}

void SmbBrowser::onWindowClosed(quint64 winId)
{
}

void SmbBrowser::initOperations()
{
    DSC_USE_NAMESPACE
    FileOperationsFunctions funcs(new FileOperationsSpace::FileOperationsInfo);
    funcs->openFiles = &SmbBrowserUtils::mountSmb;
    FileOperationsService::service()->registerOperations(Global::kSmb, funcs);
    FileOperationsService::service()->registerOperations(Global::kFtp, funcs);
    FileOperationsService::service()->registerOperations(Global::kSFtp, funcs);
}

void SmbBrowser::addNeighborToSidebar()
{
    DSB_FM_USE_NAMESPACE

    SideBar::ItemInfo entry;
    entry.group = SideBar::DefaultGroup::kNetwork;
    entry.iconName = SmbBrowserUtils::icon().name();
    entry.text = tr("Computers in LAN");
    entry.url = SmbBrowserUtils::netNeighborRootUrl();
    entry.flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    SideBarService::service()->insertItem(0, entry);
}

void SmbBrowser::registerSambaPrehandler()
{
    DSB_FM_USE_NAMESPACE
    if (!WorkspaceService::service()->registerFileViewRoutePrehandle(Global::kSmb, sambaPrehandler)) {
        qWarning() << "smb's prehandler has been registered";
    }
}

void SmbBrowser::sambaPrehandler(const QUrl &url, std::function<void()> after)
{
    if (url.scheme() == Global::kSmb) {
        DevMngIns->mountNetworkDeviceAsync(url.toString(), [after](bool ok, DFMMOUNT::DeviceError err, const QString &) {
            if ((ok || err == DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted) && after) {
                after();
            } else if (after) {
                DialogManager::instance()->showErrorDialog(tr("Mount device error"), "");
                qDebug() << DeviceUtils::errMessage(err);
            }
        },
                                           10000);
    }
}

QDebug operator<<(QDebug dbg, const DPSMBBROWSER_NAMESPACE::SmbShareNode &node)
{
    dbg.nospace() << "SmbShareNode: {"
                  << "\n\turl: " << node.url
                  << "\n\tname: " << node.displayName
                  << "\n\ticon: " << node.iconType
                  << "\n}";
    return dbg;
}
