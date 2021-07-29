/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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
#include <QDateTime>

#include "dfileiodeviceproxy.h"


using namespace testing;
DFM_USE_NAMESPACE

class DFileIODeviceProxyTest:public testing::Test{

public:

    DFileIODeviceProxy *device = nullptr;
    DUrl url;
    QFile *file = nullptr;
    virtual void SetUp() override{
        file = new QFile("~/test.log");
        device = new DFileIODeviceProxy();
        std::cout << "start DFileIODeviceProxyTest" << std::endl;
    }

    virtual void TearDown() override{
        if (device) {
            delete device;
            device = nullptr;
        }
        if (file) {
            file->close();
            delete file;
        }
        std::cout << "end DFileIODeviceProxyTest" << std::endl;
    }
};

TEST_F(DFileIODeviceProxyTest,can_setDevice_device) {
    EXPECT_TRUE(device->device() == nullptr);
    device->setDevice(file);
    EXPECT_TRUE(device->device() != nullptr);
}

#ifndef __arm__
TEST_F(DFileIODeviceProxyTest,can_read_write) {
    EXPECT_TRUE(device->device() == nullptr);
    device->setDevice(file);
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    file->setFileName(url.toLocalFile());
    EXPECT_EQ(true,device->atEnd());
    EXPECT_EQ(false,device->reset());
    EXPECT_EQ(false, device->waitForReadyRead(1));
    EXPECT_EQ(false, device->waitForBytesWritten(1));
    EXPECT_TRUE(device->bytesToWrite() == 0);
    EXPECT_TRUE(device->bytesAvailable() == 0);
    device->close();
    EXPECT_TRUE(device->open(QIODevice::ReadWrite | QIODevice::Truncate));
    EXPECT_FALSE(device->waitForReadyRead(1));
    EXPECT_FALSE(device->waitForBytesWritten(1));
    char buffer[20] = {'a','a','a','\n'};
    device->write(buffer,20);
    EXPECT_TRUE(device->bytesToWrite() != 0);
    EXPECT_TRUE(device->bytesAvailable() == 0);
    device->seek(0);
    device->read(buffer,20);
    EXPECT_EQ(true,device->atEnd());
    EXPECT_EQ(true,device->reset());
    EXPECT_TRUE(device->bytesToWrite() == 0);
    EXPECT_TRUE(device->bytesAvailable() != 0);
    device->close();
}
#endif
