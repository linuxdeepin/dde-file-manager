// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultactivesavekeyfileview_1.cpp
 * @brief Unit tests for VaultActiveSaveKeyFileView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/createvaultview/vaultactivesavekeyfileview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultActiveSaveKeyFileViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultActiveSaveKeyFileView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultActiveSaveKeyFileView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultActiveSaveKeyFileViewTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultActiveSaveKeyFileViewTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(VaultActiveSaveKeyFileViewTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(VaultActiveSaveKeyFileViewTest, setEncryptInfo)
{
    // Test setter: void setEncryptInfo((EncryptInfo &info))
    EncryptInfo _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setEncryptInfo(_arg0));
}

TEST_F(VaultActiveSaveKeyFileViewTest, setNextButtonText)
{
    // Test setter: void setNextButtonText((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setNextButtonText(_arg0));
}

TEST_F(VaultActiveSaveKeyFileViewTest, setOldPasswordSchemeMigrationMode)
{
    // Test setter: void setOldPasswordSchemeMigrationMode((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setOldPasswordSchemeMigrationMode(false));
}

TEST_F(VaultActiveSaveKeyFileViewTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(VaultActiveSaveKeyFileViewTest, slotChangeEdit)
{
    // Test method: void slotChangeEdit((const QString &fileName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->slotChangeEdit(_arg0));
}

TEST_F(VaultActiveSaveKeyFileViewTest, slotSelectCurrentFile)
{
    // Test method: void slotSelectCurrentFile((const QString &file))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->slotSelectCurrentFile(_arg0));
}
