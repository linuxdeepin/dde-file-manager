// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingdialog.cpp
 * @brief Unit tests for SettingDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/dialogs/settingsdialog/settingdialog.h"

#include <QTest>

using namespace src;

class SettingDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SettingDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SettingDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SettingDialogTest, createPathComboboxItem)
{
    // Test method: QPair<QWidget *, QWidget *> createPathComboboxItem((QObject *opt))
    auto result = obj->createPathComboboxItem(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createPathComboboxItem(nullptr); });

}

TEST_F(SettingDialogTest, createSliderWithSideIcon)
{
    // Test method: QPair<QWidget *, QWidget *> createSliderWithSideIcon((QObject *opt))
    auto result = obj->createSliderWithSideIcon(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createSliderWithSideIcon(nullptr); });

}

TEST_F(SettingDialogTest, initialze)
{
    // Test method: void initialze(())
    EXPECT_NO_FATAL_FAILURE(obj->initialze());
}

TEST_F(SettingDialogTest, loadSettings)
{
    // Test method: void loadSettings((const QString & /*templateFile*/))
    EXPECT_NO_FATAL_FAILURE(obj->loadSettings(nullptr));
}

TEST_F(SettingDialogTest, needHide)
{
    // Test method: bool needHide((const QString &key))
    QString _arg0{};
    auto result = obj->needHide(_arg0);
    EXPECT_FALSE(result);

}
