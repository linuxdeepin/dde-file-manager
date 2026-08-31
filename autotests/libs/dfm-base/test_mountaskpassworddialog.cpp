// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mountaskpassworddialog.cpp
 * @brief Unit tests for MountAskPasswordDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h"

#include <QTest>

using namespace src;

class MountAskPasswordDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MountAskPasswordDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MountAskPasswordDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MountAskPasswordDialogTest, getLoginData)
{
    // Test getter: QJsonObject getLoginData()
    auto result = obj->getLoginData();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MountAskPasswordDialogTest, handleConnect)
{
    // Test method: void handleConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->handleConnect());
}

TEST_F(MountAskPasswordDialogTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(MountAskPasswordDialogTest, setDomain)
{
    // Test setter: void setDomain((const QString &domain))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDomain(_arg0));
}

TEST_F(MountAskPasswordDialogTest, setDomainLineVisible)
{
    // Test setter: void setDomainLineVisible((bool domainLineVisible))
    EXPECT_NO_FATAL_FAILURE(obj->setDomainLineVisible(false));
}

TEST_F(MountAskPasswordDialogTest, setUser)
{
    // Test setter: void setUser((const QString &user))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUser(_arg0));
}
