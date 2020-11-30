/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     oucaijun <oucaijun@uniontech.com>
 *
 * Maintainer: oucaijun <oucaijun@uniontech.com>
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
#define private public
#define protected public
#include "dfmevent.h"
#include "gvfs/networkmanager.h"
#include "utils/singleton.h"
#include "app/define.h"

namespace {
class TestNetworkManager: public testing::Test
{
public:
    NetworkManager *m_manager = Singleton<NetworkManager>::instance();
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

};
}

TEST_F(TestNetworkManager, fetchNetworks)
{
    Singleton<NetworkManager>::instance()->fetchNetworks(DFMUrlBaseEvent(nullptr, DUrl("smb:///")));
}



namespace {
class TestNetworkNode: public testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};
}

TEST_F(TestNetworkNode, url)
{
    NetworkNode node;
    QString urlname = "urlname";
    QString displayname = "displayname";
    QString icontype = "icontype";

    node.setUrl(urlname);
    node.setDisplayName(displayname);
    node.setIconType(icontype);

    EXPECT_STREQ(node.url().toStdString().c_str(), urlname.toStdString().c_str());
    EXPECT_STREQ(node.displayName().toStdString().c_str(), displayname.toStdString().c_str());
    EXPECT_STREQ(node.iconType().toStdString().c_str(), icontype.toStdString().c_str());
}
