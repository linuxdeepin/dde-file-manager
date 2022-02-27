/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "utils/recentmanager.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

DPRECENT_BEGIN_NAMESPACE

void Recent::initialize()
{
    UrlRoute::regScheme(RecentManager::scheme(), "/", RecentManager::icon(), true, tr("Recent"));
    //注册Scheme为"recent"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<RecentFileInfo>(RecentManager::scheme());
    WacherFactory::regClass<RecentFileWatcher>(RecentManager::scheme());
    DirIteratorFactory::regClass<RecentDirIterator>(RecentManager::scheme());

    connect(RecentManager::winServIns(), &WindowsService::windowOpened, this, &Recent::onWindowOpened, Qt::DirectConnection);
    connect(&dpfInstance.listener(), &dpf::Listener::pluginsInitialized, this, &Recent::onAllPluginsInitialized, Qt::DirectConnection);
    connect(Application::instance(), &Application::recentDisplayChanged, this, &Recent::onRecentDisplayChanged, Qt::DirectConnection);

    RecentManager::instance();
}

bool Recent::start()
{

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
    auto window = RecentManager::winServIns()->findWindowById(windId);
    Q_ASSERT_X(window, "Recent", "Cannot find window by id");
    if (window->titleBar())
        regRecentCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Recent::regRecentCrumbToTitleBar, Qt::DirectConnection);
    if (window->sideBar())
        installToSideBar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, [this] { installToSideBar(); }, Qt::DirectConnection);
}

void Recent::addRecentItem()
{
    SideBar::ItemInfo item;
    item.group = SideBar::DefaultGroup::kCommon;
    item.url = RecentManager::rootUrl();
    item.iconName = RecentManager::icon().name();
    item.text = tr("Recent");
    item.flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    item.contextMenuCb = RecentManager::contenxtMenuHandle;

    RecentManager::sideBarServIns()->insertItem(0, item);
}

void Recent::removeRecentItem()
{
    RecentManager::sideBarServIns()->removeItem(RecentManager::rootUrl());
}

void Recent::regRecentCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        auto &ctx = dpfInstance.serviceContext();
        if (ctx.load(TitleBarService::name())) {
            auto titleBarServ = ctx.service<TitleBarService>(TitleBarService::name());
            TitleBar::CustomCrumbInfo info;
            info.scheme = RecentManager::scheme();
            info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == RecentManager::scheme(); };
            info.seperateCb = [](const QUrl &url) -> QList<TitleBar::CrumbData> {
                Q_UNUSED(url);
                return { TitleBar::CrumbData(RecentManager::rootUrl(), tr("Recent"), RecentManager::icon().name()) };
            };
            titleBarServ->addCustomCrumbar(info);
        }
    });
}
void Recent::onAllPluginsInitialized()
{
    addFileOperations();
}

void Recent::installToSideBar()
{
    bool showRecentEnabled = Application::instance()->genericAttribute(Application::kShowRecentFileEntry).toBool();
    if (showRecentEnabled) {
        addRecentItem();
    }
}

void Recent::addFileOperations()
{
    RecentManager::workspaceServIns()->addScheme(RecentManager::scheme());
    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &RecentManager::openFilesHandle;
    fileOpeationsHandle->writeUrlsToClipboard = &RecentManager::writeToClipBoardHandle;
    RecentManager::fileOperationsServIns()->registerOperations(RecentManager::scheme(), fileOpeationsHandle);
}
DPRECENT_END_NAMESPACE
