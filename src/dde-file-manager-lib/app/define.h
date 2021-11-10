/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef GLOBAL_H
#define GLOBAL_H

class DialogManager;
class GvfsMountManager;
class UDiskListener;
class QMutex;
namespace DISOMasterNS {
class DISOMaster;
}

DialogManager *getDialogManager(bool doConstruct = false);
GvfsMountManager *getGvfsMountManager(bool doConstruct = false);
UDiskListener *getUDiskListener(bool doConstruct = false);
DISOMasterNS::DISOMaster *getDISOMaster(bool doContruct = false);
QMutex *getOpticalDriveMutex();

#define bluetoothManager BluetoothManager::instance()
#define searchHistoryManager  Singleton<SearchHistroyManager>::instance()
#define bookmarkManager  Singleton<BookMarkManager>::instance()
#define fileMenuManger  Singleton<DFileMenuManager>::instance()
#define fileSignalManager Singleton<FileSignalManager>::instance()
#define dialogManager getDialogManager(true)
#define appController AppController::instance()
#define fileService DFileService::instance()
#define rootFileManager DRootFileManager::instance()
#define deviceListener getUDiskListener(true)
#define mimeAppsManager Singleton<MimesAppsManager>::instance()
#define systemPathManager Singleton<PathManager>::instance()
#define mimeTypeDisplayManager Singleton<MimeTypeDisplayManager>::instance()
#define networkManager Singleton<NetworkManager>::instance()
#define gvfsMountManager getGvfsMountManager(true)
#define secretManager Singleton<SecretManager>::instance()
#define userShareManager Singleton<UserShareManager>::instance()
#define ISOMaster getDISOMaster(true)

// begin file item global define
#define TEXT_PADDING 4
#define ICON_MODE_ICON_SPACING 5
#define COLUMU_PADDING 10
#define LEFT_PADDING 10
#define RIGHT_PADDING 10
// end

// begin file view global define
#define LIST_MODE_LEFT_MARGIN 10
#define LIST_MODE_RIGHT_MARGIN 10
// end


#define MAX_PROPERTY_DIALOG_NUMBER 16

#define DISCBURN_STAGING "discburn"
#define DISCBURN_CACHE_MID_PATH "/.cache/deepin/discburn/_dev_"

#define VAULT_CREATE "com.deepin.filemanager.daemon.VaultManager.Create"
#define VAULT_REMOVE "com.deepin.filemanager.daemon.VaultManager.Remove"

#define IPHONE_STAGING   "Apple_Inc"
#define MOBILE_ROOT_PATH "/run/user"
#define NETWORK_REDIRECT_SCHEME_EX "-all-network"

// root 用户下 挂载路径为 /.gvfs/mtp:host=", 所以这里去掉/
#define MTP_STAGING   "gvfs/mtp:host="

extern bool g_isFileDialogMode;

#endif // GLOBAL_H
