/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "gvfs/qdrive.h"

#include <gtest/gtest.h>

namespace {
class TestQDrive: public testing::Test
{
public:
    QDrive *m_drive {nullptr};
    void SetUp() override
    {
        m_drive = new QDrive;
        qDebug() << *m_drive;
    }

    void TearDown() override
    {
        delete m_drive;
    }
};
}

TEST_F(TestQDrive, name)
{
    m_drive->setName("name");
    EXPECT_STREQ(m_drive->name().toStdString().c_str(), "name");
}

TEST_F(TestQDrive, valid)
{
    EXPECT_FALSE(m_drive->isValid());
}

TEST_F(TestQDrive, icons)
{
    m_drive->setIcons(QStringList() << "a" << "b" << "c");
    EXPECT_EQ(m_drive->icons().size(), 3);
}

TEST_F(TestQDrive, symbolic_icons)
{
    m_drive->setSymbolic_icons(QStringList() << "a" << "b" << "c");
    EXPECT_EQ(m_drive->symbolic_icons().size(), 3);
}

TEST_F(TestQDrive, has_volumes)
{
    m_drive->setHas_volumes(true);
    EXPECT_TRUE(m_drive->has_volumes());
}

TEST_F(TestQDrive, can_eject)
{
    m_drive->setCan_eject(true);
    EXPECT_TRUE(m_drive->can_eject());
}

TEST_F(TestQDrive, setCan_start)
{
    m_drive->setCan_start(true);
    EXPECT_TRUE(m_drive->can_start());
}

TEST_F(TestQDrive, can_start_degraded)
{
    m_drive->setCan_start_degraded(true);
    EXPECT_TRUE(m_drive->can_start_degraded());
}

TEST_F(TestQDrive, can_poll_for_media)
{
    m_drive->setCan_poll_for_media(true);
    EXPECT_TRUE(m_drive->can_poll_for_media());
}

TEST_F(TestQDrive, can_stop)
{
    m_drive->setCan_stop(true);
    EXPECT_TRUE(m_drive->can_stop());
}

TEST_F(TestQDrive, has_media)
{
    m_drive->setHas_media(true);
    EXPECT_TRUE(m_drive->has_media());
}

TEST_F(TestQDrive, is_media_check_automatic)
{
    m_drive->setIs_media_check_automatic(true);
    EXPECT_TRUE(m_drive->is_media_check_automatic());
}

TEST_F(TestQDrive, is_removable)
{
    m_drive->setIs_removable(true);
    EXPECT_TRUE(m_drive->is_removable());
}

TEST_F(TestQDrive, is_media_removable)
{
    m_drive->setIs_media_removable(true);
    EXPECT_TRUE(m_drive->is_media_removable());
}

TEST_F(TestQDrive, start_stop_type)
{
    m_drive->setStart_stop_type(GDriveStartStopType::G_DRIVE_START_STOP_TYPE_NETWORK);
    EXPECT_EQ(m_drive->start_stop_type(), GDriveStartStopType::G_DRIVE_START_STOP_TYPE_NETWORK);
}

TEST_F(TestQDrive, unix_device)
{
    m_drive->setUnix_device("drive");
    EXPECT_STREQ(m_drive->unix_device().toStdString().c_str(), "drive");
}

