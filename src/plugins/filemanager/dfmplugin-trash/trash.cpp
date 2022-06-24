/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#include "trash.h"
#include "trashfileinfo.h"
#include "trashdiriterator.h"
#include "trashfilewatcher.h"
#include "utils/trashhelper.h"
#include "utils/trashfilehelper.h"
#include "menus/trashmenuscene.h"

#include "services/common/delegate/delegateservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/menu/menuservice.h"
#include "services/common/propertydialog/propertydialogservice.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

Q_DECLARE_METATYPE(Qt::DropAction *)

DSC_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

void Trash::initialize()
{
    UrlRoute::regScheme(TrashHelper::scheme(), "/", TrashHelper::icon(), true, tr("Trash"));
    InfoFactory::regClass<TrashFileInfo>(TrashHelper::scheme());
    WatcherFactory::regClass<TrashFileWatcher>(TrashHelper::scheme());
    DirIteratorFactory::regClass<TrashDirIterator>(TrashHelper::scheme());
    delegateServIns->registerUrlTransform(TrashHelper::scheme(), TrashHelper::toLocalFile);
    DSC_NAMESPACE::MenuService::service()->registerScene(TrashMenuCreator::name(), new TrashMenuCreator());

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Trash::onWindowOpened, Qt::DirectConnection);
}

bool Trash::start()
{
    addCustomTopWidget();
    addFileOperations();

    dpfHookSequence->follow("dfmplugin_workspace", "hook_CheckDragDropAction", TrashHelper::instance(), &TrashHelper::checkDragDropAction);
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_DetailViewIcon", TrashHelper::instance(), &TrashHelper::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomColumnRoles", TrashHelper::instance(), &TrashHelper::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomRoleDisplayName", TrashHelper::instance(), &TrashHelper::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomRoleData", TrashHelper::instance(), &TrashHelper::customRoleData);

    return true;
}

dpf::Plugin::ShutdownFlag Trash::stop()
{
    return kSync;
}

void Trash::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "Trash", "Cannot find window by id");
    if (window->titleBar())
        regTrashCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Trash::regTrashCrumbToTitleBar, Qt::DirectConnection);

    if (window->sideBar())
        installToSideBar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { installToSideBar(); }, Qt::DirectConnection);
}

void Trash::regTrashCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        TitleBar::CustomCrumbInfo info;
        info.scheme = TrashHelper::scheme();
        info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == TrashHelper::scheme(); };
        TrashHelper::titleServIns()->addCustomCrumbar(info);
    });
}

void Trash::installToSideBar()
{
    SideBar::ItemInfo item;
    item.group = SideBar::DefaultGroup::kCommon;
    item.url = TrashHelper::rootUrl();
    item.iconName = TrashHelper::icon().name();
    item.text = tr("Trash");
    item.flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    item.contextMenuCb = TrashHelper::contenxtMenuHandle;
    TrashHelper::sideBarServIns()->addItem(item);
}

void Trash::addFileOperations()
{
    TrashHelper::workspaceServIns()->addScheme(TrashHelper::scheme());
    WorkspaceService::service()->setWorkspaceMenuScene(Global::Scheme::kTrash, TrashMenuCreator::name());

    propertyServIns->registerBasicViewFiledExpand(TrashHelper::propetyExtensionFunc, TrashHelper::scheme());
    propertyServIns->registerFilterControlField(TrashHelper::scheme(), Property::FilePropertyControlFilter::kPermission);

    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &TrashFileHelper::openFilesHandle;
    fileOpeationsHandle->writeUrlsToClipboard = &TrashFileHelper::writeToClipBoardHandle;
    fileOpeationsHandle->moveToTash = &TrashFileHelper::moveToTrashHandle;
    fileOpeationsHandle->moveFromPlugin = &TrashFileHelper::moveFromTrashHandle;
    fileOpeationsHandle->deletes = &TrashFileHelper::deletesHandle;
    fileOpeationsHandle->copy = &TrashFileHelper::copyHandle;
    fileOpeationsHandle->cut = &TrashFileHelper::cutHandle;
    TrashHelper::fileOperationsServIns()->registerOperations(TrashHelper::scheme(), fileOpeationsHandle);
}

void Trash::addCustomTopWidget()
{
    Workspace::CustomTopWidgetInfo info;
    info.scheme = TrashHelper::scheme();
    info.createTopWidgetCb = TrashHelper::createEmptyTrashTopWidget;
    info.showTopWidgetCb = TrashHelper::showTopWidget;
    TrashHelper::workspaceServIns()->addCustomTopWidget(info);
}
