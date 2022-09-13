// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsidebardeviceitemhandler.h"

#include "singleton.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfileservices.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "interfaces/dfmsidebaritem.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "dfmsidebarmanager.h"
#include "dfilemenumanager.h"
#include "dfilemenu.h"
#include "models/dfmrootfileinfo.h"
#include "interfaces/drootfilemanager.h"
#include "shutil/fileutils.h"
#include "dblockdevice.h"

#include <QAction>

#define REPORT_SHARE_DIR "Sharing Folders"

DFM_BEGIN_NAMESPACE

DViewItemAction *DFMSideBarDeviceItemHandler::createUnmountOrEjectAction(const DUrl &url, bool withText)
{
    DViewItemAction * action = new DViewItemAction(Qt::AlignCenter, QSize(16, 16), QSize(), true);
    if (withText) {
        action->setText(QObject::tr("Unmount"));
    }
    action->setIcon(QIcon::fromTheme("media-eject-symbolic"));

    QObject::connect(action, &QAction::triggered, [url](){
        const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, url);
        const QVector<MenuAction> menuactions = infoPointer->menuActionList();
        if (static_cast<DFMRootFileInfo::ItemType>(infoPointer->fileType()) == DFMRootFileInfo::ItemType::UDisksOptical) {
            AppController::instance()->actionEject(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
        } else if (menuactions.contains(MenuAction::SafelyRemoveDrive)) {
            AppController::instance()->actionSafelyRemoveDrive(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
        } else if (menuactions.contains(MenuAction::Unmount)) {
            AppController::instance()->actionUnmount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
        }
    });

    return action;
}

DFMSideBarItem *DFMSideBarDeviceItemHandler::createItem(const DUrl &url)
{
    DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, url);
    //fix 60959 如果是smb,sftp,ftp的共享文件，本来是显示的，但是断网了以后，新开窗口就创建了新的dfmrootfileinfo判断文件就不存在，
    //到DRootFileManager中去获取缓存，如果缓存不存在就返回
    const QString &urlPath = url.path();

    bool fileinfoExist = infoPointer && infoPointer->exists();
    if(!infoPointer
            || (!fileinfoExist && infoPointer->suffix() == SUFFIX_GVFSMP && (FileUtils::isSmbPath(urlPath) || (urlPath.contains("ftp") && urlPath.contains("host"))))) {
        infoPointer = DRootFileManager::getFileInfo(url);
    }
    fileinfoExist = infoPointer && infoPointer->exists();
    if(!infoPointer
            || (!fileinfoExist && infoPointer->suffix() != SUFFIX_GVFSMP && !(FileUtils::isSmbPath(urlPath) || (urlPath.contains("ftp") && urlPath.contains("host"))))) {
        return nullptr;
    }
    QString displayName = infoPointer->fileDisplayName();
    QString iconName = infoPointer->iconName() + "-symbolic";
    if(url.scheme() == SMB_SCHEME){
        if (displayName.isEmpty()) {
            displayName = url.host();
        }
        if (iconName == "-symbolic") {
            iconName = "folder-remote-symbolic";
        }
    }

    DFMSideBarItem *item = new DFMSideBarItem(QIcon::fromTheme(iconName), displayName, url);
    item->setReportName(reportName(url));

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;

    auto actionlist = infoPointer->menuActionList();
    if (actionlist.contains(MenuAction::Rename))
        flags |= Qt::ItemIsEditable;
    item->setFlags(flags);
    item->setData(SIDEBAR_ID_DEVICE, DFMSideBarItem::ItemUseRegisteredHandlerRole);

    DViewItemActionList lst;
    DViewItemAction * act = createUnmountOrEjectAction(url, false);
    act->setIcon(QIcon::fromTheme("media-eject-symbolic"));
    act->setVisible(actionlist.contains(MenuAction::Eject) ||
                    actionlist.contains(MenuAction::Unmount) ||
                    actionlist.contains(MenuAction::SafelyRemoveDrive));

    // 防止无图标的act也占用空间，item缩小时文字被压缩至空
    if (act->isVisible()) {
        lst.push_back(act);
        item->setActionList(Qt::RightEdge, lst);
    }

    return item;
}

DFMSideBarDeviceItemHandler::DFMSideBarDeviceItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

void DFMSideBarDeviceItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    AppController::instance()->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(sidebar, item->url()));
}

QMenu *DFMSideBarDeviceItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(this, item->url());
    QVariantHash info = infoPointer->extraProperties();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());
    QSet<MenuAction> disabled;

    if (shouldDisable) {
        disabled.insert(MenuAction::OpenInNewTab);
        disabled.insert(MenuAction::OpenDiskInNewTab);
    }
    //fix:光驱还没有加载成功前，右键点击光驱“挂载”，光驱自动弹出。
    if ((!info["mounted"].toBool()) && \
            (info["fsType"].toString().isEmpty()) && \
            (info["fsSize"].toLongLong() ==  0)) {
        disabled.insert(MenuAction::OpenDiskInNewWindow);
        disabled.insert(MenuAction::OpenDiskInNewTab);
        disabled.insert(MenuAction::Mount);
        disabled.insert(MenuAction::OpticalBlank);
        //fix:不插光盘，打开文件管理器，光盘的弹出按钮不能置灰
        //disabled.insert(MenuAction::Eject);
        disabled.insert(MenuAction::SafelyRemoveDrive);

        disabled.insert(MenuAction::Property);
    }

    if (item->url().path().endsWith(QString(".%1").arg(SUFFIX_STASHED_REMOTE))) {
        disabled.remove(MenuAction::Mount);
        disabled.remove(MenuAction::Property);
    }
    QVector<MenuAction> av = infoPointer->menuActionList();
    bool isSmbIp = FileUtils::isSmbHostOnly(item->url());
    if(isSmbIp){
        av.clear();//对于侧边栏smb挂载聚合项的右键菜单只显示 卸载 和 取消记住密码并卸载
        av.push_back(MenuAction::UnmountAllSmbMount);
        av.push_back(MenuAction::ForgetAllSmbPassword);
    }

    DFileMenu *menu = DFileMenuManager::genereteMenuByKeys(av, disabled);
    menu->setEventData(DUrl(), {item->url()}, WindowManager::getWindowId(wnd), sidebar);
    menu->setAccessibleInfo(AC_FILE_MENU_SIDEBAR_DEVICE_ITEM);

    return menu;
}

void DFMSideBarDeviceItemHandler::rename(const DFMSideBarItem *item, QString name)
{
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(this, item->url());
    if (infoPointer->fileDisplayName() != name) {
        DUrl newUrl;
        newUrl.setPath(name); // 直接构造 URL 会忽略掉一些特殊符号，因此使用 setPath
        DFileService::instance()->renameFile(this, item->url(), newUrl);
    }
}

QString DFMSideBarDeviceItemHandler::reportName(const DUrl &url)
{
    if (url.scheme() == SMB_SCHEME) {
        return REPORT_SHARE_DIR;
    } else if (url.scheme() == DFMROOT_SCHEME) {
        QString strPath = url.path();
        if (strPath.endsWith(SUFFIX_UDISKS)) {
            // 截获盘符名
            int startIndex = strPath.indexOf("/");
            int endIndex = strPath.indexOf(".");
            int count = endIndex - startIndex - 1;
            QString result = strPath.mid(startIndex + 1, count);
            // 组装盘符绝对路径
            QString localPath = "/dev/" + result;
            // 获得块设备路径
            QStringList devicePaths = DDiskManager::resolveDeviceNode(localPath, {});
            if (!devicePaths.isEmpty()) {
                QString devicePath = devicePaths.first();
                // 获得块设备对象
                DBlockDevice *blDev = DDiskManager::createBlockDevice(devicePath);
                // 获得块设备挂载点
                QByteArrayList mounts = blDev->mountPoints();
                if (!mounts.isEmpty()) {
                    QString mountPath = mounts.first();
                    // 如果挂载点为"/"，则为系统盘
                    if (mountPath == "/") {
                        return "System Disk";
                    } else {    // 数据盘
                        return "Data Disk";
                    }
                }
            }
        } else if (strPath.endsWith(SUFFIX_GVFSMP)) {
            return REPORT_SHARE_DIR;
        }
    }
    return "unknow disk";
}

DFM_END_NAMESPACE
