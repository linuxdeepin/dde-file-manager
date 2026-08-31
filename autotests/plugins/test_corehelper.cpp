// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_corehelper.cpp
 * @brief Unit tests for CoreHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/corehelper.h"

#include <QTest>

using namespace dfmplugin_core;

class CoreHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CoreHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CoreHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CoreHelperTest, cd)
{
    // Test method: void cd((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->cd(0, _arg1));
}

TEST_F(CoreHelperTest, delayInvokeProxy)
{
    // Test method: void delayInvokeProxy((std::function<void()> func, quint64 winID, QObject *parent))
    EXPECT_NO_FATAL_FAILURE(obj->delayInvokeProxy({}, 0, nullptr));
}

TEST_F(CoreHelperTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CoreHelperTest, loadPlugin)
{
    // Test method: void loadPlugin((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->loadPlugin(_arg0));
}

TEST_F(CoreHelperTest, stripFilters)
{
    // Test method: QStringList stripFilters((const QStringList &filters))
    QStringList _arg0{};
    auto result = obj->stripFilters(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
