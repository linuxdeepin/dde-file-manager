#include <gtest/gtest.h>
#include <QDateTime>

#include "dlocalfiledevice.h"


using namespace testing;
DFM_USE_NAMESPACE

class DLocalFileDeviceTest:public testing::Test{

public:

    DLocalFileDevice *device = nullptr;
    DUrl url;
    virtual void SetUp() override{
        device = new DLocalFileDevice();
        std::cout << "start DLocalFileDeviceTest" << std::endl;
    }

    virtual void TearDown() override{
        if (device) {
            delete device;
            device = nullptr;
        }
        std::cout << "end DLocalFileDeviceTest" << std::endl;
    }
};

TEST_F(DLocalFileDeviceTest,can_setFileUrl) {
    EXPECT_EQ(false,device->setFileUrl(url));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test_lo.log");
    EXPECT_EQ(true,device->setFileUrl(url));
}

TEST_F(DLocalFileDeviceTest,can_openandclose) {
    device->close();
    EXPECT_EQ(false,device->open(QIODevice::ReadOnly));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(false,device->open(QIODevice::Text));
    EXPECT_EQ(true,device->open(QIODevice::Truncate | QIODevice::ReadOnly));
    device->closeWriteReadFailed(false);
    EXPECT_EQ(false,device->open(QIODevice::ReadWrite));
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::ReadWrite));
    device->closeWriteReadFailed(true);
    device->closeWriteReadFailed(false);
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::WriteOnly));
    device->closeWriteReadFailed(true);
    device->close();
}

TEST_F(DLocalFileDeviceTest,can_handle) {
    device->close();
    EXPECT_EQ(-1,device->handle());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_TRUE(device->handle() == -1);
}

TEST_F(DLocalFileDeviceTest,can_size) {
    EXPECT_EQ(0,device->size());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(0,device->size());
}

TEST_F(DLocalFileDeviceTest,can_resize) {
    EXPECT_EQ(false,device->resize(64));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(true, device->open(QIODevice::ReadWrite));
    EXPECT_EQ(true,device->resize(64));
    device->close();
}

TEST_F(DLocalFileDeviceTest,can_isSequential) {
    EXPECT_EQ(false,device->isSequential());
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

TEST_F(DLocalFileDeviceTest,can_write) {
    char buffer[20] = {'a','\n'};
    EXPECT_EQ(-1,device->write(buffer,20));
    device->closeWriteReadFailed(true);
    device->closeWriteReadFailed(false);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(true,device->open(QIODevice::ReadOnly));
    EXPECT_EQ(-1,device->write(buffer,20));
    EXPECT_EQ(true,device->flush());
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::WriteOnly));
    EXPECT_EQ(20,device->write(buffer,20));
    EXPECT_EQ(true,device->flush());
    EXPECT_EQ(true,device->syncToDisk());
    device->close();
    EXPECT_EQ(true,device->open(QIODevice::ReadWrite));
    EXPECT_EQ(20,device->write(buffer,20));
    device->closeWriteReadFailed(true);
    device->closeWriteReadFailed(false);
    device->close();
}

TEST_F(DLocalFileDeviceTest,can_read) {
    char buffer[20] = {};
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
    EXPECT_EQ(0,device->read(buffer,20));
    device->close();

}

TEST_F(DLocalFileDeviceTest,can_pos_seek) {
    EXPECT_EQ(false,device->seek(1));
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

TEST_F(DLocalFileDeviceTest,can_fileUrl) {
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(url,static_cast<DFileDevice *>(device)->fileUrl());
}

TEST_F(DLocalFileDeviceTest,can_setFileName) {
    EXPECT_EQ(false,device->setFileName(url.path()));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    device->setFileUrl(url);
    EXPECT_EQ(true,device->setFileName(url.path()));
    EXPECT_EQ(url,static_cast<DFileDevice *>(device)->fileUrl());
}
