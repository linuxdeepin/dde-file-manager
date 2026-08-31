// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspaceeventcaller_1.cpp
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

TEST_F(WorkspaceEventCallerTest, sendChangeCurrentUrl)
{
    // Test method: void sendChangeCurrentUrl((const quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendChangeCurrentUrl(0, _arg1));
}

TEST_F(WorkspaceEventCallerTest, sendCheckTabAddable)
{
    // Test method: bool sendCheckTabAddable((quint64 windowId))
    auto result = obj->sendCheckTabAddable(0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventCallerTest, sendCloseTab)
{
    // Test method: void sendCloseTab((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendCloseTab(_arg0));
}

TEST_F(WorkspaceEventCallerTest, sendEnterDirReportLog)
{
    // Test method: void sendEnterDirReportLog((const QVariantMap &data))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendEnterDirReportLog(_arg0));
}

TEST_F(WorkspaceEventCallerTest, sendModelFilesEmpty)
{
    // Test method: void sendModelFilesEmpty(())
    EXPECT_NO_FATAL_FAILURE(obj->sendModelFilesEmpty());
}

TEST_F(WorkspaceEventCallerTest, sendOpenAsAdmin)
{
    // Test method: void sendOpenAsAdmin((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenAsAdmin(_arg0));
}

TEST_F(WorkspaceEventCallerTest, sendOpenNewTab)
{
    // Test method: void sendOpenNewTab((const quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenNewTab(0, _arg1));
}

TEST_F(WorkspaceEventCallerTest, sendPaintEmblems)
{
    // Test method: void sendPaintEmblems((QPainter *painter, const QRectF &paintArea, const FileInfoPointer &info))
    QRectF _arg1{};
    FileInfoPointer _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sendPaintEmblems(nullptr, _arg1, _arg2));
}

TEST_F(WorkspaceEventCallerTest, sendShowCustomTopWidget)
{
    // Test method: void sendShowCustomTopWidget((const quint64 windowID, const QString &scheme, bool visible))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendShowCustomTopWidget(0, _arg1, false));
}

TEST_F(WorkspaceEventCallerTest, sendViewItemClicked)
{
    // Test method: bool sendViewItemClicked((const QVariantMap &data))
    QVariantMap _arg0{};
    auto result = obj->sendViewItemClicked(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventCallerTest, sendViewSelectionChanged)
{
    // Test method: void sendViewSelectionChanged((const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected))
    QItemSelection _arg1{};
    QItemSelection _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sendViewSelectionChanged(0, _arg1, _arg2));
}
