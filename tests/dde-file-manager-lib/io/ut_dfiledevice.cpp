/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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

#include "dfiledevice.h"
#include "dfmglobal.h"
#include "testhelper.h"
#include <QThread>
#include <QProcess>

using namespace testing;
DFM_USE_NAMESPACE

class DFileDeviceTestTemp : public DFileDevice{
public:
    explicit DFileDeviceTestTemp(QObject *parent = nullptr): DFileDevice (parent)
    {}
    ~DFileDeviceTestTemp() override{}
    bool setFileUrl(const DUrl &url) override{
        return DFileDevice::setFileUrl(url);
    }
protected:
    qint64 readData(char *data, qint64 maxlen) override{
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return 0;
    }
    qint64 writeData(const char *data, qint64 len) override{
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};

class DFileDeviceTest:public testing::Test{

public:
    QSharedPointer<DFileDeviceTestTemp> device;
    virtual void SetUp() override{
        device.reset(new DFileDeviceTestTemp());
        std::cout << "start DFileDeviceTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileDeviceTest" << std::endl;
    }
};

#ifndef __arm__ // arm崩溃 暂时干掉
TEST_F(DFileDeviceTest,can_fileUrl) {
    TestHelper::runInLoop([](){});
    DUrl url;
    url.fromLocalFile("~/");
    device->setFileUrl(url);
    EXPECT_FALSE(device->fileUrl().isValid());
}
#endif

TEST_F(DFileDeviceTest,can_flush) {
    EXPECT_FALSE(device->flush());
}

TEST_F(DFileDeviceTest,can_resize) {
    EXPECT_FALSE(device->resize(0));
}

TEST_F(DFileDeviceTest,can_syncToDisk) {
    EXPECT_FALSE(device->syncToDisk(false));
}

TEST_F(DFileDeviceTest,can_closeWriteReadFailed) {
    device->closeWriteReadFailed(false);
    device->cancelAllOperate();
}
