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

#include "gvfs/qmount.h"

#include <QDebug>

#include <gtest/gtest.h>

namespace {
class TestQMount: public testing::Test
{
public:
    QMount *m_mount {nullptr};
    void SetUp() override
    {
        m_mount = new QMount;
        m_mount->setName("name");
        m_mount->setDefault_location("location");
        m_mount->setIcons(QStringList() << "1" << "2");
        m_mount->setCan_unmount(true);
        m_mount->setCan_eject(true);
        m_mount->setIs_shadowed(true);
        m_mount->setMounted_root_uri("uri");
        m_mount->setSymbolic_icons(QStringList() << "3" << "4");
    }

    void TearDown() override
    {
        delete m_mount;
    }
};
}

TEST_F(TestQMount, debug)
{
    qDebug() << (*m_mount);
}

TEST_F(TestQMount, name)
{
    EXPECT_EQ(QString("name"), m_mount->name());
}

TEST_F(TestQMount, location)
{
    EXPECT_EQ(QString("location"), m_mount->default_location());
}

TEST_F(TestQMount, icons)
{
    QStringList icons(QStringList() << "1" << "2");
    EXPECT_EQ(m_mount->icons(), icons);
}

TEST_F(TestQMount, unmount)
{
    EXPECT_TRUE(m_mount->can_unmount());
}

TEST_F(TestQMount, eject)
{
    EXPECT_TRUE(m_mount->can_eject());
}

TEST_F(TestQMount, shadowed)
{
    EXPECT_TRUE(m_mount->is_shadowed());
}

TEST_F(TestQMount, mounted_root_uri)
{
    EXPECT_EQ(QString("uri"), m_mount->mounted_root_uri());
}

TEST_F(TestQMount, uuid)
{
    EXPECT_TRUE(m_mount->uuid().isEmpty());
}

TEST_F(TestQMount, symbolic_icons)
{
    QStringList icons(QStringList() << "3" << "4");
    EXPECT_EQ(m_mount->symbolic_icons(), icons);
}
