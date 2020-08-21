#include <gtest/gtest.h>
#include <QDateTime>

#include "dgiofiledevice.h"


using namespace testing;
DFM_USE_NAMESPACE

class DGIOFileDeviceTest:public testing::Test{

public:

    DGIOFileDevice *device = nullptr;
    DUrl url;
    virtual void SetUp() override{
        device = new DGIOFileDevice(url);
        std::cout << "start DGIOFileDeviceTest" << std::endl;
    }

    virtual void TearDown() override{
        if (device) {
            delete device;
            device = nullptr;
        }
        std::cout << "end DGIOFileDeviceTest" << std::endl;
    }
};

TEST_F(DGIOFileDeviceTest,can_setFileUrl) {
    EXPECT_EQ(true,device->setFileUrl(url));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    EXPECT_EQ(true,device->setFileUrl(url));
}

TEST_F(DGIOFileDeviceTest,can_openandclose) {
    EXPECT_EQ(true,device->setFileUrl(url));
    device->close();
    EXPECT_EQ(false,device->open(QIODevice::ReadOnly));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(false,device->open(QIODevice::Text));
    EXPECT_EQ(true,device->open(QIODevice::Truncate | QIODevice::ReadOnly));
    device->closeWriteReadFailed(false);
    EXPECT_EQ(true,device->open(QIODevice::ReadWrite));
    device->closeWriteReadFailed(true);
    device->closeWriteReadFailed(false);
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::WriteOnly));
    device->closeWriteReadFailed(true);
    device->close();
}

TEST_F(DGIOFileDeviceTest,can_handle) {
    device->close();
    EXPECT_EQ(-1,device->handle());
}

TEST_F(DGIOFileDeviceTest,can_size) {
    EXPECT_EQ(-1,device->size());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(0,device->size());
}

TEST_F(DGIOFileDeviceTest,can_resize) {
    EXPECT_EQ(false,device->resize(64));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(true, device->open(QIODevice::ReadWrite));
    EXPECT_EQ(true,device->resize(64));
    device->close();
}

TEST_F(DGIOFileDeviceTest,can_isSequential) {
    EXPECT_EQ(true,device->isSequential());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(true,device->open(QIODevice::ReadOnly));
    EXPECT_EQ(false,device->isSequential());
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::WriteOnly));
    EXPECT_EQ(false,device->isSequential());
    device->close();
}

TEST_F(DGIOFileDeviceTest,can_write) {
    char buffer[20] = {'a','\n'};
    EXPECT_EQ(-1,device->write(buffer,20));
    device->closeWriteReadFailed(true);
    device->closeWriteReadFailed(false);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(true,device->open(QIODevice::ReadOnly));
    EXPECT_EQ(-1,device->write(buffer,20));
    EXPECT_EQ(false,device->flush());
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::WriteOnly));
    EXPECT_EQ(20,device->write(buffer,20));
    EXPECT_EQ(true,device->flush());
    EXPECT_EQ(true,device->syncToDisk());
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::ReadWrite));
    EXPECT_EQ(-1,device->write(buffer,20));
    device->closeWriteReadFailed(true);
    device->closeWriteReadFailed(false);
    device->close();
}

TEST_F(DGIOFileDeviceTest,can_read) {
    char buffer[20] = {};
    device->close();
    device->closeWriteReadFailed(true);
    device->closeWriteReadFailed(false);
    EXPECT_EQ(-1,device->read(buffer,20));
    EXPECT_EQ(false,device->syncToDisk());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(true,device->open(QIODevice::ReadOnly));
    EXPECT_EQ(20,device->read(buffer,20));
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::WriteOnly));
    EXPECT_EQ(-1,device->read(buffer,20));
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::ReadWrite));
    EXPECT_EQ(-1,device->read(buffer,20));
    device->close();

}

TEST_F(DGIOFileDeviceTest,can_pos_seek) {
    EXPECT_EQ(true,device->seek(1));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(true,device->open(QIODevice::ReadOnly));
    EXPECT_EQ(0,device->pos());
    EXPECT_EQ(true,device->seek(1));
    EXPECT_EQ(1,device->pos());
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::WriteOnly));
    EXPECT_EQ(0,device->pos());
    EXPECT_EQ(true,device->seek(1));
    device->close();
}

TEST_F(DGIOFileDeviceTest,can_fileUrl) {
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(url,static_cast<DFileDevice *>(device)->fileUrl());
}

