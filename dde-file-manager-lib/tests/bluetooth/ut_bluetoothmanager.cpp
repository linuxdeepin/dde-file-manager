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

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>



namespace  {
/**
 * @brief 此类难以编写单元测试，需要蓝牙适配器支持
 */
class TestBluetoothManager:  public testing::Test {
public:
    void SetUp() override
    {
        bluetoothManager;
        bluetoothManager->refresh();
    }

    void TearDown() override
    {

    }
};
}

TEST_F(TestBluetoothManager, model)
{
    EXPECT_NO_FATAL_FAILURE(bluetoothManager->model());
}

TEST_F(TestBluetoothManager, sendFiles)
{
    BluetoothDevice device;
    EXPECT_TRUE(bluetoothManager->sendFiles(device, QStringList()).isEmpty());
}

TEST_F(TestBluetoothManager, cancleTransfer)
{
    EXPECT_TRUE(bluetoothManager->cancelTransfer(""));
}

TEST_F(TestBluetoothManager, showBluetoothSettings)
{
    bluetoothManager->showBluetoothSettings();
}

