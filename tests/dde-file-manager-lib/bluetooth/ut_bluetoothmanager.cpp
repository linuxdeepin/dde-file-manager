// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/define.h"
#include "bluetooth/bluetoothmodel.h"
#include "bluetooth/bluetoothmanager.h"
#include "bluetooth/bluetoothmanager_p.h"

#include <stub.h>
#include <stubext.h>
#include "testhelper.h"
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace {
/**
 * @brief 此类难以编写单元测试，需要蓝牙适配器支持
 */
class TestBluetoothManager : public testing::Test
{
public:
    BluetoothManagerPrivate *d { nullptr };

    void SetUp() override
    {
        stub.set_lamda(&BluetoothManagerPrivate::initConnects, []() {
            qInfo() << __PRETTY_FUNCTION__;
        });
        //        bluetoothManager;
        //        bluetoothManager->refresh();
        //        d = new BluetoothManagerPrivate(bluetoothManager);
    }

    void TearDown() override
    {
        //        stub.clear();
        //        delete d;
    }

private:
    stub_ext::StubExt stub;
};
}
#ifndef __arm__
TEST_F(TestBluetoothManager, connect)
{
    //    stub_ext::StubExt *st = new stub_ext::StubExt;
    //    BluetoothAdapter *adapter = new BluetoothAdapter;
    //    st->set_lamda(ADDR(BluetoothModel, adapterById), [adapter] (void *, const QString &) {
    //        return adapter;
    //    });

    //    EXPECT_NO_FATAL_FAILURE(
    //        emit d->m_bluetoothInter->AdapterAdded("");
    //        emit d->m_bluetoothInter->AdapterRemoved("");
    //        emit d->m_bluetoothInter->AdapterPropertiesChanged("");
    //        emit d->m_bluetoothInter->DeviceAdded("");
    //        emit d->m_bluetoothInter->DeviceRemoved("");
    //        emit d->m_bluetoothInter->DevicePropertiesChanged("");
    //        emit d->m_bluetoothInter->TransferCreated("", QDBusObjectPath(), QDBusObjectPath());
    //        emit d->m_bluetoothInter->TransferRemoved("", QDBusObjectPath(), QDBusObjectPath(), true);
    //        emit d->m_bluetoothInter->TransferRemoved("", QDBusObjectPath(), QDBusObjectPath(), false);
    //        emit d->m_bluetoothInter->ObexSessionCreated(QDBusObjectPath());
    //        emit d->m_bluetoothInter->ObexSessionRemoved(QDBusObjectPath());
    //        emit d->m_bluetoothInter->ObexSessionProgress(QDBusObjectPath(), 0, 0, 0);
    //        emit d->m_bluetoothInter->TransferFailed("", QDBusObjectPath(), "");
    //    );

    //    TestHelper::runInLoop([](){
    //    }, 500);

    //    delete st;
}

TEST_F(TestBluetoothManager, model)
{
    // EXPECT_NO_FATAL_FAILURE(bluetoothManager->model());
}

TEST_F(TestBluetoothManager, hasAdapter)
{
    //  EXPECT_NO_FATAL_FAILURE(bluetoothManager->hasAdapter());
}

TEST_F(TestBluetoothManager, bluetoothSendEnable)
{
    //  EXPECT_NO_FATAL_FAILURE(BluetoothManager::bluetoothSendEnable());
}

TEST_F(TestBluetoothManager, cancleTransfer)
{
    //  EXPECT_TRUE(bluetoothManager->cancelTransfer(""));
}

TEST_F(TestBluetoothManager, showBluetoothSettings)
{
    //  EXPECT_NO_FATAL_FAILURE(bluetoothManager->showBluetoothSettings());
}

TEST_F(TestBluetoothManager, p_resolve)
{
    //  EXPECT_NO_FATAL_FAILURE(d->resolve(QDBusReply<QString>()));
}

TEST_F(TestBluetoothManager, p_inflateAdapter)
{
    //    class QDBusPendingCall_R {
    //    public:
    //        bool isError() const {
    //            return false;
    //        }

    //    };

    //    Stub *st = new Stub;
    //    st->set(ADDR(QDBusPendingCall, isError), &QDBusPendingCall_R::isError);

    //    BluetoothAdapter *adapter = new BluetoothAdapter;
    //    QJsonObject obj;
    //    EXPECT_NO_FATAL_FAILURE(d->inflateAdapter(adapter, obj));

    //    TestHelper::runInLoop([](){
    //    }, 500);

    //    delete st;
    //    delete adapter;
}

TEST_F(TestBluetoothManager, p_inflateDevice)
{
    //    BluetoothDevice dev;
    //    QJsonObject obj;

    //    EXPECT_NO_FATAL_FAILURE(d->inflateDevice(&dev, obj));
}
#endif
