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

#include "usershare/shareinfo.h"
#include <QProcess>
#include <QDebug>

using namespace testing;

class ShareInfoTest:public testing::Test{

public:
    ShareInfo *info = nullptr;
    virtual void SetUp() override{
        info = new ShareInfo("share_info","/sys/power");
        std::cout << "start ShareInfoTest" << std::endl;
    }

    virtual void TearDown() override{
        if (info) {
            delete info;
        }
        std::cout << "end ShareInfoTest" << std::endl;
    }
};

TEST_F(ShareInfoTest,can_get_shareName) {
    EXPECT_EQ(QString("share_info"),info->shareName());
}

TEST_F(ShareInfoTest,can_setShareName) {
    info->setShareName("share_test");
    EXPECT_EQ(QString("share_test"),info->shareName());
}

TEST_F(ShareInfoTest,can_get_and_setPath) {
    EXPECT_EQ(QString("/sys/power"),info->path());
    info->setPath("/sys");
    EXPECT_EQ(QString("/sys"),info->path());
}

TEST_F(ShareInfoTest,can_get_and_setComment) {
    EXPECT_FALSE(info->comment().isNull());
    info->setComment("share info comment");
    EXPECT_EQ(QString("share info comment"),info->comment());
}

TEST_F(ShareInfoTest,can_get_and_setUsershare_acl) {
    EXPECT_FALSE(info->usershare_acl().isNull());
    info->setUsershare_acl("");
    EXPECT_TRUE(info->usershare_acl().contains("\"\""));
    info->setUsershare_acl("Everyone:R");
    EXPECT_EQ(QString("Everyone:R"),info->usershare_acl());
}

TEST_F(ShareInfoTest,can_get_and_setGuest_ok) {
    EXPECT_TRUE(info->guest_ok().compare("n"));
    info->setGuest_ok("");
    EXPECT_TRUE(info->guest_ok().contains("n"));
    info->setGuest_ok("n");
    info->setGuest_ok("y");
    EXPECT_EQ(QString("y"),info->guest_ok());
}

TEST_F(ShareInfoTest,can_get_and_setIsWritable) {
    EXPECT_FALSE(info->isWritable());
    info->setIsWritable(false);
    info->setIsWritable(true);
    EXPECT_TRUE(info->isWritable());
}

TEST_F(ShareInfoTest,can_get_and_setIsGuestOk) {
    EXPECT_FALSE(info->isGuestOk());
    info->setIsGuestOk(false);
    info->setIsGuestOk(true);
    EXPECT_TRUE(info->isGuestOk());
}

TEST_F(ShareInfoTest,can_isValid) {
    qDebug() << *info;
    EXPECT_TRUE(info->isValid());
    info->setShareName("");
    EXPECT_FALSE(info->isValid());
    qDebug() << *info;
}



