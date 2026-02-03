// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLUETOOTHTRANSDIALOG_H
#define BLUETOOTHTRANSDIALOG_H

#include "dfmplugin_utils_global.h"

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

namespace dfmplugin_utils {

class BluetoothDevice;
class BluetoothAdapter;
class BluetoothTransDialog : public DDialog
{
    Q_OBJECT
public:
    BluetoothTransDialog(const QStringList &urls, QString targetDevId = QString(), QWidget *parent = nullptr);

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
    void setNextButtonEnable(bool enable);

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
    DLabel *failedErrorMsgLabel { nullptr };   // 失败页面的错误消息标签
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
}

#endif   // BLUETOOTHTRANSDIALOG_H
