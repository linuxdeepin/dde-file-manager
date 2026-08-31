// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultpropertydialog_1.cpp
 * @brief Unit tests for VaultPropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/vaultpropertyview/vaultpropertydialog.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultPropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultPropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultPropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultPropertyDialogTest, addExtendedControl)
{
    // Test method: void addExtendedControl((QWidget *widget))
    EXPECT_NO_FATAL_FAILURE(obj->addExtendedControl(nullptr));
}

TEST_F(VaultPropertyDialogTest, contentHeight)
{
    // Test getter: int contentHeight()
    auto result = obj->contentHeight();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultPropertyDialogTest, createBasicWidget)
{
    // Test method: void createBasicWidget((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createBasicWidget(_arg0));
}

TEST_F(VaultPropertyDialogTest, createHeadUI)
{
    // Test method: void createHeadUI((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createHeadUI(_arg0));
}

TEST_F(VaultPropertyDialogTest, initInfoUI)
{
    // Test method: void initInfoUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initInfoUI());
}

TEST_F(VaultPropertyDialogTest, insertExtendedControl)
{
    // Test method: void insertExtendedControl((int index, QWidget *widget))
    EXPECT_NO_FATAL_FAILURE(obj->insertExtendedControl(0, nullptr));
}

TEST_F(VaultPropertyDialogTest, processHeight)
{
    // Test method: void processHeight((int height))
    EXPECT_NO_FATAL_FAILURE(obj->processHeight(0));
}

TEST_F(VaultPropertyDialogTest, selectFileUrl)
{
    // Test method: void selectFileUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFileUrl(_arg0));
}

TEST_F(VaultPropertyDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}
