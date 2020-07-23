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

class BluetoothTransDialog : public DDialog
{
    Q_OBJECT
public:
    BluetoothTransDialog(QWidget *parent = nullptr);

    /**
     * @brief 设置要发送的文件列表
     * @param urls 待发送的文件列表 url
     */
    void setFiles(const DUrlList &urls) { m_urls = urls; }

Q_SIGNALS:
    void startSpinner();
    void stopSpinner();
    void resetProgress();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();
    QWidget *initDeviceSelectorPage();
    QWidget *initNonDevicePage();
    QWidget *initWaitForRecvPage();
    QWidget *initTranferingPage();
    QWidget *initFailedPage();
    QWidget *initSuccessPage();

    QList<QString> listDevices();
    void UpdateDeviceList();

private Q_SLOTS:
    void showBluetoothSetting();

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
};

#endif // BluetoothTransDialog_H
