/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: xushitong<xushitong@uniontech.com>
 *
 *
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

#ifndef BluetoothTransDialog_H
#define BluetoothTransDialog_H

#include <dtkwidget_global.h>
#include <DDialog>
#include <durl.h>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DCommandLinkButton;
class DListView;
class DProgressBar;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QStackedWidget;
QT_END_NAMESPACE

#define TITLE_BT_TRANS_FILE BluetoothTransDialog::tr("Bluetooth File Transfer")
#define TITLE_BT_TRANS_SUCC BluetoothTransDialog::tr("File Transfer Successful")
#define TITLE_BT_TRANS_FAIL BluetoothTransDialog::tr("File Transfer Failed")

#define TXT_SENDING_FILE BluetoothTransDialog::tr("Sending files to \"%1\"")
#define TXT_SENDING_FAIL BluetoothTransDialog::tr("Failed to send files to \"%1\"")
#define TXT_SENDING_SUCC BluetoothTransDialog::tr("Sent to \"%1\" successfully")
#define TXT_SELECT_DEVIC BluetoothTransDialog::tr("Select a Bluetooth device to receive files")
#define TXT_NO_DEV_FOUND BluetoothTransDialog::tr("Cannot find the connected Bluetooth device")
#define TXT_WAIT_FOR_RCV BluetoothTransDialog::tr("Waiting for receiving, please wait...")
#define TXT_GOTO_BT_SETS BluetoothTransDialog::tr("Go to Bluetooth Settings")
#define TXT_SEND_PROGRES BluetoothTransDialog::tr("%1/%2 Sent")
#define TXT_ERROR_REASON BluetoothTransDialog::tr("Error: the Bluetooth device is disconnected")

#define TXT_NEXT BluetoothTransDialog::tr("Next")
#define TXT_CANC BluetoothTransDialog::tr("Cancel")
#define TXT_DONE BluetoothTransDialog::tr("Done")
#define TXT_RTRY BluetoothTransDialog::tr("Retry")

#define ICON_CONNECT "notification-bluetooth-connected"
#define ICON_DISCONN "notification-bluetooth-disconnected"
#define ICON_PHONE "phone"

#define PXMP_NO_DEV_LIGHT "://icons/deepin/builtin/light/icons/dfm_bluetooth_empty_light.svg"
#define PXMP_NO_DEV_DARKY "://icons/deepin/builtin/dark/icons/dfm_bluetooth_empty_dark.svg"

class BluetoothTransDialog : public DDialog
{
    Q_OBJECT
public:
    BluetoothTransDialog(QWidget *parent = nullptr);
    void setFiles(const DUrlList &urls) { m_urls = urls; }

protected:
    void initUI();

    QWidget *initDeviceSelectorPage();
    QWidget *initNonDevicePage();
    QWidget *initWaitForRecvPage();
    QWidget *initTranferingPage();
    QWidget *initFailedPage();
    QWidget *initSuccessPage();

    QList<QString> listDevices();
    void UpdateDeviceList();

    void closeEvent(QCloseEvent *event) override;

Q_SIGNALS:
    void startSpinner();
    void stopSpinner();
    void resetProgress();

public Q_SLOTS:
    void showBluTthSetting();
    void updateProgress();
    void retry();

    // 这个函数只处理每个页面的按键事件
    void onBtnClicked(const int &nIdx);

    // 该函数负责刷新每个页面的按钮和标题文字
    void onPageChagned(const int &nIdx);

private:
    enum Page {
        SelectDevicePage,
        NoneDevicePage,
        WaitForRecvPage,
        TransferPage,
        FailedPage,
        SuccessPage,
    };

    DLabel *m_title = nullptr;
    QStackedWidget *m_stack = nullptr;
    DListView *m_devicesList = nullptr;
    QStandardItemModel *m_devModel = nullptr;
    DLabel *m_subTitle1 = nullptr;
    DLabel *m_subTitle2 = nullptr;
    DLabel *m_subTitle3 = nullptr;
    DLabel *m_subTitle4 = nullptr;
    DLabel *m_sendingStatus = nullptr;

    QTimer *timer = nullptr;

private:
    DUrlList m_urls;
    QString m_selectedDevice;
    QMap<QString, int> m_indexes;
};

#endif // BluetoothTransDialog_H
