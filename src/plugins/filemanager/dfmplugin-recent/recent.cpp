/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "recent.h"
#include "recentfileinfo.h"
#include "recentdiriterator.h"
#include "recentfilewatcher.h"
#include "utils/recenthelper.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"

#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

DPRECENT_BEGIN_NAMESPACE

namespace GlobalPrivate {
static WindowsService *winServ { nullptr };
static SideBarService *sideBarService { nullptr };
static WorkspaceService *workspaceService { nullptr };
static FileOperationsService *fileOperationsService { nullptr };
}   // namespace GlobalPrivate

void Recent::initialize()
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

    UrlRoute::regScheme(RecentHelper::scheme(), "/", RecentHelper::icon(), true, tr("Recent"));
    //注册Scheme为"recent"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<RecentFileInfo>(RecentHelper::scheme());
    WacherFactory::regClass<RecentFileWatcher>(RecentHelper::scheme());
    DirIteratorFactory::regClass<RecentDirIterator>(RecentHelper::scheme());

    GlobalPrivate::winServ = ctx.service<WindowsService>(WindowsService::name());
    Q_ASSERT(GlobalPrivate::winServ);
    connect(GlobalPrivate::winServ, &WindowsService::windowOpened, this, &Recent::onWindowOpened, Qt::DirectConnection);

    GlobalPrivate::fileOperationsService = ctx.service<FileOperationsService>(FileOperationsService::name());
    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &RecentHelper::openFilesHandle;
    fileOpeationsHandle->writeUrlsToClipboard = &RecentHelper::writeToClipBoardHandle;
    GlobalPrivate::fileOperationsService->registerOperations(RecentHelper::scheme(), fileOpeationsHandle);
}

bool Recent::start()
{
    auto &ctx = dpfInstance.serviceContext();
    GlobalPrivate::sideBarService = ctx.service<SideBarService>(SideBarService::name());
    if (!GlobalPrivate::sideBarService) {
        qCritical() << "Failed, init sidebar \"sideBarService\" is empty";
        abort();
    }

    bool showRecentEnabled = Application::instance()->genericAttribute(Application::kShowRecentFileEntry).toBool();

    if (showRecentEnabled) {
        addRecentItem();
    }

    GlobalPrivate::workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());

    if (!GlobalPrivate::workspaceService) {
        qCritical() << "Failed, init workspace \"workspaceService\" is empty";
        abort();
    }
    GlobalPrivate::workspaceService->addScheme(RecentHelper::scheme());

    connect(Application::instance(), &Application::recentDisplayChanged, this, &Recent::onRecentDisplayChanged, Qt::DirectConnection);
    return true;
}

dpf::Plugin::ShutdownFlag Recent::stop()
{
    return kSync;
}

void Recent::onRecentDisplayChanged(bool enabled)
{
    if (enabled) {
        addRecentItem();
    } else {
        removeRecentItem();
    }
}

void Recent::onWindowOpened(quint64 windId)
{
    auto window = GlobalPrivate::winServ->findWindowById(windId);
    Q_ASSERT_X(window, "Recent", "Cannot find window by id");
    if (window->titleBar())
        regRecentCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Recent::regRecentCrumbToTitleBar, Qt::DirectConnection);
}

void Recent::addRecentItem()
{
    SideBar::ItemInfo item;
    item.group = SideBar::DefaultGroup::kCommon;
    item.url = RecentHelper::rootUrl();
    item.iconName = RecentHelper::icon().name();
    item.text = tr("Recent");
    item.flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    item.contextMenuCb = RecentHelper::contenxtMenuHandle;

    GlobalPrivate::sideBarService->insertItem(0, item);
}

void Recent::removeRecentItem()
{
    GlobalPrivate::sideBarService->removeItem(RecentHelper::rootUrl());
}

void Recent::regRecentCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        auto &ctx = dpfInstance.serviceContext();
        if (ctx.load(TitleBarService::name())) {
            auto titleBarServ = ctx.service<TitleBarService>(TitleBarService::name());
            TitleBar::CustomCrumbInfo info;
            info.scheme = RecentHelper::scheme();
            info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == RecentHelper::scheme(); };
            info.seperateCb = [](const QUrl &url) -> QList<TitleBar::CrumbData> {
                Q_UNUSED(url);
                return { TitleBar::CrumbData(RecentHelper::rootUrl(), tr("Recent"), RecentHelper::icon().name()) };
            };
            titleBarServ->addCustomCrumbar(info);
        }
    });
}

DPRECENT_END_NAMESPACE
