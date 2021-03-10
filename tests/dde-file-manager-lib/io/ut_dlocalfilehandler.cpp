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

#include "dlocalfilehandler.h"

using namespace testing;
DFM_USE_NAMESPACE

class DLocalFileHandlerTest:public testing::Test{

public:

    DLocalFileHandler *handler = nullptr;
    DUrl dirurl,fileurl,filelinkurl,newurl,pathurl;
    virtual void SetUp() override{
        handler = new DLocalFileHandler();
        dirurl.setScheme(FILE_SCHEME);
        fileurl.setScheme(FILE_SCHEME);
        filelinkurl.setScheme(FILE_SCHEME);
        newurl.setScheme(FILE_SCHEME);
        pathurl.setScheme(FILE_SCHEME);
        dirurl.setPath(QString("~/testdir"));
        pathurl.setPath(QString("~/testpath"));
        fileurl.setPath(QString("~/testfile"));
        filelinkurl.setPath(QString("~/testfilelink"));
        newurl.setPath(QString("~/testfilenew"));
        std::cout << "start DLocalFileHandlerTest" << std::endl;
    }

    virtual void TearDown() override{
        if (handler) {
            delete handler;
        }
        std::cout << "end DLocalFileHandlerTest" << std::endl;
    }
};

TEST_F(DLocalFileHandlerTest,can_get_mkdir){
    EXPECT_EQ(true, handler->mkdir(dirurl));
}

TEST_F(DLocalFileHandlerTest,can_not_get_mkdir){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath(QString("/etc/test_dir"));
    EXPECT_EQ(false, handler->mkdir(url));
}

TEST_F(DLocalFileHandlerTest,can_get_mkpath){
    EXPECT_EQ(true, handler->mkpath(pathurl));
}

TEST_F(DLocalFileHandlerTest,can_not_get_mkpath){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath(QString("/etc/test_dir"));
    EXPECT_EQ(false, handler->mkpath(url));
}

TEST_F(DLocalFileHandlerTest,can_get_touch){
    EXPECT_EQ(true, handler->touch(fileurl));
    QString error = static_cast<DFileHandler *>(handler)->errorString();
    std::cout << error.toUtf8().toStdString() << std::endl;
    EXPECT_EQ(true, error.isEmpty());
    error = static_cast<DFileHandler*>(handler)->errorString();
    EXPECT_EQ(true, error.isEmpty());
}

TEST_F(DLocalFileHandlerTest,can_not_get_fileexists_touch){
    EXPECT_EQ(false, handler->touch(fileurl));
}

TEST_F(DLocalFileHandlerTest,can_not_get_readyonly_touch){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath(QString("/etc/test_touch"));
    EXPECT_EQ(false, handler->touch(url));
    QString error = handler->errorString();
    std::cout << error.toUtf8().toStdString() << std::endl;
    EXPECT_EQ(false, error.isEmpty());
}


TEST_F(DLocalFileHandlerTest,can_get_link){
    EXPECT_EQ(true, handler->link(fileurl.path(),filelinkurl));
}

TEST_F(DLocalFileHandlerTest,can_not_get_link){
    EXPECT_EQ(false, handler->link(newurl.path(),filelinkurl));
}

TEST_F(DLocalFileHandlerTest,can_get_rename){
    EXPECT_EQ(true, handler->rename(fileurl,newurl));
}

TEST_F(DLocalFileHandlerTest,can_not_get_rename){
    EXPECT_EQ(false, handler->rename(fileurl,newurl));
}

TEST_F(DLocalFileHandlerTest,can_get_setPermissions){
    EXPECT_EQ(true, handler->setPermissions(newurl,QFileDevice::ReadOwner|QFileDevice::WriteOwner));
}

TEST_F(DLocalFileHandlerTest,can_not_get_setPermissions){
    EXPECT_EQ(false, handler->setPermissions(fileurl,QFileDevice::ReadOwner|QFileDevice::WriteOwner));
}

TEST_F(DLocalFileHandlerTest,can_get_setFileTime){
    EXPECT_EQ(true, handler->setFileTime(newurl,QDateTime::currentDateTime(),QDateTime::currentDateTime()));
}

TEST_F(DLocalFileHandlerTest,can_not_get_setFileTime){
    EXPECT_EQ(false, handler->setFileTime(fileurl,QDateTime::currentDateTime(),QDateTime::currentDateTime()));
}

TEST_F(DLocalFileHandlerTest,can_get_file_exists){
    EXPECT_EQ(true, handler->exists(newurl));
}

TEST_F(DLocalFileHandlerTest,can_get_syslink_exists) {
    EXPECT_EQ(true, handler->exists(filelinkurl));
}

TEST_F(DLocalFileHandlerTest,can_not_get_exists){
    EXPECT_EQ(false, handler->exists(fileurl));
}

TEST_F(DLocalFileHandlerTest,can_get_remove){
    EXPECT_EQ(true, handler->remove(newurl));
}

TEST_F(DLocalFileHandlerTest,can_get_link_remove){
    EXPECT_EQ(true, handler->remove(filelinkurl));
}

TEST_F(DLocalFileHandlerTest,can_not_get_remove){
    EXPECT_EQ(false, handler->remove(fileurl));
}

TEST_F(DLocalFileHandlerTest,can_get_rmdir){
    EXPECT_EQ(true, handler->rmdir(dirurl));
    EXPECT_EQ(true, handler->rmdir(pathurl));
}

TEST_F(DLocalFileHandlerTest,can_not_get_rmdir){
    EXPECT_EQ(false, handler->rmdir(dirurl));
}




