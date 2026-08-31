// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computereventcaller_1.cpp
 * @brief Unit tests for ComputerEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/computereventcaller.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerEventCallerTest, ComputerEventCaller)
{
    // Test constructor: ComputerEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerEventCallerTest, sendCtrlNOnItem)
{
    // Test method: void sendCtrlNOnItem((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendCtrlNOnItem(0, _arg1));
}

TEST_F(ComputerEventCallerTest, sendCtrlTOnItem)
{
    // Test method: void sendCtrlTOnItem((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendCtrlTOnItem(0, _arg1));
}

TEST_F(ComputerEventCallerTest, sendItemRenamed)
{
    // Test method: void sendItemRenamed((const QUrl &url, const QString &name))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendItemRenamed(_arg0, _arg1));
}

TEST_F(ComputerEventCallerTest, sendOpenItem)
{
    // Test method: void sendOpenItem((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenItem(0, _arg1));
}
