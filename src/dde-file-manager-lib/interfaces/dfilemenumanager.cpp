/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "dfilemenumanager.h"
#include "dfmglobal.h"
#include "app/define.h"
#include "dfmevent.h"
#include "dfilemenu.h"
#include "dfileservices.h"
#include "dfmeventdispatcher.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "controllers/vaultcontroller.h"
#include "controllers/appcontroller.h"
#include "controllers/trashmanager.h"
#include "models/desktopfileinfo.h"
#include "shutil/mimetypedisplaymanager.h"
#include "singleton.h"
#include "views/windowmanager.h"
#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "shutil/danythingmonitorfilter.h"
#include "controllers/pathmanager.h"
#include "plugins/pluginmanager.h"
#include "dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h"
#include "dfmstandardpaths.h"
#include "deviceinfo/udisklistener.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "views/dtagactionwidget.h"
#include "plugins/dfmadditionalmenu.h"
#include "models/dfmrootfileinfo.h"
#include "bluetooth/bluetoothmanager.h"
#include "bluetooth/bluetoothmodel.h"
#include "io/dstorageinfo.h"
#include "vault/vaultlockmanager.h"
#include "vault/vaulthelper.h"
#include "app/filesignalmanager.h"
#include "views/dfilemanagerwindow.h"
#include "customization/dcustomactionbuilder.h"
#include "customization/dcustomactionparser.h"
#include "gvfs/gvfsmountmanager.h"

#include <DSysInfo>

#include <QMetaObject>
#include <QMetaEnum>
#include <QMenu>
#include <QTextCodec>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QQueue>
#include <QDebug>
#include <QPushButton>
#include <QWidgetAction>

#include <dgiosettings.h>
#include <unistd.h>


//fix:临时获取光盘刻录前临时的缓存地址路径，便于以后直接获取使用

namespace DFileMenuData {
static QMap<MenuAction, QString> actionKeys;
static QMap<MenuAction, QIcon> actionIcons;
static QMap<MenuAction, QAction *> actions;
static QMap<const QAction *, MenuAction> actionToMenuAction;
static QMap<MenuAction, QString> actionIDs;
static QVector<MenuAction> sortActionTypes;
static QSet<MenuAction> whitelist;
static QSet<MenuAction> blacklist;
static QQueue<MenuAction> availableUserActionQueue;
static DFMAdditionalMenu *additionalMenu;
static DCustomActionParser *customMenuParser = nullptr;

void initData();
void initActions();
void clearActions();

MenuAction takeAvailableUserActionType()
{
    if (availableUserActionQueue.isEmpty()) {
        availableUserActionQueue.append(MenuAction(MenuAction::UserMenuAction + 1));

        return MenuAction::UserMenuAction;
    }

    MenuAction type = availableUserActionQueue.dequeue();

    if (availableUserActionQueue.isEmpty()) {
        availableUserActionQueue.append(MenuAction(type + 1));
    }

    return type;
}
void recycleUserActionType(MenuAction type)
{
    availableUserActionQueue.prepend(type);
    QAction *action = actions.take(type);

    if (action) {
        actionToMenuAction.remove(action);
    }
    if (DFileMenuManager::needDeleteAction())
        delete action;
}
}

DFileMenu *DFileMenuManager::createDefaultBookMarkMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(2);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab
               << MenuAction::Property;

    return genereteMenuByKeys(actionKeys, disableList);
}

DFileMenu *DFileMenuManager::createUserShareMarkMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;

    actionKeys.reserve(1);

    actionKeys << MenuAction::OpenInNewWindow
               << MenuAction::OpenInNewTab;

    DFileMenu *menu = genereteMenuByKeys(actionKeys, disableList);
    menu->setAccessibleInfo(AC_FILE_MENU_USER_SHARE);
    return menu;
}

DFileMenu *DFileMenuManager::createToolBarSettingsMenu(const QSet<MenuAction> &disableList)
{
    QVector<MenuAction> actionKeys;
    QMap<MenuAction, QVector<MenuAction> >  subMenuKeys;

    actionKeys.reserve(5);

    actionKeys << MenuAction::NewWindow
               << MenuAction::Separator
               << MenuAction::ConnectToServer
               << MenuAction::SetUserSharePassword
               << MenuAction::Settings;

    DFileMenu *menu = genereteMenuByKeys(actionKeys, disableList, false, subMenuKeys, false);
    menu->setAccessibleInfo(AC_FILE_MENU_TOOLBAR_SEETINGS);
    return menu;
}

DFileMenu *DFileMenuManager::createNormalMenu(const DUrl &currentUrl, const DUrlList &urlList, QSet<MenuAction> disableList, QSet<MenuAction> unusedList, int windowId, bool onDesktop)
{
    // remove compress/decompress action
    unusedList << MenuAction::Compress << MenuAction::Decompress << MenuAction::DecompressHere;

    DAbstractFileInfoPointer info = fileService->createFileInfo(Q_NULLPTR, currentUrl);
    DFileMenu *menu = Q_NULLPTR;
    if (!info) {
        return menu;
    }

    //! urlList中有保险箱的DUrl需要进行转换否则会出现报错或功能不可用
    DUrlList urls = urlList;
    for (int i = 0; i < urlList.size(); ++i) {
        if (urlList[i].isVaultFile())
            urls[i] = VaultController::vaultToLocalUrl(urlList[i]);
    }

    auto dirInUrls = [](const DUrlList &urls) {
        for (const auto &url: urls) {
            auto fileInfo = fileService->createFileInfo(nullptr, url);
            if (fileInfo && fileInfo->isDir())
                return true;
        }
        return false;
    };

    DUrlList redirectedUrlList;
    if (urls.length() == 1) {
        QVector<MenuAction> actions = info->menuActionList(DAbstractFileInfo::SingleFile);
        //修改在挂载的文件下面，不能删除，但是显示了删除
        //判定逻辑，如果是挂载盘，并且这个文件是对应的不是一个虚拟的项（表示有实体），那么这个文件就有彻底删除权限MenuAction::CompleteDeletion
        //但是这个文件canrename权限为false，以前的判断输出就是，不能MenuAction::Cut，MenuAction::Rename
        // MenuAction::delete
        //当彻底删除时，判断canrename权限，true可以删除，false不可以删除
        //所以这里要把MenuAction::CompleteDeletion权限不能用
        if (FileUtils::isGvfsMountFile(info->absoluteFilePath()) && !info->canRename()) {
            disableList << MenuAction::CompleteDeletion;
        }
        if (info->isDir()) { //判断是否目录
            if (info->ownerId() != getuid() && !DFMGlobal::isRootUser()) { //判断文件属主与进程属主是否相同，排除进程属主为根用户情况
                disableList << MenuAction::UnShare << MenuAction::Share; //设置取消共享、取消共享不可选
            }
        }
        foreach (MenuAction action, unusedList) {
            if (actions.contains(action)) {
                actions.remove(actions.indexOf(action));
            }
        }

        if (actions.isEmpty()) {
            return menu;
        }

        const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();
        disableList += DFileMenuManager::getDisableActionList(urls);
        const bool &tabAddable = WindowManager::tabAddableByWinId(windowId);
        if (!tabAddable) {
            disableList << MenuAction::OpenInNewTab;
        }

        ///###: tag protocol.
        if (!DFileMenuManager::whetherShowTagActions(urls)) {
            actions.removeAll(MenuAction::TagInfo);
            actions.removeAll(MenuAction::TagFilesUseColor);
        }

        // sp3 feature: root用户, 服务器版本用户, 非开发者模式均不需要以管理员身份打开的功能
        if (DFMGlobal::isRootUser() || DFMGlobal::isServerSys() || !DFMGlobal::isDeveloperMode()) {
            actions.removeAll(MenuAction::OpenAsAdmin);
        }

        menu = DFileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    } else {
        bool isSystemPathIncluded = false;
        bool isAllCompressedFiles = true;
//        QMimeType fileMimeType;
        QStringList supportedMimeTypes;
        bool mime_displayOpenWith = true;
        //fix bug 35546 【文件管理器】【5.1.2.2-1】【sp2】【sp1】选择多个只读文件夹，删除按钮没有置灰
        //只判断当前选中的文件夹是否是只读文件夹
        if (!info->isWritable() && !info->isFile() && !info->isSymLink()) {
            disableList << MenuAction::Delete;
        }

#if 1   //!fix bug#29264.部分格式的文件在上面的MimesAppsManager::getDefaultAppDesktopFileByMimeType
        //!调用中可以找到app打开，但是在后面的判断是由于该app的supportedMimeTypes中并没有本文件的mimeTypeList支持
        //!导致了无法匹配，matched为false。因此这里加做一次判断，如果本次文件的后缀名与与上面查询app的文件后缀名一样，则直接匹配

        //获取当前文件的app
        if (supportedMimeTypes.isEmpty()) {
            QMimeType fileMimeType = info->mimeType();
            QString defaultAppDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(fileMimeType.name());
            QSettings desktopFile(defaultAppDesktopFile, QSettings::IniFormat);
            desktopFile.setIniCodec("UTF-8");
            Properties mimeTypeList(defaultAppDesktopFile, "Desktop Entry");
            supportedMimeTypes = mimeTypeList.value("MimeType").toString().split(';');
            supportedMimeTypes.removeAll("");
        }

        redirectedUrlList.clear();
        foreach (DUrl url, urls) {
            const DAbstractFileInfoPointer &file_info = fileService->createFileInfo(Q_NULLPTR, url);
            // fix bug202007010011 优化文件判断效率，提升右键菜单响应速度
            auto redirectedUrl = file_info->redirectedFileUrl();
            if (redirectedUrl.isValid()) {
                redirectedUrlList << redirectedUrl;
            }

            if (!FileUtils::isArchive(url.path())) {
                isAllCompressedFiles = false;
            }

            if (systemPathManager->isSystemPath(file_info->fileUrl().toLocalFile())) {
                isSystemPathIncluded = true;
            }

            if (!mime_displayOpenWith) {
                continue;
            }

            QStringList mimeTypeList = { file_info->mimeType().name() };
            mimeTypeList.append(file_info->mimeType().parentMimeTypes());
            bool matched = false;

            //后缀名相同直接匹配
            if (file_info->suffix() == info->suffix()) {
                matched = true;
            } else {
                for (const QString &oneMimeType : mimeTypeList) {
                    if (supportedMimeTypes.contains(oneMimeType)) {
                        matched = true;
                        break;
                    }
                }
            }

            if (!matched) {
                mime_displayOpenWith = false;
                disableList << MenuAction::Open << MenuAction::OpenWith;
                break;
            }
        }
#else  //原来的实现
        foreach (DUrl url, urls) {
            const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(Q_NULLPTR, url);

            if (!FileUtils::isArchive(url.path())) {
                isAllCompressedFiles = false;
            }

            if (systemPathManager->isSystemPath(fileInfo->fileUrl().toLocalFile())) {
                isSystemPathIncluded = true;
            }

            if (!mime_displayOpenWith) {
                continue;
            }

            if (supportedMimeTypes.isEmpty()) {
                QMimeType fileMimeType = fileInfo->mimeType();
                QString defaultAppDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(fileMimeType.name());
                QSettings desktopFile(defaultAppDesktopFile, QSettings::IniFormat);
                desktopFile.setIniCodec("UTF-8");
                Properties mimeTypeList(defaultAppDesktopFile, "Desktop Entry");
                supportedMimeTypes = mimeTypeList.value("MimeType").toString().split(';');
                supportedMimeTypes.removeAll({});
            } else {
                QStringList mimeTypeList = { fileInfo->mimeType().name() };
                mimeTypeList.append(fileInfo->mimeType().parentMimeTypes());
                bool matched = false;
                for (const QString &oneMimeType : mimeTypeList) {
                    if (supportedMimeTypes.contains(oneMimeType)) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    mime_displayOpenWith = false;
                    disableList << MenuAction::Open << MenuAction::OpenWith;
                }
            }
        }
#endif
        QVector<MenuAction> actions;

        if (isSystemPathIncluded) {
            actions = info->menuActionList(DAbstractFileInfo::MultiFilesSystemPathIncluded);
        } else {
            actions = info->menuActionList(DAbstractFileInfo::MultiFiles);
        }

        if (actions.isEmpty()) {
            return menu;
        }

        if (isAllCompressedFiles) {
            int index = actions.indexOf(MenuAction::Compress);
            actions.insert(index + 1, MenuAction::Decompress);
            actions.insert(index + 2, MenuAction::DecompressHere);
        }

        const QMap<MenuAction, QVector<MenuAction> > &subActions  = info->subMenuActionList();
        disableList += DFileMenuManager::getDisableActionList(urls);
        const bool &tabAddable = WindowManager::tabAddableByWinId(windowId);
        if (!tabAddable) {
            disableList << MenuAction::OpenInNewTab;
        }

        foreach (MenuAction action, unusedList) {
            if (actions.contains(action)) {
                actions.remove(actions.indexOf(action));
            }
        }

        ///###: tag protocol.
        if (!DFileMenuManager::whetherShowTagActions(urls)) {
            actions.removeAll(MenuAction::TagInfo);
            actions.removeAll(MenuAction::TagFilesUseColor);
        }

        menu = DFileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    }

    QAction *openWithAction = menu->actionAt(DFileMenuManager::getActionString(DFMGlobal::OpenWith));
    DFileMenu *openWithMenu = openWithAction ? qobject_cast<DFileMenu *>(openWithAction->menu()) : Q_NULLPTR;

    if (openWithMenu && openWithMenu->isEnabled()) {
        QStringList recommendApps = mimeAppsManager->getRecommendedApps(info->redirectedFileUrl());
        recommendApps.removeOne("/usr/share/applications/dde-open.desktop"); //在右键菜单打开方式中屏蔽/usr/share/applications/dde-open.desktop，在此处更改不影响打开效果
//        bug 20275 【桌面社区版版V20】【beta】【DDE】【 桌面】选中图片右键打开方式多了“ImageMagick”需隐藏
        recommendApps.removeOne("/usr/share/applications/display-im6.q16.desktop"); //按产品经理要求屏蔽咯
        recommendApps.removeOne("/usr/share/applications/display-im6.q16hdri.desktop"); //按产品经理要求屏蔽咯
        foreach (QString app, recommendApps) {
//            const DesktopFile& df = mimeAppsManager->DesktopObjs.value(app);
            //ignore no show apps
//            if(df.getNoShow())
//                continue;
            DesktopFile desktopFile(app);
            QAction *action = new QAction(desktopFile.getDisplayName(), openWithMenu);
            action->setIcon(FileUtils::searchAppIcon(desktopFile));
            action->setProperty("app", app);
            if (urls.length() == 1) {
                action->setProperty("url", QVariant::fromValue(info->redirectedFileUrl()));
            } else {
#if 0       // fix bug202007010011 优化文件判断效率，提升右键菜单响应速度
                DUrlList redirectedUrlList;
                for (auto url : urls) {
                    DAbstractFileInfoPointer info = fileService->createFileInfo(Q_NULLPTR, url);
                    auto redirectedUrl = info->redirectedFileUrl();
                    if (redirectedUrl.isValid()) {
                        redirectedUrlList << redirectedUrl;
                    }
                }
#endif
                action->setProperty("urls", QVariant::fromValue(redirectedUrlList));
            }
            openWithMenu->addAction(action);
            connect(action, &QAction::triggered, appController, &AppController::actionOpenFileByApp);
        }

        QAction *action = new QAction(fileMenuManger->getActionString(MenuAction::OpenWithCustom), openWithMenu);
        action->setData((int)MenuAction::OpenWithCustom);
        openWithMenu->addAction(action);
        DFileMenuData::actions[MenuAction::OpenWithCustom] = action;
        DFileMenuData::actionToMenuAction[action] = MenuAction::OpenWithCustom;
    }

    if (deviceListener->isMountedRemovableDiskExits()
            || bluetoothManager->hasAdapter()) {
        QAction *sendToMountedRemovableDiskAction = menu->actionAt(DFileMenuManager::getActionString(DFMGlobal::SendToRemovableDisk));
        if (currentUrl.path().contains("/dev/sr")
                || (currentUrl.scheme() == SEARCH_SCHEME && currentUrl.query().contains("/dev/sr"))) // 禁用光盘搜索列表中的发送到选项
            menu->removeAction(sendToMountedRemovableDiskAction);
        else {
            DFileMenu *sendToMountedRemovableDiskMenu = sendToMountedRemovableDiskAction ? qobject_cast<DFileMenu *>(sendToMountedRemovableDiskAction->menu()) : Q_NULLPTR;
            if (sendToMountedRemovableDiskMenu) {
                // 如果有蓝牙设备并且当前文件不在保险箱内
                if (BluetoothManager::bluetoothSendEnable() // is system disabled "sending via bluetooth"
                        && bluetoothManager->hasAdapter()
                        && !VaultController::isVaultFile(currentUrl.toLocalFile())) {
                    QAction *sendToBluetooth = new QAction(DFileMenuManager::getActionString(DFMGlobal::SendToBluetooth), sendToMountedRemovableDiskMenu);
                    sendToBluetooth->setProperty("urlList", DUrl::toStringList(urls));
                    sendToMountedRemovableDiskMenu->addAction(sendToBluetooth);
                    connect(sendToBluetooth, &QAction::triggered, appController, &AppController::actionSendToBluetooth);
                    if (dirInUrls(urls))
                        sendToBluetooth->setEnabled(false);
                }

                foreach (UDiskDeviceInfoPointer pDeviceinfo, deviceListener->getCanSendDisksByUrl(currentUrl.toLocalFile()).values()) {
                    //fix:临时获取光盘刻录前临时的缓存地址路径，便于以后直接获取使用 id="/dev/sr1" -> tempId="sr1"
                    QString tempId = pDeviceinfo->getDiskInfo().id().mid(5);
                    DUrl gvfsmpurl;
                    gvfsmpurl.setScheme(DFMROOT_SCHEME);
                    gvfsmpurl.setPath("/" + QUrl::toPercentEncoding(tempId) + "." SUFFIX_UDISKS);

                    DAbstractFileInfoPointer fp(new DFMRootFileInfo(gvfsmpurl)); // 通过DFMRootFileInfo 拿到与桌面显示一致的 名字

                    qDebug() << "add send action: [ diskinfoname:" << pDeviceinfo->getDiskInfo().name() << " to RootFileInfo: " << fp->fileDisplayName();

                    QAction *action = new QAction(fp->fileDisplayName(), sendToMountedRemovableDiskMenu);
                    action->setProperty("mounted_root_uri", pDeviceinfo->getDiskInfo().mounted_root_uri());
                    action->setProperty("urlList", DUrl::toStringList(urls));
                    action->setProperty("blkDevice", tempId);

                    // 禁用发送到列表中的本设备项
                    if (urls.count() > 0) {
                        DUrl durl = urls[0];
                        if (durl.path().contains(pDeviceinfo->getDiskInfo().id()))
                            action->setEnabled(false);
                    }

                    sendToMountedRemovableDiskMenu->addAction(action);
                    connect(action, &QAction::triggered, appController, &AppController::actionSendToRemovableDisk, Qt::QueuedConnection); //改为队列，防止exec无法退出，关联bug#25613
                }
                // 如果子菜单中没有内容，移除父菜单
                if(sendToMountedRemovableDiskMenu->actions().count() < 1) {
                    menu->removeAction(sendToMountedRemovableDiskAction);
                }
            }

        }
    }
    if (QAction *stageAction = menu->actionAt(DFileMenuManager::getActionString(DFMGlobal::StageFileForBurning))) {

        QMap<QString, DUrl> diskUrlsMap;
        QStringList odrv;
        DDiskManager diskm;
        for (auto &blks : diskm.blockDevices({})) {
            QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
            QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
            if (drv->mediaCompatibility().join(' ').contains("_r")) {
                if ((currentUrl.scheme() == BURN_SCHEME && QString(blk->device()) == currentUrl.burnDestDevice()) || odrv.contains(drv->path())
                        || (currentUrl.scheme() == SEARCH_SCHEME && currentUrl.query().contains("/dev/sr"))) { // 禁用光盘搜索列表中的“添加到光盘刻录”选项
                    continue;
                }
                DUrl rootUrl(DFMROOT_ROOT + QString(blk->device()).mid(QString("/dev/").length()) + ".localdisk");
                odrv.push_back(drv->path());
                diskUrlsMap[drv->path()] = rootUrl;
            }
        }

        if (odrv.empty()) {
            stageAction->setEnabled(false);
        } else if (odrv.size() == 1) {
            QScopedPointer<DDiskDevice> dev(DDiskManager::createDiskDevice(odrv.front()));
            QString devID = dev->id();
            stageAction->setProperty("dest_drive", odrv.front());
            stageAction->setProperty("urlList", DUrl::toStringList(urls));
            connect(stageAction, &QAction::triggered, appController, &AppController::actionStageFileForBurning, Qt::UniqueConnection);
            if (DFileMenu *stageMenu = qobject_cast<DFileMenu *>(stageAction->menu())) {
                stageAction->setMenu(nullptr);
                delete stageMenu;
            }
        } else {
            if (DFileMenu *stageMenu = qobject_cast<DFileMenu *>(stageAction->menu())) {
                for (auto &devs : odrv) {
                    QScopedPointer<DDiskDevice> dev(DDiskManager::createDiskDevice(devs));
                    //右键菜单 发送文件到光驱 选项需要展示光驱目录的displayName
                    DAbstractFileInfoPointer fi;
                    QString devName(dev->id());
                    if (diskUrlsMap.contains(devs)) {
                        fi = fileService->createFileInfo(nullptr, diskUrlsMap[devs]);
                        if (fi) {
                            if (!fi->fileDisplayName().isEmpty()) {
                                devName = fi->fileDisplayName();
                            }
                        }
                    }
                    QAction *action = new QAction(devName, stageMenu);
                    action->setProperty("dest_drive", devs);
                    action->setProperty("urlList", DUrl::toStringList(urls));
                    stageMenu->addAction(action);
                    connect(action, &QAction::triggered, appController, &AppController::actionStageFileForBurning, Qt::UniqueConnection);
                }
            }
        }
    }

    if (currentUrl == DesktopFileInfo::computerDesktopFileUrl()
            || currentUrl == DesktopFileInfo::trashDesktopFileUrl()
            || currentUrl == DesktopFileInfo::homeDesktopFileUrl()) {
        return menu;
    }
    // 保险箱不需要其它非文管的插件菜单
    if(!VaultController::isVaultFile(currentUrl.toLocalFile())) {
        loadNormalPluginMenu(menu, urls, currentUrl, onDesktop);
    }
    // stop loading Extension menus from json files
    //loadNormalExtensionMenu(menu, urlList, currentUrl);

    return menu;
}

DFileMenu *DFileMenuManager::createVaultMenu(QWidget *topWidget, const QObject *sender)
{
    DFileMenu *menu = nullptr;

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topWidget);
    VaultController *controller = VaultController::ins();

    VaultController::VaultState vaultState = controller->state();

    DUrl durl = controller->vaultToLocalUrl(controller->makeVaultUrl());
    durl.setScheme(DFMVAULT_SCHEME);
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, durl);

    QSet<MenuAction> disableList;
    if (!VaultLockManager::getInstance().isValid()) {
        disableList << MenuAction::FiveMinutes
                    << MenuAction::TenMinutes
                    << MenuAction::TwentyMinutes;
    }

    menu = DFileMenuManager::genereteMenuByKeys(infoPointer->menuActionList(), disableList, true, infoPointer->subMenuActionList(), false);
    menu->setEventData(DUrl(), {durl}, WindowManager::getWindowId(wnd), sender);
    menu->setAccessibleInfo(AC_FILE_MENU_VAULT);

    auto lockNow = [](DFileManagerWindow * wnd)->bool {
        //! Is there a vault task, top it if exist.
        if (!VaultHelper::topVaultTasks())
        {
            emit fileSignalManager->requestCloseAllTabOfVault(wnd->windowId());
            VaultController::ins()->lockVault();
        }

        return true;
    };

    auto autoLock = [](int lockState)->bool {
        return VaultLockManager::getInstance().autoLock(static_cast<VaultLockManager::AutoLockState>(lockState));
    };

    auto showView = [&](QWidget * wndPtr, QString host) {
        DFileManagerWindow *file_mng_wnd = qobject_cast<DFileManagerWindow *>(wndPtr);
        file_mng_wnd->cd(VaultController::makeVaultUrl("/", host));
    };

    if (vaultState == VaultController::Unlocked) {

        //! 立即上锁
        QAction *action = DFileMenuManager::getAction(MenuAction::LockNow);
        QObject::connect(action, &QAction::triggered, action, [ &, wnd]() {
            lockNow(wnd);
        });

        //! 自动上锁
        VaultLockManager::AutoLockState lockState = VaultLockManager::getInstance().autoLockState();

        QAction *actionNever = DFileMenuManager::getAction(MenuAction::Never);
        QObject::connect(actionNever, &QAction::triggered, actionNever, [&]() {
            autoLock(VaultLockManager::Never);
        });
        actionNever->setCheckable(true);
        actionNever->setChecked(lockState == VaultLockManager::Never ? true : false);

        QAction *actionFiveMins = DFileMenuManager::getAction(MenuAction::FiveMinutes);
        QObject::connect(actionFiveMins, &QAction::triggered, actionFiveMins, [&]() {
            autoLock(VaultLockManager::FiveMinutes);
        });
        actionFiveMins->setCheckable(true);
        actionFiveMins->setChecked(lockState == VaultLockManager::FiveMinutes ? true : false);

        QAction *actionTenMins = DFileMenuManager::getAction(MenuAction::TenMinutes);
        QObject::connect(actionTenMins, &QAction::triggered, actionTenMins, [&]() {
            autoLock(VaultLockManager::TenMinutes);
        });
        actionTenMins->setCheckable(true);
        actionTenMins->setChecked(lockState == VaultLockManager::TenMinutes ? true : false);

        QAction *actionTwentyMins = DFileMenuManager::getAction(MenuAction::TwentyMinutes);
        QObject::connect(actionTwentyMins, &QAction::triggered, actionTwentyMins, [&]() {
            autoLock(VaultLockManager::TwentyMinutes);
        });
        actionTwentyMins->setCheckable(true);
        actionTwentyMins->setChecked(lockState == VaultLockManager::TwentyMinutes ? true : false);

        //! 删除保险柜
        action = DFileMenuManager::getAction(MenuAction::DeleteVault);
        QObject::connect(action, &QAction::triggered, action, [ &, topWidget]() {
            showView(topWidget, "delete");
        });
    } else if (vaultState == VaultController::Encrypted) {

        //! 解锁
        QAction *action = DFileMenuManager::getAction(MenuAction::UnLock);
        QObject::connect(action, &QAction::triggered, action, [ &, topWidget]() {
            showView(topWidget, "unlock");
        });

        //! 使用恢复凭证
        action = DFileMenuManager::getAction(MenuAction::UnLockByKey);
        QObject::connect(action, &QAction::triggered, action, [ &, topWidget]() {
            showView(topWidget, "certificate");
        });
    }

    return menu;
}

QList<QAction *> DFileMenuManager::loadNormalPluginMenu(DFileMenu *menu, const DUrlList &urlList, const DUrl &currentUrl, bool onDesktop)
{
    qDebug() << "load normal plugin menu";
    QStringList files;
    foreach (DUrl url, urlList) {
        files << url.toString();
    }

    // 对menu->actions()增加判空，避免ut崩溃(临时方案，后续需要结合该函数的使用情况完成为空情况的处理)
    QList<QAction *> actions;
    if (menu->actions().isEmpty())
        return actions;

    QAction *lastAction = menu->actions().last();
    if (lastAction->isSeparator()) {
        lastAction = menu->actionAt(menu->actions().count() - 2);
    }

    if (DFileMenuData::additionalMenu) {
        actions = DFileMenuData::additionalMenu->actions(files, currentUrl.toString(), onDesktop);
    }
    foreach (QAction *action, actions) {
        menu->insertAction(lastAction, action);
    }

    menu->insertSeparator(lastAction);
    return actions;
}

QList<QAction *> DFileMenuManager::loadEmptyAreaPluginMenu(DFileMenu *menu, const DUrl &currentUrl, bool onDesktop)
{
    qDebug() << "load empty area plugin menu";
    QList<QAction *> actions;
    // 对menu->actions()增加为空判断(临时方案，后续需要结合该函数的使用情况完成为空情况的处理)
    if (menu->actions().isEmpty())
        return actions;
    QAction *lastAction = menu->actions().last();
    if (lastAction->isSeparator()) {
        lastAction = menu->actionAt(menu->actions().count() - 2);
    }

    if (DFileMenuData::additionalMenu) {
        actions = DFileMenuData::additionalMenu->actions({}, currentUrl.toString(), onDesktop);
    }

    for (QAction *action : actions) {
        menu->insertAction(lastAction, action);
    }

    menu->insertSeparator(lastAction);
    return actions;
}

QAction *DFileMenuManager::getAction(MenuAction action)
{
    return DFileMenuData::actions.value(action);
}

QString DFileMenuManager::getActionText(MenuAction action)
{
    return DFileMenuData::actionKeys.value(action);
}

QSet<MenuAction> DFileMenuManager::getDisableActionList(const DUrl &fileUrl)
{
    DUrlList list;

    list << fileUrl;

    return getDisableActionList(list);
}

QSet<MenuAction> DFileMenuManager::getDisableActionList(const DUrlList &urlList)
{
    QSet<MenuAction> disableList;

    for (const DUrl &file_url : urlList) {
        DUrl durl = file_url;
        if (VaultController::isVaultFile(durl.path())) {
            durl = VaultController::localUrlToVault(file_url);
        }
        const DAbstractFileInfoPointer &file_info = fileService->createFileInfo(Q_NULLPTR, durl);

        if (file_info) {
            disableList += file_info->disableMenuActionList();
        }
    }

    if (DFMGlobal::instance()->clipboardAction() == DFMGlobal::UnknowAction) {
        disableList << MenuAction::Paste;
    }

    return disableList;
}

DFileMenuManager::DFileMenuManager()
{
    qRegisterMetaType<QMap<QString, QString>>(QT_STRINGIFY(QMap<QString, QString>));
    qRegisterMetaType<QList<QUrl>>(QT_STRINGIFY(QList<QUrl>));
}

DFileMenuManager::~DFileMenuManager()
{
    if (DFileMenuData::additionalMenu) {
        DFileMenuData::additionalMenu->deleteLater();
        DFileMenuData::additionalMenu = nullptr;
    }

    if (DFileMenuData::customMenuParser) {
        DFileMenuData::customMenuParser->deleteLater();
        DFileMenuData::customMenuParser = nullptr;
    }
}

void DFileMenuData::initData()
{
    actionKeys[MenuAction::Open] = QObject::tr("Open");
    actionKeys[MenuAction::OpenInNewWindow] = QObject::tr("Open in new window");
    actionKeys[MenuAction::OpenInNewTab] = QObject::tr("Open in new tab");
    actionKeys[MenuAction::OpenDisk] = QObject::tr("Open");
    actionKeys[MenuAction::OpenDiskInNewWindow] = QObject::tr("Open in new window");
    actionKeys[MenuAction::OpenDiskInNewTab] = QObject::tr("Open in new tab");
    actionKeys[MenuAction::OpenAsAdmin] = QObject::tr("Open as administrator");
    actionKeys[MenuAction::OpenWith] = QObject::tr("Open with");
    actionKeys[MenuAction::OpenWithCustom] = QObject::tr("Select default program");
    actionKeys[MenuAction::OpenFileLocation] = QObject::tr("Open file location");
    actionKeys[MenuAction::Compress] = QObject::tr("Compress");
    actionKeys[MenuAction::Decompress] = QObject::tr("Extract");
    actionKeys[MenuAction::DecompressHere] = QObject::tr("Extract here");
    actionKeys[MenuAction::Cut] = QObject::tr("Cut");
    actionKeys[MenuAction::Copy] = QObject::tr("Copy");
    actionKeys[MenuAction::Paste] = QObject::tr("Paste");
    actionKeys[MenuAction::Rename] = QObject::tr("Rename");
    actionKeys[MenuAction::BookmarkRename] = QObject::tr("Rename");
    actionKeys[MenuAction::BookmarkRemove] = QObject::tr("Remove bookmark");
    actionKeys[MenuAction::CreateSymlink] = QObject::tr("Create link");
    actionKeys[MenuAction::SendToDesktop] = QObject::tr("Send to desktop");
    actionKeys[MenuAction::SendToRemovableDisk] = QObject::tr("Send to");
    actionKeys[MenuAction::SendToBluetooth] = QObject::tr("Bluetooth");
    actionKeys[MenuAction::AddToBookMark] = QObject::tr("Add to bookmark");
    actionKeys[MenuAction::Delete] = QObject::tr("Delete");
    actionKeys[MenuAction::CompleteDeletion] = QObject::tr("Delete");
    actionKeys[MenuAction::Property] = QObject::tr("Properties");

    actionKeys[MenuAction::NewFolder] = QObject::tr("New folder");
    actionKeys[MenuAction::NewWindow] = QObject::tr("New window");
    actionKeys[MenuAction::SelectAll] = QObject::tr("Select all");
    actionKeys[MenuAction::ClearRecent] = QObject::tr("Clear recent history");
    actionKeys[MenuAction::ClearTrash] = QObject::tr("Empty Trash");
    actionKeys[MenuAction::DisplayAs] = QObject::tr("Display as");
    actionKeys[MenuAction::SortBy] = QObject::tr("Sort by");
    actionKeys[MenuAction::NewDocument] = QObject::tr("New document");
    actionKeys[MenuAction::NewWord] = QObject::tr("Office Text");
    actionKeys[MenuAction::NewExcel] = QObject::tr("Spreadsheets");
    actionKeys[MenuAction::NewPowerpoint] = QObject::tr("Presentation");
    actionKeys[MenuAction::NewText] = QObject::tr("Plain Text");
    actionKeys[MenuAction::OpenInTerminal] = QObject::tr("Open in terminal");
    actionKeys[MenuAction::Restore] = QObject::tr("Restore");
    actionKeys[MenuAction::RestoreAll] = QObject::tr("Restore all");
    actionKeys[MenuAction::Mount] = QObject::tr("Mount");
    actionKeys[MenuAction::Unmount] = QObject::tr("Unmount");
    actionKeys[MenuAction::Eject] = QObject::tr("Eject");
    actionKeys[MenuAction::SafelyRemoveDrive] = QObject::tr("Safely Remove");
    actionKeys[MenuAction::RemoveFromRecent] = QObject::tr("Remove");
    actionKeys[MenuAction::Name] = QObject::tr("Name");
    actionKeys[MenuAction::Size] = QObject::tr("Size");
    actionKeys[MenuAction::Type] = QObject::tr("Type");
    actionKeys[MenuAction::CreatedDate] = QObject::tr("Time created");
    actionKeys[MenuAction::LastModifiedDate] = QObject::tr("Time modified");
    actionKeys[MenuAction::LastRead] = qApp->translate("DFileSystemModel", "Last access");
    actionKeys[MenuAction::Settings] = QObject::tr("Settings");
    actionKeys[MenuAction::Exit] = QObject::tr("Exit");
    actionKeys[MenuAction::IconView] = QObject::tr("Icon");
    actionKeys[MenuAction::ListView] = QObject::tr("List");
    actionKeys[MenuAction::ExtendView] = QObject::tr("Extend");
    actionKeys[MenuAction::SetAsWallpaper] = QObject::tr("Set as wallpaper");
    actionKeys[MenuAction::ForgetPassword] = QObject::tr("Clear saved password and unmount");
    actionKeys[MenuAction::DeletionDate] = QObject::tr("Time deleted");
    actionKeys[MenuAction::SourcePath] = QObject::tr("Source path");
    actionKeys[MenuAction::AbsolutePath] = QObject::tr("Path");
    actionKeys[MenuAction::Share] = QObject::tr("Share folder");
    actionKeys[MenuAction::UnShare] = QObject::tr("Cancel sharing");
    actionKeys[MenuAction::Vault] = QObject::tr("File Vault");
    actionKeys[MenuAction::ConnectToServer] = QObject::tr("Connect to Server");
    actionKeys[MenuAction::SetUserSharePassword] = QObject::tr("Set share password");
    actionKeys[MenuAction::FormatDevice] = QObject::tr("Format");
    actionKeys[MenuAction::OpticalBlank] = QObject::tr("Erase");

    ///###: tag protocol.
    actionKeys[MenuAction::TagInfo] = QObject::tr("Tag information");
    actionKeys[MenuAction::TagFilesUseColor] = QString{"Add color tags"};
    actionKeys[MenuAction::DeleteTags] = QObject::tr("Delete");
    actionKeys[MenuAction::ChangeTagColor] = QString{"Change color of present tag"};
    actionKeys[MenuAction::RenameTag] = QObject::tr("Rename");

    actionKeys[MenuAction::MountImage] = QObject::tr("Mount");
    //fix: 调整右键菜单中的"刻录"为"添加至光盘刻录"
    //actionKeys[MenuAction::StageFileForBurning] = QObject::tr("Burn");
    actionKeys[MenuAction::StageFileForBurning] = QObject::tr("Add to disc");

    // Vault
    actionKeys[MenuAction::LockNow] = QObject::tr("Lock");
    actionKeys[MenuAction::AutoLock] = QObject::tr("Auto lock");
    actionKeys[MenuAction::Never] = QObject::tr("Never");
    actionKeys[MenuAction::FiveMinutes] = QObject::tr("5 minutes");
    actionKeys[MenuAction::TenMinutes] = QObject::tr("10 minutes");
    actionKeys[MenuAction::TwentyMinutes] = QObject::tr("20 minutes");
    actionKeys[MenuAction::DeleteVault] = QObject::tr("Delete File Vault");
    actionKeys[MenuAction::UnLock] = QObject::tr("Unlock");
    actionKeys[MenuAction::UnLockByKey] = QObject::tr("Unlock by key");

    // Action Icons:
    DGioSettings settings("com.deepin.dde.filemanager.general", "/com/deepin/dde/filemanager/general/");
    if (settings.value("context-menu-icons").toBool()) {
        actionIcons[MenuAction::NewFolder] = QIcon::fromTheme("folder-new");
        actionIcons[MenuAction::NewDocument] = QIcon::fromTheme("document-new");
        actionIcons[MenuAction::OpenInNewWindow] = QIcon::fromTheme("window-new");
        actionIcons[MenuAction::OpenInNewTab] = QIcon::fromTheme("tab-new");
        actionIcons[MenuAction::OpenInTerminal] = QIcon::fromTheme("utilities-terminal");
        actionIcons[MenuAction::AddToBookMark] = QIcon::fromTheme("bookmark-new");
        actionIcons[MenuAction::BookmarkRemove] = QIcon::fromTheme("bookmark-remove");
        actionIcons[MenuAction::Copy] = QIcon::fromTheme("edit-copy");
        actionIcons[MenuAction::Paste] = QIcon::fromTheme("edit-paste");
        actionIcons[MenuAction::Cut] = QIcon::fromTheme("edit-cut");
        actionIcons[MenuAction::Rename] = QIcon::fromTheme("edit-rename");
        actionIcons[MenuAction::Delete] = QIcon::fromTheme("edit-delete");
        actionIcons[MenuAction::CompleteDeletion] = QIcon::fromTheme("edit-delete-shred");
        actionIcons[MenuAction::Share] = QIcon::fromTheme("document-share");
        actionIcons[MenuAction::SelectAll] = QIcon::fromTheme("edit-select-all");
        actionIcons[MenuAction::CreateSymlink] = QIcon::fromTheme("insert-link");
        actionIcons[MenuAction::Property] = QIcon::fromTheme("document-properties");
    }

    actionKeys[MenuAction::RemoveStashedRemoteConn] = QObject::tr("Remove");
    actionKeys[MenuAction::RefreshView] = QObject::tr("Refresh");
}

void DFileMenuData::initActions()
{
    QList<MenuAction> unCachedActions;
    unCachedActions << MenuAction::NewWindow << MenuAction::RefreshView;
    foreach (MenuAction key, actionKeys.keys()) {
        if (unCachedActions.contains(key)) {
            continue;
        }

        ///###: MenuAction::TagFilesUseColor represents the button for tagging files.
        ///###: MenuAction::ChangeTagColor represents that you change the color of a present tag.
        ///###: They are different event.
        if (key == MenuAction::TagFilesUseColor || key == MenuAction::ChangeTagColor) {
//为了解决在自动整理模式下，选择多个文件发送到一个没有空间的光驱上，在桌面上打开任意文件，
//弹出很多文件的窗口并且右键菜单颜色标记出现英文问题。临时方案
//将里面color相关的代码到外面初始化时重新加载
//            DTagActionWidget *tagWidget{ new DTagActionWidget };
//            QWidgetAction *tagAction{ new QWidgetAction{ nullptr } };

//            tagAction->setDefaultWidget(tagWidget);

//            switch (key) {
//            case MenuAction::TagFilesUseColor: {
//                tagAction->setText("Add color tags");
//                break;
//            }
//            case MenuAction::ChangeTagColor: {
//                tagAction->setText("Change color of present tag");
//                tagWidget->setExclusive(true);
//                tagWidget->setToolTipVisible(false);
//                break;
//            }
//            default:
//                break;
//            }

//            tagAction->setData(key);
//            actions.insert(key, tagAction);
//            actionToMenuAction.insert(tagAction, key);
            continue;
        }

        QAction *action = new QAction(actionIcons.value(key), actionKeys.value(key), nullptr);
        action->setData(key);
        actions.insert(key, action);
        actionToMenuAction.insert(action, key);
    }

    additionalMenu = new DFMAdditionalMenu();
}

/**
 * @brief 谨慎调用!!! 该函数用于ut清理action用, 其它地方调用后果自负
 * @param
 * @return
 */
void DFileMenuData::clearActions()
{
    for (const auto &menu : actions.keys()) {
        QAction *action = actions.take(menu);
        actionToMenuAction.remove(action);
        delete action;
    }
    actions.clear();
    actionToMenuAction.clear();
}

bool DFileMenuManager::needDeleteAction()
{
    return false;
}

DFileMenu *DFileMenuManager::genereteMenuByKeys(const QVector<MenuAction> &keys,
                                                const QSet<MenuAction> &disableList,
                                                bool checkable,
                                                const QMap<MenuAction, QVector<MenuAction> > &subMenuList, bool isUseCachedAction, bool isRecursiveCall)
{
    static bool actions_initialized = false;

    if (!actions_initialized) {
        actions_initialized = true;
        DFileMenuData::initData();
        DFileMenuData::initActions();
    }

    if (!isUseCachedAction) {
        foreach (MenuAction actionKey, keys) {
            QAction *action = DFileMenuData::actions.take(actionKey);

            if (action) {
                DFileMenuData::actionToMenuAction.remove(action);
            }
            if (needDeleteAction())
                delete action;
        }
    }

    DFileMenu *menu = new DFileMenu();

    if (!isRecursiveCall) {
        connect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);
    }

    foreach (MenuAction key, keys) {
        if (!isAvailableAction(key)) {
            continue;
        }
        /****************************************************************************/
        //为了解决在自动整理模式下，选择多个文件发送到一个没有空间的光驱上，在桌面上打开任意文件，
        //弹出很多文件的窗口并且右键菜单颜色标记出现英文问题。临时方案
        //将里面color相关的代码到外面初始化时重新加载
        if (key == MenuAction::TagFilesUseColor || key == MenuAction::ChangeTagColor) {
            DTagActionWidget *tagWidget{ new DTagActionWidget };
            QWidgetAction *tagAction{ new QWidgetAction{ nullptr } };

            tagAction->setDefaultWidget(tagWidget);

            switch (key) {
            case MenuAction::TagFilesUseColor: {
                tagAction->setText("Add color tags");
                break;
            }
            case MenuAction::ChangeTagColor: {
                tagAction->setText("Change color of present tag");
                tagWidget->setExclusive(true);
                tagWidget->setToolTipVisible(false);
                break;
            }
            default:
                break;
            }

            tagAction->setData(key);
            auto keyAction = DFileMenuData::actions.take(key);
            if (keyAction) {
                QWidgetAction *widAction = dynamic_cast<QWidgetAction *>(keyAction);
                if (widAction && widAction->defaultWidget()) {
                    widAction->defaultWidget()->deleteLater();
                }

                DFileMenuData::actionToMenuAction.remove(keyAction);
                keyAction->deleteLater();
            }
            DFileMenuData::actions.insert(key, tagAction);
            DFileMenuData::actionToMenuAction.insert(tagAction, key);
        }
        /****************************************************************************/
        if (key == MenuAction::Separator) {
            menu->addSeparator();
        } else {
            QAction *action = DFileMenuData::actions.value(key);

            if (!action) {
                action = new QAction(DFileMenuData::actionKeys.value(key), nullptr);
                action->setData(key);
                DFileMenuData::actions[key] = action;
                DFileMenuData::actionToMenuAction[action] = key;
            }

            action->setDisabled(disableList.contains(key));
            action->setProperty("_dfm_menu", QVariant::fromValue(menu));

            menu->addAction(action);

            if (!subMenuList.contains(key)) {
                continue;
            }

            DFileMenu *subMenu = genereteMenuByKeys(subMenuList.value(key), disableList, checkable, QMap<MenuAction, QVector<MenuAction> >(), true, true);

            subMenu->QObject::setParent(menu);
            action->setMenu(subMenu);
        }
    }

    return menu;
}

QString DFileMenuManager::getActionString(MenuAction type)
{
    return DFileMenuData::actionKeys.value(type);
}

//创建自定义菜单
void DFileMenuManager::extendCustomMenu(DFileMenu *menu, bool isNormal, const DUrl &dir, const DUrl &focusFile, const DUrlList &selected, bool onDesktop)
{
    if (!DFileMenuData::customMenuParser) {
        DFileMenuData::customMenuParser = new DCustomActionParser;
    }

    const QList<DCustomActionEntry> &rootEntry = DFileMenuData::customMenuParser->getActionFiles(onDesktop);
    qDebug() << "extendCustomMenu " << isNormal << dir << focusFile << "files" << selected.size() << "entrys" << rootEntry.size();

    if (menu == nullptr || rootEntry.isEmpty())
        return;

    DCustomActionBuilder builder;
    //呼出菜单的文件夹
    builder.setActiveDir(dir);

    //获取文件列表的组合
    DCustomActionDefines::ComboType fileCombo = DCustomActionDefines::BlankSpace;
    if (isNormal) {
        fileCombo = builder.checkFileCombo(selected);
        if (fileCombo == DCustomActionDefines::BlankSpace)
            return;

        //右键单击作用的文件
        builder.setFocusFile(focusFile);
    }

    //获取支持的菜单项
    auto usedEntrys = builder.matchFileCombo(rootEntry, fileCombo);

    //匹配类型支持
    usedEntrys = builder.matchActions(selected, usedEntrys);
    qDebug() << "selected combo" << fileCombo << "entry count" << usedEntrys.size();

    if (usedEntrys.isEmpty())
        return;

    //添加菜单响应所需的数据
    {
        QVariant var;
        var.setValue(dir);
        menu->setProperty(DCustomActionDefines::kCustomActionDataDir, var);

        var.setValue(focusFile);
        menu->setProperty(DCustomActionDefines::kCustomActionDataFoucsFile, var);

        var.setValue(selected);
        menu->setProperty(DCustomActionDefines::kCustomActionDataSelectedFiles, var);
    }

    //开启tooltips
    menu->setToolTipsVisible(true);

    //移除所有菜单项
    auto systemActions = menu->actions();
    for (auto it = systemActions.begin(); it != systemActions.end(); ++it)
        menu->removeAction(*it);
    Q_ASSERT(menu->actions().isEmpty());

    QMap<int, QList<QAction*>> locate;
    QMap<QAction*, DCustomActionDefines::Separator> actionsSeparator;
    //根据配置信息创建菜单项
    for (auto it = usedEntrys.begin(); it != usedEntrys.end(); ++it) {
        const DCustomActionData &actionData = it->data();
        auto *action = builder.buildAciton(actionData, menu);
        if (action == nullptr)
            continue;

        //自动释放
        action->setParent(menu);

        //记录分隔线
        if (actionData.separator() != DCustomActionDefines::None)
            actionsSeparator.insert(action, actionData.separator());

        //根据组合类型获取插入位置
        auto pos = actionData.position(fileCombo);

        //位置是否有效
        if (pos > 0) {
            auto temp = locate.find(pos);
            if (temp == locate.end()) {
                locate.insert(pos, {action});
            }
            else { //位置冲突，往后放
                temp->append(action);
            }
        }
        else {  //没有配置位置，则直接添加
            systemActions.append(action);
        }
    }

    //开始按顺序插入菜单
    DCustomActionDefines::sortFunc(locate, systemActions, [menu](const QList<QAction *> &acs){
        menu->addActions(acs);
    },[](QAction *ac) ->bool {
        return ac && !ac->isSeparator();
    });

    Q_ASSERT(systemActions.isEmpty());

    //插入分隔线
    for (auto it = actionsSeparator.begin(); it != actionsSeparator.end(); ++it) {
        //上分割线
        if (it.value() & DCustomActionDefines::Top) {
            menu->insertSeparator(it.key());
        }

        //下分割线
        if ((it.value() & DCustomActionDefines::Bottom)) {
            const QList<QAction*> &actionList = menu->actions();
            int nextIndex = actionList.indexOf(it.key()) + 1;

            //后一个action
            if (nextIndex < actionList.size()) {
                auto nextAction = menu->actionAt(nextIndex);

                //不是分割线则插入
                if (!nextAction->isSeparator()) {
                    menu->insertSeparator(nextAction);
                }
            }
        }

    }
}

/*
 * 自定义菜单在以下作用域时支持：
 * U盘盘内文件、移动硬盘盘内文件、桌面
 *
 * 自定义菜单在以下作用域时不支持：
 * 光盘盘内文件、回收站入口、回收站目录内、搜索框内、
 * 搜索结果列表页、文件管理器计算机主页面左侧导航全部菜单项、
 * 文件管理器计算机主页面我的目录及分区、保险箱入口、保险箱路径
 */
bool DFileMenuManager::isCustomMenuSupported(const DUrl &viewRootUrl)
{
    const QString &path = viewRootUrl.toLocalFile();
    //U盘、硬盘支持显示
    if (deviceListener->isBlockFile(path))
        return true;

    DStorageInfo st(path);
    return st.isLocalDevice() //过滤 手机 网络 smb ftp
            && !viewRootUrl.isUserShareFile() //过滤共享文件
            && !deviceListener->isFileFromDisc(path) //过滤光盘
            && !viewRootUrl.isVaultFile() //过滤保险箱
            && !viewRootUrl.isTrashFile();//过滤回收站

}

void DFileMenuManager::addActionWhitelist(MenuAction action)
{
    DFileMenuData::whitelist << action;
}

void DFileMenuManager::setActionWhitelist(const QSet<MenuAction> &actionList)
{
    DFileMenuData::whitelist = actionList;
}

QSet<MenuAction> DFileMenuManager::actionWhitelist()
{
    return DFileMenuData::whitelist;
}

void DFileMenuManager::addActionBlacklist(MenuAction action)
{
    DFileMenuData::blacklist << action;
}

void DFileMenuManager::setActionBlacklist(const QSet<MenuAction> &actionList)
{
    DFileMenuData::blacklist = actionList;
}

QSet<MenuAction> DFileMenuManager::actionBlacklist()
{
    return DFileMenuData::blacklist;
}

bool DFileMenuManager::isAvailableAction(MenuAction action)
{
    const QString &group_name = QStringLiteral("MenuActions");

    // init menu action black list
    const QMetaEnum &action_enum = QMetaEnum::fromType<MenuAction>();

    for (const QString &action_name : DFMApplication::genericObtuselySetting()->value(group_name, "disable").toStringList()) {
        bool ok = false;
        int key = action_enum.keyToValue(action_name.toUtf8(), &ok);

        if (ok && key == action) {
            return false;
        }
    }

    if (DFileMenuData::whitelist.isEmpty()) {
        return !DFileMenuData::blacklist.contains(action);
    }

    return DFileMenuData::whitelist.contains(action) && !DFileMenuData::blacklist.contains(action);
}

void DFileMenuManager::setActionString(MenuAction type, QString actionString)
{
    DFileMenuData::actionKeys.insert(type, actionString);

    QAction *action = new QAction(actionString, nullptr);
    action->setData(type);
    DFileMenuData::actions.insert(type, action);
    DFileMenuData::actionToMenuAction[action] = type;
}

void DFileMenuManager::setActionID(MenuAction type, QString id)
{
    DFileMenuData::actionIDs.insert(type, id);
}

MenuAction DFileMenuManager::registerMenuActionType(QAction *action)
{
    Q_ASSERT(action);

    MenuAction type = DFileMenuData::actionToMenuAction.value(action, MenuAction::Unknow);

    if (type >= MenuAction::UserMenuAction) {
        return type;
    }

    type = DFileMenuData::takeAvailableUserActionType();
    DFileMenuData::actions[type] = action;
    DFileMenuData::actionToMenuAction[action] = type;

    QObject::connect(action, &QAction::destroyed, action, [type] {
        DFileMenuData::recycleUserActionType(type);
    });

    return type;
}

bool DFileMenuManager::whetherShowTagActions(const QList<DUrl> &urls)
{
#ifdef DISABLE_TAG_SUPPORT
    return false;
#endif // DISABLE_TAG_SUPPORT

    for (const DUrl &durl : urls) {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(nullptr, durl);

        if (!info)
            return false;

//        bool temp{ DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(info->toLocalFile().toLocal8Bit()) };

        if (!info->canTag()) {
            return false;
        }

        //多选文件中包含一下文件时 则不展示标记信息菜单项
        if (info->fileUrl() == DesktopFileInfo::computerDesktopFileUrl()
                || info->fileUrl() == DesktopFileInfo::trashDesktopFileUrl()
                || info->fileUrl() == DesktopFileInfo::homeDesktopFileUrl()) {
            return false;
        }
    }

    return true;
}

void DFileMenuManager::clearActions()
{
    DFileMenuData::clearActions();
}

void DFileMenuManager::actionTriggered(QAction *action)
{
    qDebug() << action << action->data().isValid();
    DFileMenu *menu = qobject_cast<DFileMenu *>(sender());
    if (!(menu->property("ToolBarSettingsMenu").isValid() && menu->property("ToolBarSettingsMenu").toBool())) {
        disconnect(menu, &DFileMenu::triggered, fileMenuManger, &DFileMenuManager::actionTriggered);
    }

    //扩展菜单
    if (action->property(DCustomActionDefines::kCustomActionFlag).isValid()) {
        QString cmd = action->property(DCustomActionDefines::kCustomActionCommand).toString();
        DCustomActionDefines::ActionArg argFlag = static_cast<DCustomActionDefines::ActionArg>
                    (action->property(DCustomActionDefines::kCustomActionCommandArgFlag).toInt());
        DUrl dir = menu->property(DCustomActionDefines::kCustomActionDataDir).value<DUrl>();
        DUrl foucs = menu->property(DCustomActionDefines::kCustomActionDataFoucsFile).value<DUrl>();
        DUrlList selected = menu->property(DCustomActionDefines::kCustomActionDataSelectedFiles).value<DUrlList>();

        qDebug() << "argflag" << argFlag << "dir" << dir << "foucs" << foucs << "selected" << selected;
        qInfo() << "extend" << action->text() << cmd;

        QPair<QString, QStringList> runable = DCustomActionBuilder::makeCommand(cmd,argFlag,dir,foucs,selected);
        qInfo () << "exec:" << runable.first << runable.second;

        if (!runable.first.isEmpty())
            FileUtils::runCommand(runable.first, runable.second);
        return;
    }

    if (action->data().isValid()) {
        bool flag = false;
        int _type = action->data().toInt(&flag);
        MenuAction type;
        if (flag) {
            type = (MenuAction)_type;
        } else {
            qDebug() << action->data().toString();;
            return;
        }

        if (type >= MenuAction::UserMenuAction) {
            return;
        }

        if (menu->ignoreMenuActions().contains(type)) {
            return;
        }

        QAction *typeAction = DFileMenuData::actions.value(type);
        qDebug() << typeAction << action;
        if (typeAction) {
            qDebug() << typeAction->text() << action->text();
            if (typeAction->text() == action->text()) {
                const QSharedPointer<DFMMenuActionEvent> &event = menu->makeEvent(type);

                // fix bug 39754 【sp3专业版】【文件管理器】【5.2.0.8-1】挂载磁盘右键菜单，拔出挂载磁盘，选择任意右键菜单列表-属性，界面展示大小异常
                // 按照测试期望处理，当 url 构造出来的文件信息不存在的时候，不执行后续的事件处理
                const DUrlList &selUrls = event->selectedUrls();
                if (selUrls.count() > 0) {
                    const DUrl &u = selUrls.first();
                    // fix bug 60949 如果url是网络文件的挂载文件的rootpath，就要解密路径,不解密路径isLowSpeedDevice返回true
                    // 网络设备不走这个流程
                    QString path  = u.path();
                    if (u.scheme() == DFMROOT_SCHEME && path.endsWith(SUFFIX_GVFSMP)) {
                        path = QUrl::fromPercentEncoding(u.path().toUtf8());
                        path = path.startsWith("//") ? path.mid(1) : path;
                    }
                    if (u.isValid() && !DStorageInfo::isLowSpeedDevice(path)) { // 这里只针对非低速设备做判定，否则可能导致正常情况下的右键菜单响应过慢
                        DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, u);
                        if (info && !info->exists())
                            return;
                    }
                }

                //批量标记操作耗时较长，同步事件处理会使menu.exec结束等待时间过长，频繁操作下会引起崩溃
                //右键菜单本身对标记操作没有同步等待结果的需求，所以直接采用异步事件处理标记操作
                if (type == MenuAction::TagFilesUseColor) {
                    //异步事件处理存在menu对象先被析构的情况，这里将标记所需的数据先取出来放在event中
                    //fileeventprocessor中直接从event中取数据
                    QAction *action{ menu->actionAt("Add color tags") };
                    if (QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(action)) {
                        if (DTagActionWidget *tagWidget = qobject_cast<DTagActionWidget *>(widgetAction->defaultWidget())) {
                            QList<QColor> colors{ tagWidget->checkedColorList() };
                            event->setTagColors(colors);
                            DFMEventDispatcher::instance()->processEventAsync(event);
                        }
                    }
                } else {
                    DFMEventDispatcher::instance()->processEvent(event);
                }
            }
        }

#ifdef SW_LABEL
        if (DFileMenuData::actionIDs.contains(type)) {
            const QSharedPointer<DFMMenuActionEvent> &menuActionEvent = menu->makeEvent(type);
            DFMEvent event;
            event.setWindowId(menuActionEvent->windowId());
            event.setData(menuActionEvent->fileUrl());
            QMetaObject::invokeMethod(appController,
                                      "actionByIds",
                                      Qt::DirectConnection,
                                      Q_ARG(DFMEvent, event), Q_ARG(QString, DFileMenuData::actionIDs.value(type)));
        }
#endif
    }
}
