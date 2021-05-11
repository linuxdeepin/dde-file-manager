/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include <gtest/gtest.h>
#include <QApplication>
#include <QStackedWidget>
#include <QTimer>
#include <DGuiApplicationHelper>
#include <DListView>

#include "stub.h"
#include "stubext.h"

DGUI_USE_NAMESPACE

#define private public
#define protected public

#include "bluetooth/bluetoothtransdialog.h"
#include "bluetooth/bluetoothmanager.h"
#include "bluetooth/bluetoothadapter.h"
#include "bluetooth/bluetoothmodel.h"

namespace {
class TestBluetoothTransDialog : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestBluetoothTransDialog";
        dlg = new BluetoothTransDialog(QStringList() << "/usr/bin/dde-desktop", BluetoothTransDialog::DirectlySend,
                                       "12-23-34-45-56");
    }

    void TearDown() override
    {
        std::cout << "end TestBluetoothTransDialog";
        delete dlg;
        dlg = nullptr;
    }

public:
    BluetoothTransDialog *dlg = nullptr;
};
} // namespace
#ifndef __arm__
//TEST_F(TestBluetoothTransDialog, ShowDialog)
//{
//    //阻塞CI流程
//    // dlg->show();
//    // dlg->hide();
//}

TEST_F(TestBluetoothTransDialog, tstHumanizedStrOfObexErrMsg)
{
    QString str("Timed out");
    EXPECT_TRUE(qApp->translate("BluetoothTransDialog", "File sending request timed out") == dlg->humanizedStrOfObexErrMsg(str));
    str = "0x53";
    EXPECT_TRUE(qApp->translate("BluetoothTransDialog", "The service is busy and unable to process the request") == dlg->humanizedStrOfObexErrMsg(str));
    str = "other";
    EXPECT_TRUE(str == dlg->humanizedStrOfObexErrMsg(str));
}

TEST_F(TestBluetoothTransDialog, tstSetObjTextStyle)
{
    dlg->setObjTextStyle(nullptr, 10, true);
    dlg->changeLabelTheme(nullptr, true);
}

TEST_F(TestBluetoothTransDialog, tstCanSendFiles)
{
    bool(*stub_canSendBluetoothRequest)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(BluetoothManager, canSendBluetoothRequest), stub_canSendBluetoothRequest);
    EXPECT_TRUE(dlg->canSendFiles());
}

TEST_F(TestBluetoothTransDialog, tstLambdaSlots)
{
     stub_ext::StubExt st;
     st.set_lamda(VADDR(DDialog, exec), []{return 1;});
     dlg->m_currSessionPath = "123";
     BluetoothManager::instance()->transferProgressUpdated("1234", 123, 1234, 0);

     BluetoothManager::instance()->transferProgressUpdated("123", 12, 1234, 0);

     dlg->m_progressUpdateShouldBeIgnore = true;
     BluetoothManager::instance()->transferProgressUpdated("123", 123, 12, 0);
     dlg->m_progressUpdateShouldBeIgnore = false;

     dlg->m_firstTransSize = 123;
     BluetoothManager::instance()->transferProgressUpdated("123", 1234, 123, 0);

     dlg->m_stack->setCurrentIndex(BluetoothTransDialog::SuccessPage);
     BluetoothManager::instance()->transferProgressUpdated("123", 1234, 1234, 0);

     QEventLoop loop;
     QTimer::singleShot(1500, nullptr, [&loop]{
         loop.exit();
     });
     loop.exec();

     dlg->m_currSessionPath = "123";
     BluetoothManager::instance()->transferCancledByRemote("123");
     dlg->m_currSessionPath = "123";
     BluetoothManager::instance()->transferCancledByRemote("1234");

     dlg->m_currSessionPath = "123";
     BluetoothManager::instance()->transferFailed("123", "123", "123");
     dlg->m_currSessionPath = "123";
     BluetoothManager::instance()->transferFailed("1234", "123", "123");

     dlg->m_urls << "test";
     dlg->m_finishedUrls.clear();
     dlg->m_currSessionPath = "123";
     BluetoothManager::instance()->fileTransferFinished("123", "123");
     dlg->m_currSessionPath = "123";
     BluetoothManager::instance()->fileTransferFinished("1234", "123");

     BluetoothManager::instance()->fileTransferFinished("123", "11234");
     BluetoothManager::instance()->fileTransferFinished("", "11234");

     DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::UnknownType);
     DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::ColorType::DarkType);
}

TEST_F(TestBluetoothTransDialog, tstFindItemByIdRole)
{
    BluetoothDevice *dev = nullptr;
    EXPECT_TRUE(nullptr == dlg->findItemByIdRole(dev));
    EXPECT_TRUE(nullptr == dlg->findItemByIdRole("testNullptr"));
}

TEST_F(TestBluetoothTransDialog, tstSendFilesToDevice)
{
    void (*sendFiles_stub)(void *, QString, QStringList, QString) = [](void *, QString, QStringList, QString) {;};

    BluetoothAdapter *adapter0 = new BluetoothAdapter;
    adapter0->setId("1");
    adapter0->setName("local");
    adapter0->setPowered(true);
    BluetoothDevice *dev0 = new BluetoothDevice;
    dev0->setId("1234");
    dev0->setIcon("phone");
    adapter0->addDevice(dev0);
    QMap<QString, const BluetoothAdapter *> map;
    map.insert(adapter0->id(), adapter0);

    stub_ext::StubExt stExt;
    stExt.set_lamda(ADDR(BluetoothModel, adapters), [&map]{return map;});

    Stub st;
    st.set(static_cast<void(BluetoothManager::*)(const QString &, const QStringList &, const QString &)>(&BluetoothManager::sendFiles), sendFiles_stub);
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
    st.set(pQDialogExec, stub_DDialog_exec);

    dlg->m_finishedUrls << "/usr/bin/dde-file-manager";
    dlg->m_urls << "/usr/bin/dde-file-manager" << "/usr/bin/dde-desktop";
    dlg->sendFilesToDevice("1234");
    dlg->sendFilesToDevice("12345");

    dlg->initConn();
    emit BluetoothManager::instance()->model()->adapterAdded(nullptr);
    dlg->updateDeviceList();

    BluetoothDevice *dev = new BluetoothDevice;
    dev->setId("testDev");
    dlg->addDevice(dev);
    dlg->removeDevice("testDev");

    BluetoothAdapter *adapter = new BluetoothAdapter;
    adapter->setId("1");
    adapter->addDevice(dev);
    BluetoothManager::instance()->model()->adapterRemoved(adapter);
    BluetoothManager::instance()->model()->adapterRemoved(nullptr);

    dlg->getStyledItem(nullptr);
    dev->setPaired(true);
    dev->setIcon("phone");
    dev->setState(BluetoothDevice::StateConnected);
    DStandardItem *item = dlg->getStyledItem(dev);

    if (item)
        delete item;
    delete dev;
    delete adapter;
    delete dev0;
    delete adapter0;

    dlg->close();
}
#endif
