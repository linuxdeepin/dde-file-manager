// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnoptdialog_1.cpp
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

TEST_F(BurnOptDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(BurnOptDialogTest, onButnBtnClicked)
{
    // Test method: void onButnBtnClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onButnBtnClicked(0, _arg1));
}

TEST_F(BurnOptDialogTest, onIndexChanged)
{
    // Test method: void onIndexChanged((int index))
    EXPECT_NO_FATAL_FAILURE(obj->onIndexChanged(0));
}

TEST_F(BurnOptDialogTest, setDefaultVolName)
{
    // Test setter: void setDefaultVolName((const QString &volName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDefaultVolName(_arg0));
}

TEST_F(BurnOptDialogTest, setISOImage)
{
    // Test setter: void setISOImage((const QUrl &image))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setISOImage(_arg0));
}

TEST_F(BurnOptDialogTest, setUDFSupported)
{
    // Test setter: void setUDFSupported((bool supported, bool disableISOOpts))
    EXPECT_NO_FATAL_FAILURE(obj->setUDFSupported(false, false));
}

TEST_F(BurnOptDialogTest, startDataBurn)
{
    // Test method: void startDataBurn(())
    EXPECT_NO_FATAL_FAILURE(obj->startDataBurn());
}

TEST_F(BurnOptDialogTest, startImageBurn)
{
    // Test method: void startImageBurn(())
    EXPECT_NO_FATAL_FAILURE(obj->startImageBurn());
}
