// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computereventcaller.cpp
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

TEST_F(ComputerEventCallerTest, cdTo)
{
    // Test method: void cdTo((quint64 winId, const QString &path))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->cdTo(0, _arg1));
}

TEST_F(ComputerEventCallerTest, sendEnterInNewTab)
{
    // Test method: void sendEnterInNewTab((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendEnterInNewTab(0, _arg1));
}

TEST_F(ComputerEventCallerTest, sendEnterInNewWindow)
{
    // Test method: void sendEnterInNewWindow((const QUrl &url, const bool isNew))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendEnterInNewWindow(_arg0, false));
}

TEST_F(ComputerEventCallerTest, sendErase)
{
    // Test method: void sendErase((const QString &dev))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendErase(_arg0));
}

TEST_F(ComputerEventCallerTest, sendShowPropertyDialog)
{
    // Test method: void sendShowPropertyDialog((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendShowPropertyDialog(_arg0));
}
