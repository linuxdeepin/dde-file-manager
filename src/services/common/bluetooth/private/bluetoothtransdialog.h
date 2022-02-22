/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             lvwujun<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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

#ifndef BLUETOOTHTRANSDIALOG_H
#define BLUETOOTHTRANSDIALOG_H

#include "dfm_common_service_global.h"

#include <dtkwidget_global.h>
#include <DDialog>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DCommandLinkButton;
class DListView;
class DProgressBar;
class DStandardItem;
class DSpinner;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QStackedWidget;
QT_END_NAMESPACE

DSC_BEGIN_NAMESPACE

class BluetoothDevice;
class BluetoothAdapter;
class BluetoothTransDialog : public DDialog
{
    Q_OBJECT
public:
    enum TransferMode {
        kSelectDeviceToSend = 0,   // 先选择设备再发送
        kSendToDeviceDirectly   // 直接发送到指定设备
    };

    BluetoothTransDialog(const QStringList &urls, TransferMode mode = kSelectDeviceToSend, QString targetDevId = QString(), QWidget *parent = nullptr);

    static bool isBluetoothIdle();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();
    void initConn();
    QWidget *createDeviceSelectorPage();
    QWidget *createNonDevicePage();
    QWidget *createWaitForRecvPage();
    QWidget *createTranferingPage();
    QWidget *createFailedPage();
    QWidget *createSuccessPage();

    DStandardItem *createStyledItem(const BluetoothDevice *dev);
    DStandardItem *findItemByIdRole(const BluetoothDevice *dev);
    DStandardItem *findItemByIdRole(const QString &devId);

    void updateDeviceList();
    void addDevice(const BluetoothDevice *dev);
    void removeDevice(const BluetoothDevice *dev);
    void removeDevice(const QString &id);
    void sendFiles();
    void sendFilesToDevice(const QString &devId);
    void changeLabelTheme(QLabel *obj, bool isTitle = false);
    void setObjTextStyle(QWidget *obj, int size, bool bold);
    QString humanizeObexErrMsg(const QString &msg);

private Q_SLOTS:
    void showBluetoothSetting();
    void onBtnClicked(const int &nIdx);
    void onPageChagned(const int &nIdx);
    void connectAdapter(const BluetoothAdapter *);
    void connectDevice(const BluetoothDevice *);

private:
    enum Page {
        kSelectDevicePage,
        kNoneDevicePage,
        kWaitForRecvPage,
        kTransferPage,
        kFailedPage,
        kSuccessPage,
    };

    enum DeviceRoles {
        kDevNameRole = Qt::UserRole + 100,
        kDevIdRole,
    };

    DLabel *titleOfDialog { nullptr };
    QStackedWidget *stackedWidget { nullptr };   // 多页面容器
    DListView *devicesListView { nullptr };   // 设备列表
    QStandardItemModel *devModel { nullptr };   // 列表的 model
    DLabel *subTitleForWaitPage { nullptr };
    DLabel *subTitleOfTransPage { nullptr };
    DLabel *subTitleOfFailedPage { nullptr };
    DLabel *subTitleOfSuccessPage { nullptr };
    DLabel *sendingStatusLabel { nullptr };
    DProgressBar *progressBar { nullptr };
    DSpinner *spinner { nullptr };

private:
    QStringList urlsWaitToSend;   // 待发送文件
    QStringList finishedUrls;
    QString selectedDeviceName;
    QString selectedDeviceId;
    QString currSessionPath;   // 当前发送进程的 obex 会话路径，用于取消当前传输会话
    // 忽略每次调用 sendFile 发送过来的第一次更新进度的信号，此时接收方还未同意接收文件，
    // 信号被触发可能是因为创建链路的时候携带有部分请求信息导致 transferred 数据的更新
    bool ignoreProgress { true };
    qulonglong firstTransSize = 0;
    QStringList connectedAdapters;
    QString dialogToken;   // 用于标识当前对话框ID
};
DSC_END_NAMESPACE

#endif   // BLUETOOTHTRANSDIALOG_H
