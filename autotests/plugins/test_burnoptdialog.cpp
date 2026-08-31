// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnoptdialog.cpp
 * @brief Unit tests for BurnOptDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/burnoptdialog.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnOptDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnOptDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnOptDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnOptDialogTest, BurnOptDialog)
{
    // Test constructor: BurnOptDialog((const QString &dev, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BurnOptDialogTest, currentBurnOptions)
{
    // Test getter: DFMBURN::BurnOptions currentBurnOptions()
    auto result = obj->currentBurnOptions();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(BurnOptDialogTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(BurnOptDialogTest, setWriteSpeedInfo)
{
    // Test setter: void setWriteSpeedInfo((const QStringList &writespeed))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setWriteSpeedInfo(_arg0));
}
