/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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
#include "app/define.h"
#include "bluetooth/bluetoothmodel.h"
#include "bluetooth/bluetoothmanager.h"
#include "bluetooth/bluetoothmanager_p.h"

#include <stub.h>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>



namespace  {
/**
 * @brief 此类难以编写单元测试，需要蓝牙适配器支持
 */
class TestBluetoothManager:  public testing::Test {
public:
    BluetoothManagerPrivate *d{nullptr};

    void SetUp() override
    {
        bluetoothManager;
        bluetoothManager->refresh();
        d = new BluetoothManagerPrivate(bluetoothManager);
    }

    void TearDown() override
    {
        delete d;
    }
};
}

TEST_F(TestBluetoothManager, connect)
{
    Stub *st = new Stub;
    BluetoothAdapter *(*adapterById_r)(void *, const QString &) = [] (void *, const QString &) { return  new BluetoothAdapter;};
    st->set(ADDR(BluetoothModel, adapterById), adapterById_r);

    EXPECT_NO_FATAL_FAILURE(
        emit d->m_bluetoothInter->AdapterAdded("");
        emit d->m_bluetoothInter->AdapterRemoved("");
        emit d->m_bluetoothInter->AdapterPropertiesChanged("");
        emit d->m_bluetoothInter->DeviceAdded("");
        emit d->m_bluetoothInter->DeviceRemoved("");
        emit d->m_bluetoothInter->DevicePropertiesChanged("");
        emit d->m_bluetoothInter->TransferCreated("", QDBusObjectPath(), QDBusObjectPath());
        emit d->m_bluetoothInter->TransferRemoved("", QDBusObjectPath(), QDBusObjectPath(), true);
        emit d->m_bluetoothInter->TransferRemoved("", QDBusObjectPath(), QDBusObjectPath(), false);
        emit d->m_bluetoothInter->ObexSessionCreated(QDBusObjectPath());
        emit d->m_bluetoothInter->ObexSessionRemoved(QDBusObjectPath());
        emit d->m_bluetoothInter->ObexSessionProgress(QDBusObjectPath(), 0, 0, 0);
        emit d->m_bluetoothInter->TransferFailed("", QDBusObjectPath(), "");
    );
}

TEST_F(TestBluetoothManager, model)
{
    EXPECT_NO_FATAL_FAILURE(bluetoothManager->model());
}

TEST_F(TestBluetoothManager, cancleTransfer)
{
    EXPECT_TRUE(bluetoothManager->cancelTransfer(""));
}

TEST_F(TestBluetoothManager, showBluetoothSettings)
{
    EXPECT_NO_FATAL_FAILURE(bluetoothManager->showBluetoothSettings());
}

TEST_F(TestBluetoothManager, p_resolve)
{
    EXPECT_NO_FATAL_FAILURE(d->resolve(QDBusReply<QString>()));
}



TEST_F(TestBluetoothManager, p_inflateAdapter)
{
    class QDBusPendingCall_R {
    public:
        bool isError() const {
            return false;
        }

    };

    Stub *st = new Stub;
    st->set(ADDR(QDBusPendingCall, isError), &QDBusPendingCall_R::isError);

    BluetoothAdapter *adapter = new BluetoothAdapter;
    QJsonObject obj;
    EXPECT_NO_FATAL_FAILURE(d->inflateAdapter(adapter, obj));
}

TEST_F(TestBluetoothManager, p_inflateDevice)
{
    BluetoothDevice dev;
    QJsonObject obj;

    EXPECT_NO_FATAL_FAILURE(d->inflateDevice(&dev, obj));
}
