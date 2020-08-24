/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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
#include "vfs/dfmvfsdevice.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
class TestDFMVfsDevice: public testing::Test {
public:
    dde_file_manager::DFMVfsDevice *m_service = {nullptr};
    void SetUp() override
    {
        DUrl url = DUrl::fromLocalFile("smb://10.0.12.161:445/");
        url.setScheme(SMB_SCHEME);
        m_service = dde_file_manager::DFMVfsDevice::createUnsafe(url);
        qDebug() << "--->" << m_service;
    }

    void TearDown() override
    {
        delete m_service;
    }
};
}

TEST_F(TestDFMVfsDevice, attach)
{
    EXPECT_FALSE(m_service->attach());
}

TEST_F(TestDFMVfsDevice, detachAsync)
{
    EXPECT_TRUE(m_service->detachAsync());
}

TEST_F(TestDFMVfsDevice, eventHandler)
{
    m_service->setEventHandler(nullptr);
    EXPECT_FALSE(m_service->eventHandler());
}

TEST_F(TestDFMVfsDevice, isReadOnly)
{
    EXPECT_TRUE(m_service->isReadOnly());
}

TEST_F(TestDFMVfsDevice, canDetach)
{
    EXPECT_TRUE(m_service->canDetach());
}

TEST_F(TestDFMVfsDevice, totalBytes)
{
    EXPECT_EQ(m_service->totalBytes(), 0);
}

TEST_F(TestDFMVfsDevice, usedBytes)
{
    EXPECT_EQ(m_service->usedBytes(), 0);
}

TEST_F(TestDFMVfsDevice, freeBytes)
{
    EXPECT_EQ(m_service->freeBytes(), 0);
}

TEST_F(TestDFMVfsDevice, iconList)
{
    EXPECT_TRUE(m_service->iconList().size() >= 0);
}


TEST_F(TestDFMVfsDevice, symbolicIconList)
{
    EXPECT_TRUE(m_service->symbolicIconList().size() >= 0);
}



