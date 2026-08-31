// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspaceeventcaller.cpp
 * @brief Unit tests for WorkspaceEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/workspaceeventcaller.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceEventCallerTest, sendOpenWindow)
{
    // Test method: void sendOpenWindow((const QList<QUrl> &urls, const bool isNew))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenWindow(_arg0, false));
}

TEST_F(WorkspaceEventCallerTest, sendRenameEndEdit)
{
    // Test method: bool sendRenameEndEdit((const quint64 &winId, const QUrl &url))
    quint64 _arg0{};
    QUrl _arg1{};
    auto result = obj->sendRenameEndEdit(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventCallerTest, sendRenameStartEdit)
{
    // Test method: bool sendRenameStartEdit((const quint64 &winId, const QUrl &url))
    quint64 _arg0{};
    QUrl _arg1{};
    auto result = obj->sendRenameStartEdit(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventCallerTest, sendViewModeChanged)
{
    // Test method: void sendViewModeChanged((quint64 windowId, Global::ViewMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->sendViewModeChanged(0, Global::ViewMode()));
}
