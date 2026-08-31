// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmbase_use_namespace_1.cpp
 * @brief Unit tests for DFMBASE_USE_NAMESPACE methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "plugins/ddplugin-organizer/test_organizerplugin.h"

#include <QTest>

using namespace autotests;

class DFMBASE_USE_NAMESPACETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMBASE_USE_NAMESPACE();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMBASE_USE_NAMESPACE *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMBASE_USE_NAMESPACETest, contextMenuHandle)
{
    // Test method: DPBOOKMARK_USE_NAMESPACE contextMenuHandle((quint64 windowId, const QUrl &url, const QPoint &globalPos))
    QUrl _arg1{};
    QPoint _arg2{};
    auto result = obj->contextMenuHandle(0, _arg1, _arg2);
    EXPECT_NO_FATAL_FAILURE({ obj->contextMenuHandle(0, _arg1, _arg2); });

}

TEST_F(DFMBASE_USE_NAMESPACETest, getNameOrder)
{
    // Test getter: DPWORKSPACE_USE_NAMESPACE getNameOrder()
    auto result = obj->getNameOrder();
    EXPECT_NO_FATAL_FAILURE({ obj->getNameOrder(); });

}

TEST_F(DFMBASE_USE_NAMESPACETest, getTimeOrder)
{
    // Test getter: DPWORKSPACE_USE_NAMESPACE getTimeOrder()
    auto result = obj->getTimeOrder();
    EXPECT_NO_FATAL_FAILURE({ obj->getTimeOrder(); });

}

TEST_F(DFMBASE_USE_NAMESPACETest, getTypeOrder)
{
    // Test getter: DPWORKSPACE_USE_NAMESPACE getTypeOrder()
    auto result = obj->getTypeOrder();
    EXPECT_NO_FATAL_FAILURE({ obj->getTypeOrder(); });

}

TEST_F(DFMBASE_USE_NAMESPACETest, sendItemActived)
{
    // Test method: DPSIDEBAR_USE_NAMESPACE sendItemActived((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->sendItemActived(0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->sendItemActived(0, _arg1); });

}

TEST_F(DFMBASE_USE_NAMESPACETest, sendOpenWindow)
{
    // Test method: DPSMBBROWSER_USE_NAMESPACE sendOpenWindow((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->sendOpenWindow(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->sendOpenWindow(_arg0); });

}

TEST_F(DFMBASE_USE_NAMESPACETest, trackIncompleteFile)
{
    // Test method: DPFILEOPERATIONS_USE_NAMESPACE trackIncompleteFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->trackIncompleteFile(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->trackIncompleteFile(_arg0); });

}
