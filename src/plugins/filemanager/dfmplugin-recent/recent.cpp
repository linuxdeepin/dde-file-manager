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
#include "utils/recentfilehelper.h"
#include "menus/recentmenuscene.h"
#include "events/recenteventreceiver.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"

using BasicViewFieldFunc = std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &url)>;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
Q_DECLARE_METATYPE(ContextMenuCallback);
Q_DECLARE_METATYPE(QList<QVariantMap> *);
Q_DECLARE_METATYPE(QList<QUrl> *);
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QFlags<QFileDevice::Permission>)
Q_DECLARE_METATYPE(BasicViewFieldFunc)
Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(QVariant *)

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

    QStringList &&filtes { "kFileSizeField", "kFileChangeTimeField", "kFileInterviewTimeField" };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicFiledFilter_Add",
                         RecentManager::scheme(), filtes);

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", RecentManager::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", RecentManager::scheme(), RecentMenuCreator::name());

    addFileOperations();

    // events
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_SetPermission", RecentFileHelper::instance(), &RecentFileHelper::setPermissionHandle);

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
    ContextMenuCallback contextMenuCb { RecentManager::contenxtMenuHandle };

    Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable };
    QVariantMap map {
        { "Property_Key_Group", "Group_Common" },
        { "Property_Key_DisplayName", tr("Recent") },
        { "Property_Key_Icon", RecentManager::icon() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) }
    };

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Insert", 0, RecentManager::rootUrl(), map);
}

void Recent::removeRecentItem()
{
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", RecentManager::rootUrl());
}

void Recent::followEvent()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles", RecentManager::instance(), &RecentManager::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName", RecentManager::instance(), &RecentManager::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomRoleData", RecentManager::instance(), &RecentManager::customRoleData);
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch", RecentManager::instance(), &RecentManager::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", RecentManager::instance(), &RecentManager::sepateTitlebarCrumb);
    dpfHookSequence->follow("dfmplugin_utils", "hook_UrlsTransform", RecentManager::instance(), &RecentManager::urlsToLocal);

    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CutFile", RecentFileHelper::instance(), &RecentFileHelper::cutFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CopyFile", RecentFileHelper::instance(), &RecentFileHelper::copyFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MoveToTrash", RecentFileHelper::instance(), &RecentFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_DeleteFile", RecentFileHelper::instance(), &RecentFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", RecentFileHelper::instance(), &RecentFileHelper::openFileInPlugin);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_LinkFile", RecentFileHelper::instance(), &RecentFileHelper::linkFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard", RecentFileHelper::instance(), &RecentFileHelper::writeUrlsToClipboard);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenInTerminal", RecentFileHelper::instance(), &RecentFileHelper::openFileInTerminal);
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
    BasicViewFieldFunc func { RecentManager::propetyExtensionFunc };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_BasicViewExtension_Register",
                         func, RecentManager::scheme());
}
}
