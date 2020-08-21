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



