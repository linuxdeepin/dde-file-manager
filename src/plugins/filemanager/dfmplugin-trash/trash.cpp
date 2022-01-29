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
#include "utils/trashmanager.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

DPTRASH_USE_NAMESPACE

namespace GlobalPrivate {
static WindowsService *winServ { nullptr };
static SideBarService *sideBarService { nullptr };
static WorkspaceService *workspaceService { nullptr };
static FileOperationsService *fileOperationsService { nullptr };
}   // namespace GlobalPrivate

void Trash::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(SideBarService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
    if (!ctx.load(WorkspaceService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    if (!ctx.load(FileOperationsService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    UrlRoute::regScheme(TrashManager::scheme(), "/", TrashManager::icon(), true, tr("Trash"));
    InfoFactory::regClass<TrashFileInfo>(TrashManager::scheme());
    WacherFactory::regClass<TrashFileWatcher>(TrashManager::scheme());
    DirIteratorFactory::regClass<TrashDirIterator>(TrashManager::scheme());

    GlobalPrivate::winServ = ctx.service<WindowsService>(WindowsService::name());
    Q_ASSERT(GlobalPrivate::winServ);
    connect(GlobalPrivate::winServ, &WindowsService::windowOpened, this, &Trash::onWindowOpened, Qt::DirectConnection);

    GlobalPrivate::fileOperationsService = ctx.service<FileOperationsService>(FileOperationsService::name());
}

bool Trash::start()
{
    auto &ctx = dpfInstance.serviceContext();
    GlobalPrivate::sideBarService = ctx.service<SideBarService>(SideBarService::name());
    if (!GlobalPrivate::sideBarService) {
        qCritical() << "Failed, init sidebar \"sideBarService\" is empty";
        abort();
    }

    installToSideBar();

    GlobalPrivate::workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());

    if (!GlobalPrivate::workspaceService) {
        qCritical() << "Failed, init workspace \"workspaceService\" is empty";
        abort();
    }
    GlobalPrivate::workspaceService->addScheme(TrashManager::scheme());

    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &TrashManager::openFilesHandle;
    fileOpeationsHandle->writeUrlsToClipboard = &TrashManager::writeToClipBoardHandle;
    GlobalPrivate::fileOperationsService->registerOperations(TrashManager::scheme(), fileOpeationsHandle);

    return true;
}

dpf::Plugin::ShutdownFlag Trash::stop()
{
    return kSync;
}

void Trash::onWindowOpened(quint64 windId)
{
    auto window = GlobalPrivate::winServ->findWindowById(windId);
    Q_ASSERT_X(window, "Trash", "Cannot find window by id");
    if (window->titleBar())
        regTrashCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Trash::regTrashCrumbToTitleBar, Qt::DirectConnection);
}

void Trash::regTrashCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        auto &ctx = dpfInstance.serviceContext();
        if (ctx.load(TitleBarService::name())) {
            auto titleBarServ = ctx.service<TitleBarService>(TitleBarService::name());
            TitleBar::CustomCrumbInfo info;
            info.scheme = TrashManager::scheme();
            info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == TrashManager::scheme(); };
            titleBarServ->addCustomCrumbar(info);
        }
    });
}

void Trash::installToSideBar()
{
    SideBar::ItemInfo item;
    item.group = SideBar::DefaultGroup::kCommon;
    item.url = TrashManager::rootUrl();
    item.iconName = TrashManager::icon().name();
    item.text = tr("Trash");
    item.flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    item.contextMenuCb = TrashManager::contenxtMenuHandle;

    GlobalPrivate::sideBarService->addItem(item);
}
