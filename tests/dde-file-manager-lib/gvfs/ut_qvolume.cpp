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

#ifndef UT_QVOLUME_CPP
#define UT_QVOLUME_CPP

#include "gvfs/qvolume.h"

#include <QDebug>

#include <gtest/gtest.h>

namespace {
class TestQVolume : public testing::Test
{
public:
    QVolume *m_volume = nullptr;

    void SetUp() override
    {
        m_volume = new QVolume;

        m_volume->setName("name");
        m_volume->setUuid("uuid");
        m_volume->setIcons(QStringList() << "1" << "2");
        m_volume->setSymbolic_icons(QStringList() << "3" << "4");
        m_volume->setCan_mount(true);
        m_volume->setCan_eject(true);
        m_volume->setShould_automount(true);
        m_volume->setUnix_device("unix_device");
        m_volume->setLable("label");
        m_volume->setNfs_mount("nfs_mount");
        m_volume->setIsMounted(true);
        m_volume->setMounted_root_uri("mounted_root_uri");
        m_volume->setActivation_root_uri("activation_root_uri");
        m_volume->setIs_removable(true);

        QDrive drive;
        drive.setName("drive");
        m_volume->setDrive(drive);

        m_volume->setDrive_unix_device("drive_unix_device");

        qDebug() << *m_volume;
    }

    void TearDown() override
    {
        delete m_volume;
    }
};
}

TEST_F(TestQVolume, name)
{
    EXPECT_EQ(QString("name"), m_volume->name());
}

TEST_F(TestQVolume, uuid)
{
    EXPECT_EQ(QString("uuid"), m_volume->uuid());
}

TEST_F(TestQVolume, icons)
{
    QStringList icons(QStringList() << "1" << "2");
    EXPECT_EQ(m_volume->icons(), icons);
}

TEST_F(TestQVolume, symbolic_icons)
{
    QStringList icons(QStringList() << "3" << "4");
    EXPECT_EQ(m_volume->symbolic_icons(), icons);
}

TEST_F(TestQVolume, can_mount)
{
    EXPECT_TRUE(m_volume->can_mount());
}

TEST_F(TestQVolume, can_eject)
{
    EXPECT_TRUE(m_volume->can_eject());
}

TEST_F(TestQVolume, should_automount)
{
    EXPECT_TRUE(m_volume->should_automount());
}

TEST_F(TestQVolume, unix_device)
{
    EXPECT_EQ(QString("unix_device"), m_volume->unix_device());
}

TEST_F(TestQVolume, lable)
{
    EXPECT_EQ(QString("label"), m_volume->lable());
}

TEST_F(TestQVolume, nfs_mount)
{
    EXPECT_EQ(QString("nfs_mount"), m_volume->nfs_mount());
}

TEST_F(TestQVolume, isMounted)
{
    EXPECT_TRUE(m_volume->isMounted());
}

TEST_F(TestQVolume, mounted_root_uri)
{
    EXPECT_EQ(QString("mounted_root_uri"), m_volume->mounted_root_uri());
}

TEST_F(TestQVolume, isValid)
{
    EXPECT_TRUE(m_volume->isValid());
}

TEST_F(TestQVolume, activation_root_uri)
{
    EXPECT_EQ(QString("activation_root_uri"), m_volume->activation_root_uri());
}

TEST_F(TestQVolume, is_removable)
{
    EXPECT_TRUE(m_volume->is_removable());
}

TEST_F(TestQVolume, drive)
{
    QDrive drive;
    drive.setName("drive");
    EXPECT_EQ(drive.name(), m_volume->drive().name());
}

TEST_F(TestQVolume, drive_unix_device)
{
    EXPECT_EQ(QString("drive_unix_device"), m_volume->drive_unix_device());
}

#endif // UT_QVOLUME_CPP
