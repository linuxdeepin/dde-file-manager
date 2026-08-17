// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "events/workspaceeventsequence.h"
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <QUrl>
#include <QPainter>
#include <QRectF>
#include <QAbstractItemView>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

class WorkspaceEventSequenceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        testUrl = QUrl("file:///test.txt");
        testInfo = InfoFactory::create<FileInfo>(testUrl);
    }

    void TearDown() override
    {
        stub.clear();
    }

    QUrl testUrl;
    FileInfoPointer testInfo;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceEventSequenceTest, Instance_ReturnsSingleton)
{
    // Test that instance() returns same singleton instance
    WorkspaceEventSequence *instance1 = WorkspaceEventSequence::instance();
    WorkspaceEventSequence *instance2 = WorkspaceEventSequence::instance();
    
    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(WorkspaceEventSequenceTest, Constructor_CreatesValidObject)
{
    // Test that constructor creates valid object
    EXPECT_NO_THROW({
        WorkspaceEventSequence *sequence = new WorkspaceEventSequence();
        delete sequence;
    });
}

TEST_F(WorkspaceEventSequenceTest, Destructor_DoesNotCrash)
{
    // Test that destructor does not crash
    EXPECT_NO_THROW({
        WorkspaceEventSequence *sequence = new WorkspaceEventSequence();
        delete sequence;
    });
}

TEST_F(WorkspaceEventSequenceTest, DoPaintListItem_ValidParameters_DoesNotCrash)
{
    // Test painting list item with valid parameters
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    int role = static_cast<int>(Global::ItemRoles::kItemNameRole);
    
    EXPECT_NO_THROW(sequence->doPaintListItem(role, testInfo, nullptr, nullptr));
}

TEST_F(WorkspaceEventSequenceTest, DoPaintListItem_DifferentRoles_DoesNotCrash)
{
    // Test painting list item with different roles
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    
    // Test with various roles
    EXPECT_NO_THROW(sequence->doPaintListItem(static_cast<int>(Global::ItemRoles::kItemNameRole), testInfo, nullptr, nullptr));
    EXPECT_NO_THROW(sequence->doPaintListItem(static_cast<int>(Global::ItemRoles::kItemFileSizeRole), testInfo, nullptr, nullptr));
    EXPECT_NO_THROW(sequence->doPaintListItem(static_cast<int>(Global::ItemRoles::kItemFileLastModifiedRole), testInfo, nullptr, nullptr));
}

TEST_F(WorkspaceEventSequenceTest, DoPaintListItem_NullInfo_DoesNotCrash)
{
    // Test painting list item with null info
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    int role = static_cast<int>(Global::ItemRoles::kItemNameRole);
    
    EXPECT_NO_THROW(sequence->doPaintListItem(role, nullptr, nullptr, nullptr));
}

TEST_F(WorkspaceEventSequenceTest, DoIconItemLayoutText_ValidInfo_DoesNotCrash)
{
    // Test icon item layout text with valid info
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    dfmbase::ElideTextLayout *layout = nullptr;
    
    EXPECT_NO_THROW(sequence->doIconItemLayoutText(testInfo, layout));
}

TEST_F(WorkspaceEventSequenceTest, DoIconItemLayoutText_NullInfo_DoesNotCrash)
{
    // Test icon item layout text with null info
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    dfmbase::ElideTextLayout *layout = nullptr;
    
    EXPECT_NO_THROW(sequence->doIconItemLayoutText(nullptr, layout));
}

TEST_F(WorkspaceEventSequenceTest, DoCheckDragTarget_ValidParameters_DoesNotCrash)
{
    // Test checking drag target with valid parameters
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    QList<QUrl> urls;
    urls << QUrl("file:///test1.txt") << QUrl("file:///test2.txt");
    QUrl targetUrl("file:///target/");
    Qt::DropAction action = Qt::CopyAction;
    
    EXPECT_NO_THROW(sequence->doCheckDragTarget(urls, targetUrl, &action));
}

TEST_F(WorkspaceEventSequenceTest, DoCheckDragTarget_EmptyUrls_DoesNotCrash)
{
    // Test checking drag target with empty URLs
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    QList<QUrl> urls;
    QUrl targetUrl("file:///target/");
    Qt::DropAction action = Qt::CopyAction;
    
    EXPECT_NO_THROW(sequence->doCheckDragTarget(urls, targetUrl, &action));
}

TEST_F(WorkspaceEventSequenceTest, DoCheckDragTarget_NullAction_DoesNotCrash)
{
    // Test checking drag target with null action
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    QList<QUrl> urls;
    urls << QUrl("file:///test1.txt");
    QUrl targetUrl("file:///target/");
    
    EXPECT_NO_THROW(sequence->doCheckDragTarget(urls, targetUrl, nullptr));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchSelectionModes_ValidUrl_DoesNotCrash)
{
    // Test fetching selection modes with valid URL
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    QList<QAbstractItemView::SelectionMode> modes;
    
    EXPECT_NO_THROW(sequence->doFetchSelectionModes(testUrl, &modes));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchSelectionModes_NullModes_DoesNotCrash)
{
    // Test fetching selection modes with null modes
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    
    EXPECT_NO_THROW(sequence->doFetchSelectionModes(testUrl, nullptr));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchSelectionModes_EmptyUrl_DoesNotCrash)
{
    // Test fetching selection modes with empty URL
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    QList<QAbstractItemView::SelectionMode> modes;
    QUrl emptyUrl;
    
    EXPECT_NO_THROW(sequence->doFetchSelectionModes(emptyUrl, &modes));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchCustomColumnRoles_ValidUrl_DoesNotCrash)
{
    // Test fetching custom column roles with valid URL
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    QList<Global::ItemRoles> roleList;
    
    EXPECT_NO_THROW(sequence->doFetchCustomColumnRoles(testUrl, &roleList));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchCustomColumnRoles_NullRoleList_DoesNotCrash)
{
    // Test fetching custom column roles with null role list
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    
    EXPECT_NO_THROW(sequence->doFetchCustomColumnRoles(testUrl, nullptr));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchCustomColumnRoles_EmptyUrl_DoesNotCrash)
{
    // Test fetching custom column roles with empty URL
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    QList<Global::ItemRoles> roleList;
    QUrl emptyUrl;
    
    EXPECT_NO_THROW(sequence->doFetchCustomColumnRoles(emptyUrl, &roleList));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchCustomRoleDiaplayName_ValidParameters_DoesNotCrash)
{
    // Test fetching custom role display name with valid parameters
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    Global::ItemRoles role = Global::ItemRoles::kItemNameRole;
    QString displayName;
    
    EXPECT_NO_THROW(sequence->doFetchCustomRoleDiaplayName(testUrl, role, &displayName));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchCustomRoleDiaplayName_NullDisplayName_DoesNotCrash)
{
    // Test fetching custom role display name with null display name
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    Global::ItemRoles role = Global::ItemRoles::kItemNameRole;
    
    EXPECT_NO_THROW(sequence->doFetchCustomRoleDiaplayName(testUrl, role, nullptr));
}

TEST_F(WorkspaceEventSequenceTest, DoFetchCustomRoleDiaplayName_DifferentRoles_DoesNotCrash)
{
    // Test fetching custom role display name with different roles
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    QString displayName;
    
    EXPECT_NO_THROW(sequence->doFetchCustomRoleDiaplayName(testUrl, Global::ItemRoles::kItemNameRole, &displayName));
    EXPECT_NO_THROW(sequence->doFetchCustomRoleDiaplayName(testUrl, Global::ItemRoles::kItemFileSizeRole, &displayName));
    EXPECT_NO_THROW(sequence->doFetchCustomRoleDiaplayName(testUrl, Global::ItemRoles::kItemFileLastModifiedRole, &displayName));
}

TEST_F(WorkspaceEventSequenceTest, DoCheckTransparent_ValidUrl_DoesNotCrash)
{
    // Test checking transparent with valid URL
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    Global::TransparentStatus status;
    
    EXPECT_NO_THROW(sequence->doCheckTransparent(testUrl, &status));
}

TEST_F(WorkspaceEventSequenceTest, DoCheckTransparent_NullStatus_DoesNotCrash)
{
    // Test checking transparent with null status
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    
    EXPECT_NO_THROW(sequence->doCheckTransparent(testUrl, nullptr));
}

TEST_F(WorkspaceEventSequenceTest, DoCheckTransparent_EmptyUrl_DoesNotCrash)
{
    // Test checking transparent with empty URL
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    Global::TransparentStatus status;
    QUrl emptyUrl;
    
    EXPECT_NO_THROW(sequence->doCheckTransparent(emptyUrl, &status));
}

TEST_F(WorkspaceEventSequenceTest, DoCheckTransparent_DifferentUrls_DoesNotCrash)
{
    // Test checking transparent with different URLs
    WorkspaceEventSequence *sequence = WorkspaceEventSequence::instance();
    Global::TransparentStatus status;
    
    EXPECT_NO_THROW(sequence->doCheckTransparent(QUrl("file:///test.txt"), &status));
    EXPECT_NO_THROW(sequence->doCheckTransparent(QUrl("file:///directory/"), &status));
    EXPECT_NO_THROW(sequence->doCheckTransparent(QUrl("trash:///"), &status));
}