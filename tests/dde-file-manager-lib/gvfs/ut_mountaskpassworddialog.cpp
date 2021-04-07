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

#include "gvfs/mountaskpassworddialog.h"

#include <gtest/gtest.h>

namespace  {
class TestMountAskPasswordDialog: public testing::Test
{
public:
    MountAskPasswordDialog *m_dlg {nullptr};
    void SetUp() override
    {
        m_dlg = new MountAskPasswordDialog();
    }

    void TearDown() override
    {
        delete m_dlg;
    }
};
}

TEST_F(TestMountAskPasswordDialog, show)
{
    m_dlg->show();
    m_dlg->handleButtonClicked(1, "");
    EXPECT_TRUE(m_dlg->close());
}

TEST_F(TestMountAskPasswordDialog, domainLineVisible)
{
    m_dlg->show();
    m_dlg->setDomainLineVisible(true);
    EXPECT_TRUE(m_dlg->getDomainLineVisible());
    m_dlg->close();
}

TEST_F(TestMountAskPasswordDialog, loginData)
{
    QJsonObject obj;
    obj["key"] = "admin";
    obj["password"] = "123456";
    m_dlg->setLoginData(obj);
    const QJsonObject &loginData = m_dlg->getLoginData();
    EXPECT_STREQ(obj["key"].toString().toStdString().c_str(), loginData["key"].toString().toStdString().c_str());
    EXPECT_STREQ(obj["password"].toString().toStdString().c_str(), loginData["password"].toString().toStdString().c_str());
}
