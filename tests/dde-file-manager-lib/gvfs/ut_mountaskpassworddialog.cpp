// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
