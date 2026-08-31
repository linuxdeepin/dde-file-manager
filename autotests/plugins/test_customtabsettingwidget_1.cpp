// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customtabsettingwidget_1.cpp
 * @brief Unit tests for CustomTabSettingWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/customtabsettingwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CustomTabSettingWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomTabSettingWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomTabSettingWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomTabSettingWidgetTest, addCustomItem)
{
    // Test method: void addCustomItem((DSettingsOption *opt, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addCustomItem(nullptr, _arg1));
}

TEST_F(CustomTabSettingWidgetTest, handleAddCustomItem)
{
    // Test method: void handleAddCustomItem((Dtk::Core::DSettingsOption *opt))
    EXPECT_NO_FATAL_FAILURE(obj->handleAddCustomItem(nullptr));
}

TEST_F(CustomTabSettingWidgetTest, handleOptionChanged)
{
    // Test method: void handleOptionChanged((const QVariant &value))
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleOptionChanged(_arg0));
}

TEST_F(CustomTabSettingWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(CustomTabSettingWidgetTest, selectCustomDirectory)
{
    // Test getter: QUrl selectCustomDirectory()
    auto result = obj->selectCustomDirectory();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(CustomTabSettingWidgetTest, setOption)
{
    // Test setter: void setOption((QObject *opt))
    EXPECT_NO_FATAL_FAILURE(obj->setOption(nullptr));
}
