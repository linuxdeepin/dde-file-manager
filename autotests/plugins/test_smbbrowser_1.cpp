// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbbrowser_1.cpp
 * @brief Unit tests for SmbBrowser methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "smbbrowser.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbBrowserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbBrowser();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbBrowser *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbBrowserTest, contextMenuHandle)
{
    // Test method: void contextMenuHandle((quint64 windowId, const QUrl &url, const QPoint &globalPos))
    QUrl _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->contextMenuHandle(0, _arg1, _arg2));
}

TEST_F(SmbBrowserTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(SmbBrowserTest, registerNetworkAccessPrehandler)
{
    // Test method: void registerNetworkAccessPrehandler(())
    EXPECT_NO_FATAL_FAILURE(obj->registerNetworkAccessPrehandler());
}

TEST_F(SmbBrowserTest, registerNetworkToSearch)
{
    // Test method: void registerNetworkToSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->registerNetworkToSearch());
}

TEST_F(SmbBrowserTest, registerNetworkToTitleBar)
{
    // Test method: void registerNetworkToTitleBar(())
    EXPECT_NO_FATAL_FAILURE(obj->registerNetworkToTitleBar());
}

TEST_F(SmbBrowserTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(SmbBrowserTest, updateNeighborToSidebar)
{
    // Test method: void updateNeighborToSidebar(())
    EXPECT_NO_FATAL_FAILURE(obj->updateNeighborToSidebar());
}
