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

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/dialogmanager.h"

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

void SmbBrowser::initialize()
{
    UrlRoute::regScheme(Global::Scheme::kSmb, "/", SmbBrowserUtils::icon(), true);
    UrlRoute::regScheme(SmbBrowserUtils::networkScheme(), "/", SmbBrowserUtils::icon(), true);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kSmb);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kSmb);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kFtp);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kFtp);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kSFtp);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kSFtp);

    InfoFactory::regClass<SmbShareFileInfo>(SmbBrowserUtils::networkScheme());
    DirIteratorFactory::regClass<SmbShareIterator>(SmbBrowserUtils::networkScheme());

    DSB_FM_USE_NAMESPACE
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &SmbBrowser::onWindowOpened, Qt::DirectConnection);

    connect(dpfListener, &dpf::Listener::pluginsStarted, this, &SmbBrowser::registerSambaPrehandler, Qt::DirectConnection);
}

bool SmbBrowser::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(SmbBrowserMenuCreator::name(), new SmbBrowserMenuCreator());

    DSB_FM_USE_NAMESPACE
    WorkspaceService::service()->addScheme(Global::Scheme::kSmb);
    WorkspaceService::service()->setWorkspaceMenuScene(Global::Scheme::kSmb, SmbBrowserMenuCreator::name());

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
    auto window = FMWindowsIns.findWindowById(winId);
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
    FileOperationsService::service()->registerOperations(Global::Scheme::kSmb, funcs);
    FileOperationsService::service()->registerOperations(Global::Scheme::kFtp, funcs);
    FileOperationsService::service()->registerOperations(Global::Scheme::kSFtp, funcs);
}

void SmbBrowser::addNeighborToSidebar()
{
    Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren };
    QVariantMap map {
        { "Property_Key_Group", "Group_Network" },
        { "Property_Key_DisplayName", tr("Computers in LAN") },
        { "Property_Key_Icon", SmbBrowserUtils::icon() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) }
    };

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Insert", 0, SmbBrowserUtils::netNeighborRootUrl(), map);
}

void SmbBrowser::registerSambaPrehandler()
{
    DSB_FM_USE_NAMESPACE
    if (!WorkspaceService::service()->registerFileViewRoutePrehandle(Global::Scheme::kSmb, sambaPrehandler)) {
        qWarning() << "smb's prehandler has been registered";
    }
}

void SmbBrowser::sambaPrehandler(const QUrl &url, std::function<void()> after)
{
    if (url.scheme() == Global::Scheme::kSmb) {
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
