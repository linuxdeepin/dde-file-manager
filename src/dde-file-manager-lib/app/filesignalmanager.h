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

#ifndef FILESIGNALMANAGER_H
#define FILESIGNALMANAGER_H

#include <QObject>

class DFileInfo;
class DFMEvent;
class DFMUrlBaseEvent;
class DFMUrlListBaseEvent;
class DAbstractFileInfo;
class DeviceInfo;
class DUrl;

typedef QList<DUrl> DUrlList;



class FileSignalManager : public QObject
{
    Q_OBJECT

public:
    explicit FileSignalManager(QObject *parent = nullptr);

signals:
    /* emit usershare status changed*/
    void userShareCountChanged(const int count);

    // 在共享信息刷新后，请求刷新页面
    void requestRefreshFileModel(const DUrl &url);

    /* request close current tab*/
    void requestCloseCurrentTab(quint64 winId);

    // 请求关闭窗口的所有保险箱的标签
    void requestCloseAllTabOfVault(quint64 winId);

    /* request fetch network nodes*/
    void requestFetchNetworks(const DFMUrlBaseEvent &event);

    /*request choose smb mounted file*/
    void requestChooseSmbMountedFile(const DFMUrlBaseEvent &event);

    /* current display url changed*/
    // DEPRECATED: use DFileManagerWindow::currentUrlChanged() instead
    void Q_DECL_DEPRECATED currentUrlChanged(const DFMUrlBaseEvent &event);

    /* file icon*/
    void requestIcon(const DUrl &url) const;
    void iconChanged(const DUrl &url, const QIcon &icon) const;

    /*add copy/move/delete job to taskdialog when copy/move/delete job created*/
    void jobAdded(const QMap<QString, QString> &jobDetail);

    /*remove copy/move/delete job to taskdialog when copy/move/delete job finished*/
    void jobRemoved(const QMap<QString, QString> &jobDetail);

    /*update copy/move/delete job taskdialog ui*/
    void jobDataUpdated(const QMap<QString, QString> &jobDetail,
                        const QMap<QString, QString> &data);

    /*abort copy/move/delete job taskdialog from ui*/
    void abortTask(const QMap<QString, QString> &jobDetail);

    /*copy/move job conflict dialog show */
    void conflictDialogShowed(const QMap<QString, QString> &jobDetail);

    /*request rename to DFileView*/
    void requestRename(const DFMUrlBaseEvent &event);

    /*request to rename multi files */
    void requestMultiFilesRename(const DFMUrlListBaseEvent &event);

    /* view select all */
    void requestViewSelectAll(int windowId);

    /*requst open new window by given url*/
    void requestOpenNewWindowByUrl(const DUrl &url, bool isAlwaysOpen);

    /*request active window*/
    void requestActiveWindow();

    /*request show url wrong dialog*/
    void requestShowUrlWrongDialog(const DUrl &url);

    /*request show PropertyDialog*/
    void requestShowOpenWithDialog(const DFMEvent &event);

    /*request show OpenFiles PropertyDialog*/
    void requestShowOpenFilesWithDialog(const DFMUrlListBaseEvent &event);

    /*request show PropertyDialog*/
    void requestShowPropertyDialog(const DFMUrlListBaseEvent &event);

    /*request show PropertyDialog*/
    void requestShowShareOptionsInPropertyDialog(const DFMUrlListBaseEvent &event);

    /*request show TrashPropertyDialog*/
    void requestShowTrashPropertyDialog(const DFMEvent &event);

    /*request show ComputerPropertyDialog*/
    void requestShowComputerPropertyDialog(const DFMEvent &event);

    /*request show PropertyDialog*/
    void requestShowDevicePropertyDialog(const DFMEvent &event);

    /* request select file view item*/
    void requestSelectFile(const DFMUrlListBaseEvent &event);

    /* request select file view item and rename*/
    void requestSelectRenameFile(const DFMUrlBaseEvent &event);

    /*request update mime cache*/
    void requestUpdateMimeAppsCache();

    /*close last active window */
    void aboutToCloseLastActivedWindow(int winId);

    /*sort role changed*/
    void requestCacheSortState();

    /* error dialog from disk listener*/
    void showDiskErrorDialog(const QString &id, const QString &errorText);

    /* status bar items count */
    void statusBarItemsCounted(const DFMEvent &event, int number);
    void statusBarItemsSelected(const DFMEvent &event, int number);

    /* request of renaming bookmark*/
    void requestBookmarkRename(const DFMEvent &event);

    /*book mark rename*/
    void bookmarkRenamed(const QString &newname, const DFMUrlBaseEvent &event);

    /*focus back to DFileView*/
    void requestFoucsOnFileView(quint64 winId);

    /*ctrl + F*/
    void requestSearchCtrlF(quint64 winId);

    /*ctrl + L*/
    void requestSearchCtrlL(quint64 winId);

    /*request cache samba login data*/
    void requsetCacheLoginData(const QJsonObject &obj);

    /*quit application if taskdialog close*/
    void requestQuitApplication();

    /*request fresh file view*/
    void requestFreshFileView(quint64 winId);
    void requestFreshAllFileView();
    /*request fresh desktop view*/
    void requestFreshAllDesktop();

    /*request abort copy/move job*/
    void requestAbortJob(const DUrl &url);

    /*request abort copy/move job for DFileCopyMoveJob*/
    void requestAsynAbortJob(const DUrl &url);

    /*request start file job task update timer*/
    void requestStartUpdateJobTimer();

    void requestStopUpdateJobTimer();

    /*icon size changed by setting dialog*/
    void requestChangeIconSizeBySizeIndex(const int &sizeIndex);

    /*trash state changed when trash is empty or is filled*/
    void trashStateChanged();

    /*show file preview dialog*/
    void requestShowFilePreviewDialog(const DUrlList &selectUrls, const DUrlList &entryUrls);

    /*show 4G warning dialog if file copy to removal devicve which type is fat32*/
    void requestShow4GFat32Dialog();

    /*show restore failed dialog for tagart removed*/
    void requestShowRestoreFailedDialog(const DUrlList &urlList);

    /*show retore failed dialog for permission*/
    void requestShowRestoreFailedPerssionDialog(const QString &srcPath, const QString &targetPath);

    /*show restore failed dialog for ource file does not exist*/
    void requestShowRestoreFailedSourceNotExist(const DUrlList &urlList);

    /*show dialog for no permission operation*/
    void requestShowNoPermissionDialog(const DFMUrlListBaseEvent &event);

    /*show add user share failed dialog for share name same as system uername*/
    void requestShowAddUserShareFailedDialog(const QString &sharePath);

    void requestShowErrorDialog(const QString &title, const QString &message);

    /*rename a tag*/
    void requestRenameTag(const DUrl &oldTagUrl);

    /*stop cd scan timer，擦除光盘、刻录光盘时停止定时器*/
    void stopCdScanTimer(const QString &strDevice);

    // 在擦除光盘完成后重启定时器
    void restartCdScanTimer(const QString &strDevice);

    // 激活任务进度对话框
    void activeTaskDlg();

    // 强制刷新计算机页面
    void requestUpdateComputerView();

    // 请求重定向tab页
    void requestRedirectTabUrl(const DUrl &tabRootUrl, const DUrl &newUrl);

    // 请求关闭tab页
    void requestCloseTab(const DUrl &tabRootUrl);

    //隐藏系统盘状态发生改变消息
    void requestHideSystemPartition(bool ishide);

    //手动发送文件移动信号 gio专用
    void fileMoved(const QString &from, const QString &fname, const QString &to, const QString &tname);

    //移除最近访问中的包含当前路径的文件
    void requestRemoveRecentFile(const QString &path);

    // 通过信号进入目录,目前与DFileManagerWindow::cd 关联
    bool cdFolder(const DUrl &folderUrl);

    // 请求关闭本地监听端口
    void requestCloseListen();

    // 请求停止mediainfo的获取
    void requestCloseMediaInfo(const QString path);

    // 请求忽略当前鼠标拖拽事件
    void requestIgnoreDragEvent();

    /*request show smb url mounted all window*/
    void requestShowNewWindows();

    /*mounted failed and remove need open windows*/
    void requestRemoveSmbUrl(const DUrl &url);

#ifdef SW_LABEL
    /*copy/move/delete fail job show */
    void jobFailed(int nRet, const QString &jobType, const QString &srcFileName);
#endif
};

#endif // FILESIGNALMANAGER_H
