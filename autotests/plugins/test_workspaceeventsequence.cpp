// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspaceeventsequence.cpp
 * @brief Unit tests for WorkspaceEventSequence methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/workspaceeventsequence.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceEventSequenceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceEventSequence();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceEventSequence *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceEventSequenceTest, WorkspaceEventSequence)
{
    // Test constructor: WorkspaceEventSequence((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WorkspaceEventSequenceTest, doCheckDragTarget)
{
    // Test method: bool doCheckDragTarget((const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    auto result = obj->doCheckDragTarget(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventSequenceTest, doCheckTransparent)
{
    // Test method: bool doCheckTransparent((const QUrl &url, TransparentStatus *status))
    QUrl _arg0{};
    auto result = obj->doCheckTransparent(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventSequenceTest, doFetchCustomColumnRoles)
{
    // Test method: bool doFetchCustomColumnRoles((const QUrl &rootUrl, QList<ItemRoles> *roleList))
    QUrl _arg0{};
    auto result = obj->doFetchCustomColumnRoles(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventSequenceTest, doFetchCustomRoleDiaplayName)
{
    // Test method: bool doFetchCustomRoleDiaplayName((const QUrl &rootUrl, const ItemRoles role, QString *displayName))
    QUrl _arg0{};
    auto result = obj->doFetchCustomRoleDiaplayName(_arg0, ItemRoles(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventSequenceTest, doFetchSelectionModes)
{
    // Test method: bool doFetchSelectionModes((const QUrl &url, QList<QAbstractItemView::SelectionMode> *modes))
    QUrl _arg0{};
    auto result = obj->doFetchSelectionModes(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventSequenceTest, doIconItemLayoutText)
{
    // Test method: bool doIconItemLayoutText((const FileInfoPointer &info, dfmbase::ElideTextLayout *layout))
    FileInfoPointer _arg0{};
    auto result = obj->doIconItemLayoutText(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventSequenceTest, doPaintListItem)
{
    // Test method: bool doPaintListItem((int role, const FileInfoPointer &info, QPainter *painter, QRectF *rect))
    FileInfoPointer _arg1{};
    auto result = obj->doPaintListItem(0, _arg1, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventSequenceTest, instance)
{
    // Test getter: WorkspaceEventSequence instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
