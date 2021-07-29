/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             liyigang<liyigang@uniontech.com>
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
#include <QtConcurrent>
#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public

#define protected public
#include "dgiofiledevice.h"
#include "testhelper.h"
#include "stubext.h"
#include "dabstractfilewatcher.h"


using namespace testing;
using namespace stub_ext;
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
    TestHelper::runInLoop([](){});
    EXPECT_EQ(true,device->setFileUrl(url));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    EXPECT_EQ(true,device->setFileUrl(url));
}

#ifndef __arm__
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
#endif

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

#ifndef __arm__
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
    EXPECT_NE(-1,device->write(buffer,20));
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
    EXPECT_EQ(false,device->syncToDisk(true));
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
    EXPECT_NE(-1,device->read(buffer,20));
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

TEST_F(DGIOFileDeviceTest,start_readData) {
    char buf[16] = {"a"};
    gssize (*g_input_stream_read1)(GInputStream *,void *,gsize,GCancellable*,GError**) = []
            (GInputStream *,void *,gsize,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gssize(0);
    };
    StubExt stl;
    stl.set(g_input_stream_read,g_input_stream_read1);
    EXPECT_EQ(-1,device->readData(buf,16));

    gssize (*g_input_stream_read2)(GInputStream *,void *,gsize,GCancellable*,GError**) = []
            (GInputStream *,void *,gsize,GCancellable*,GError**){
        QThread::msleep(500);
        return gssize(0);
    };
    stl.set(g_input_stream_read,g_input_stream_read2);

    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "00000";
        device->cancelAllOperate();;
    });
    EXPECT_EQ(0,device->readData(buf,16));
    future.waitForFinished();
    EXPECT_EQ(0,device->readData(buf,16));
}

TEST_F(DGIOFileDeviceTest,start_writeData) {
    char buf[16] = {"a"};
    gssize (*g_output_stream_write1)(GOutputStream *,void *,gsize,GCancellable*,GError**) = []
            (GOutputStream *,void *,gsize,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gssize(0);
    };
    StubExt stl;
    stl.set(g_output_stream_write,g_output_stream_write1);
    EXPECT_EQ(-1,device->writeData(buf,16));

    gssize (*g_output_stream_write2)(GOutputStream *,void *,gsize,GCancellable*,GError**) = []
            (GOutputStream *,void *,gsize,GCancellable*,GError**){
        QThread::msleep(500);
        return gssize(0);
    };
    stl.set(g_output_stream_write,g_output_stream_write2);

    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::msleep(150);
        qDebug() << "00000";
        device->cancelAllOperate();;
    });
    EXPECT_EQ(0,device->writeData(buf,16));
    future.waitForFinished();
    EXPECT_EQ(0,device->writeData(buf,16));
}

TEST_F(DGIOFileDeviceTest,start_flush) {
    gboolean (*g_output_stream_flush1) (GOutputStream *,GCancellable *,GError **) = []
            (GOutputStream *,GCancellable *,GError **error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        return gboolean(false);
    };
    StubExt stl;
    stl.set(g_output_stream_flush,g_output_stream_flush1);
    EXPECT_FALSE(device->flush());
}

TEST_F(DGIOFileDeviceTest,start_open) {
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile());
    device->setFileUrl(url);
    EXPECT_FALSE(device->open(QIODevice::Text));
    EXPECT_TRUE(device->open(QIODevice::ReadWrite));
    EXPECT_EQ(0,device->pos());
    EXPECT_FALSE(device->open(QIODevice::ReadWrite));
    device->close();
    EXPECT_TRUE(device->open(QIODevice::ReadWrite | QIODevice::Truncate));
    device->close();
    GFileIOStream *(*g_file_open_readwrite1)(GFile *,GCancellable*,GError**) = []
            (GFile *,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        GFileIOStream *ptr = nullptr;
        return ptr;
    };
    StubExt stl;
    stl.set(g_file_open_readwrite,g_file_open_readwrite1);
    EXPECT_FALSE(device->open(QIODevice::ReadWrite));
    stl.reset(g_file_open_readwrite);
    device->close();
    GFileInputStream *(*g_file_read1)(GFile *,GCancellable*,GError**) = []
            (GFile *,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_CANCELLED,"nihao");
        GFileInputStream *ptr = nullptr;
        return ptr;
    };
    stl.set(g_file_read,g_file_read1);
    EXPECT_FALSE(device->open(QIODevice::ReadOnly));
    stl.reset(g_file_read);
    qInfo() << QFile::exists(url.path());
    device->close();
    EXPECT_TRUE(device->open(QIODevice::ReadOnly));
    EXPECT_EQ(0,device->pos());
    device->close();

    EXPECT_TRUE(device->open(QIODevice::WriteOnly));
    device->close();
    EXPECT_TRUE(device->open(QIODevice::WriteOnly | QIODevice::Append));
    device->close();

    GFileOutputStream *(*g_file_replace1)(GFile *,const char*,gboolean,
                                          GFileCreateFlags,GCancellable*,GError**) = []
            (GFile *,const char*,gboolean,GFileCreateFlags,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_NOT_FOUND,"nihao");
        GFileOutputStream *ptr = nullptr;
        return ptr;
    };
    stl.set(g_file_replace,g_file_replace1);
    EXPECT_FALSE(device->open(QIODevice::WriteOnly));
    GFileOutputStream *(*g_file_create1)(GFile *,GFileCreateFlags,gboolean,GCancellable*,GError**) = []
            (GFile *,GFileCreateFlags,gboolean,GCancellable*,GError**error){
        GFileOutputStream *ptr = nullptr;
        return ptr;
    };
    stl.set(g_file_create,g_file_create1);
    EXPECT_TRUE(device->open(QIODevice::WriteOnly));
    device->close();

    GOutputStream *(*g_file_append_to1)(GFile *,GFileCreateFlags,GCancellable*,GError**) = []
            (GFile *,GFileCreateFlags,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_EXISTS,"nihao");
        GOutputStream *ptr = nullptr;
        return ptr;
    };
    stl.reset(g_file_replace);
    stl.set(g_file_append_to,g_file_append_to1);
    EXPECT_FALSE(device->open(QIODevice::ReadWrite | QIODevice::Append));
    device->close();
    stl.reset(g_file_append_to);

    DUrl tmpurl;
    tmpurl.setScheme(FILE_SCHEME);
    tmpurl.setPath("/tmp/ut_dgiofiledevice_tst");
    device->setFileUrl(tmpurl);
    bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
    bool (*ghostSignal1)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
    bool (*ghostSignal2)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
            (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1);
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2);

    EXPECT_TRUE(device->open(QIODevice::WriteOnly));
    device->close();
    device->setFileUrl(url);
    stl.set_lamda(g_seekable_tell,[](){return 10;});
    EXPECT_TRUE(device->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Append));
    device->close();
    stl.set_lamda(g_seekable_can_seek,[](){return false;});
    EXPECT_FALSE(device->open(QIODevice::ReadWrite | QIODevice::Truncate  | QIODevice::Append));
    device->close();
    stl.reset(g_seekable_can_seek);
    stl.set_lamda(g_seekable_seek,[](){return false;});
    EXPECT_FALSE(device->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Append));
    device->close();
    gboolean (*g_seekable_seek1)(GSeekable *,goffset,GSeekType,GCancellable*,GError**) = []
            (GSeekable *,goffset,GSeekType,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_EXISTS,"nihao");
        return gboolean(false);
    };
    stl.set(g_seekable_seek,g_seekable_seek1);
    EXPECT_FALSE(device->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Append));
    device->close();
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << tmpurl.toLocalFile());
}

TEST_F(DGIOFileDeviceTest,start_seek) {
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile());
    device->setFileUrl(url);
    EXPECT_TRUE(device->open(QIODevice::ReadOnly));
    StubExt stl;
    stl.set_lamda(g_seekable_seek,[](){return false;});
    EXPECT_FALSE(device->seek(0));
    device->close();
    EXPECT_TRUE(device->open(QIODevice::WriteOnly));
    EXPECT_FALSE(device->seek(0));

    gboolean (*g_seekable_seek1)(GSeekable *,goffset,GSeekType,GCancellable*,GError**) = []
            (GSeekable *,goffset,GSeekType,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_EXISTS,"nihao");
        return gboolean(false);
    };
    stl.set(g_seekable_seek,g_seekable_seek1);
    device->close();
    EXPECT_TRUE(device->open(QIODevice::ReadOnly));
    EXPECT_FALSE(device->seek(0));
    device->close();
    EXPECT_TRUE(device->open(QIODevice::WriteOnly));
    EXPECT_FALSE(device->seek(0));
    stl.reset(g_seekable_seek);
    device->close();
    EXPECT_TRUE(device->open(QIODevice::WriteOnly));
    EXPECT_TRUE(device->seek(0));

    EXPECT_TRUE(device->resize(0));
    stl.set_lamda(g_seekable_truncate,[](){return false;});
    EXPECT_FALSE(device->resize(0));
    gboolean (*g_seekable_truncate1)(GSeekable *,goffset,GCancellable*,GError**) = []
            (GSeekable *,goffset,GCancellable*,GError**error){
        *error = g_error_new(G_IO_ERROR,G_IO_ERROR_EXISTS,"nihao");
        return gboolean(false);
    };
    stl.set(g_seekable_truncate,g_seekable_truncate1);
    EXPECT_FALSE(device->resize(0));
    device->close();
    DUrl tmpurl;
    tmpurl.setScheme(FILE_SCHEME);
    tmpurl.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << tmpurl.toLocalFile());
}
#endif

