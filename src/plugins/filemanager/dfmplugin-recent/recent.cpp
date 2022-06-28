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
#include "files/recentfileinfo.h"
#include "files/recentdiriterator.h"
#include "files/recentfilewatcher.h"
#include "utils/recentmanager.h"
#include "utils/recentfileshelper.h"
#include "menus/recentmenuscene.h"
#include "events/recenteventreceiver.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/common/propertydialog/propertydialogservice.h"
#include "services/common/delegate/delegateservice.h"
#include "services/filemanager/detailspace/detailspaceservice.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"

Q_DECLARE_METATYPE(QList<QVariantMap> *);

DSC_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace dfmplugin_recent {

void Recent::initialize()
{
    UrlRoute::regScheme(RecentManager::scheme(), "/", RecentManager::icon(), true, tr("Recent"));
    //注册Scheme为"recent"的扩展的文件信息 本地默认文件的
    InfoFactory::regClass<RecentFileInfo>(RecentManager::scheme());
    WatcherFactory::regClass<RecentFileWatcher>(RecentManager::scheme());
    DirIteratorFactory::regClass<RecentDirIterator>(RecentManager::scheme());

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Recent::onWindowOpened, Qt::DirectConnection);
    connect(Application::instance(), &Application::recentDisplayChanged, this, &Recent::onRecentDisplayChanged, Qt::DirectConnection);

    followEvent();
    RecentEventReceiver::instance()->initConnect();
    RecentManager::instance();
}

bool Recent::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(RecentMenuCreator::name(), new RecentMenuCreator());

    delegateServIns->registerUrlTransform(RecentManager::scheme(), RecentManager::urlTransform);

    DetailFilterTypes filter = DetailFilterType::kFileSizeField;
    filter |= DetailFilterType::kFileChangeTImeField;
    filter |= DetailFilterType::kFileInterviewTimeField;
    DetailSpaceService::serviceInstance()->registerFilterControlField(RecentManager::scheme(), filter);

    addFileOperations();
    addDelegateSettings();

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
    auto window = FMWindowsIns.findWindowById(windId);
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
    item.flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    item.contextMenuCb = RecentManager::contenxtMenuHandle;

    RecentManager::sideBarServIns()->insertItem(0, item);
}

void Recent::removeRecentItem()
{
    RecentManager::sideBarServIns()->removeItem(RecentManager::rootUrl());
}

void Recent::followEvent()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomColumnRoles", RecentManager::instance(), &RecentManager::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomRoleDisplayName", RecentManager::instance(), &RecentManager::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FetchCustomRoleData", RecentManager::instance(), &RecentManager::customRoleData);
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_DetailViewIcon", RecentManager::instance(), &RecentManager::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", RecentManager::instance(), &RecentManager::sepateTitlebarCrumb);
}

void Recent::regRecentCrumbToTitleBar()
{
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", RecentManager::scheme(), QVariantMap {});
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
    WorkspaceService::service()->setWorkspaceMenuScene(Global::Scheme::kRecent, RecentMenuCreator::name());

    propertyServIns->registerBasicViewFiledExpand(RecentManager::propetyExtensionFunc, RecentManager::scheme());

    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->copy = [](const quint64,
                                   const QList<QUrl>,
                                   const QUrl,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags) -> JobHandlePointer {
        return {};
    };
    fileOpeationsHandle->cut = [](const quint64,
                                  const QList<QUrl>,
                                  const QUrl,
                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags) -> JobHandlePointer {
        return {};
    };

    fileOpeationsHandle->openInTerminal = [](const quint64,
                                             const QList<QUrl>,
                                             QString *) -> bool {
        return true;
    };

    fileOpeationsHandle->deletes = &RecentFilesHelper::deleteFilesHandle;
    fileOpeationsHandle->moveToTash = &RecentFilesHelper::deleteFilesHandle;
    fileOpeationsHandle->openFiles = &RecentFilesHelper::openFilesHandle;
    fileOpeationsHandle->setPermission = &RecentFilesHelper::setPermissionHandle;
    fileOpeationsHandle->writeUrlsToClipboard = &RecentFilesHelper::writeUrlToClipboardHandle;
    fileOpeationsHandle->linkFile = &RecentFilesHelper::createLinkFileHandle;

    RecentManager::fileOperationsServIns()->registerOperations(RecentManager::scheme(), fileOpeationsHandle);
}

void Recent::addDelegateSettings()
{
    DelegateService::service()->registerUrlTransform(Global::Scheme::kRecent, [](const QUrl &in) -> QUrl {
        auto out { in };
        out.setScheme(Global::Scheme::kFile);
        return out;
    });
}
}
