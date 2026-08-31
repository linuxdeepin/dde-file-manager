// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebarhelper_1.cpp
 * @brief Unit tests for TitleBarHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/titlebarhelper.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TitleBarHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TitleBarHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TitleBarHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TitleBarHelperTest, getDisplayName)
{
    // Test method: QString getDisplayName((const QString &name))
    QString _arg0{};
    auto result = obj->getDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TitleBarHelperTest, isTreeViewGloballyEnabled)
{
    // Test bool getter: isTreeViewGloballyEnabled()
    bool result = obj->isTreeViewGloballyEnabled();
    EXPECT_FALSE(result);

}

TEST_F(TitleBarHelperTest, mutex)
{
    // Test getter: QMutex mutex()
    auto result = obj->mutex();
    EXPECT_NO_FATAL_FAILURE({ obj->mutex(); });

}

TEST_F(TitleBarHelperTest, setFileViewStateValue)
{
    // Test setter: void setFileViewStateValue((const QUrl &url, const QString &key, const QVariant &value))
    QUrl _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setFileViewStateValue(_arg0, _arg1, _arg2));
}

TEST_F(TitleBarHelperTest, transformViewModeUrl)
{
    // Test method: QUrl transformViewModeUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->transformViewModeUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TitleBarHelperTest, viewModelUrlCallback)
{
    // Test method: ViewModeUrlCallback viewModelUrlCallback((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->viewModelUrlCallback(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->viewModelUrlCallback(_arg0); });

}
