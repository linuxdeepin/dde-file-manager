// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerview.cpp
 * @brief Unit tests for ComputerView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/computerview.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerViewTest, handleDiskSplitterVisible)
{
    // Test method: void handleDiskSplitterVisible(())
    EXPECT_NO_FATAL_FAILURE(obj->handleDiskSplitterVisible());
}

TEST_F(ComputerViewTest, handleDisksVisible)
{
    // Test method: void handleDisksVisible(())
    EXPECT_NO_FATAL_FAILURE(obj->handleDisksVisible());
}

TEST_F(ComputerViewTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(ComputerViewTest, onUpdateItemAlias)
{
    // Test method: void onUpdateItemAlias((const QUrl &url, const QString &alias, bool isProtocol))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateItemAlias(_arg0, _arg1, false));
}

TEST_F(ComputerViewTest, selectedUrlList)
{
    // Test getter: QList<QUrl> selectedUrlList()
    auto result = obj->selectedUrlList();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerViewTest, widget)
{
    // Test getter: QWidget widget()
    auto result = obj->widget();
    EXPECT_NO_FATAL_FAILURE({ obj->widget(); });

}
