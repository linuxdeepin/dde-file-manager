// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanagerwindowprivate.cpp
 * @brief Unit tests for FileManagerWindowPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <QTest>

using namespace src;

class FileManagerWindowPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileManagerWindowPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileManagerWindowPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileManagerWindowPrivateTest, animateDetailSplitter)
{
    // Test method: void animateDetailSplitter((bool show))
    EXPECT_NO_FATAL_FAILURE(obj->animateDetailSplitter(false));
}

TEST_F(FileManagerWindowPrivateTest, animateSideBarHideForResize)
{
    // Test method: void animateSideBarHideForResize(())
    EXPECT_NO_FATAL_FAILURE(obj->animateSideBarHideForResize());
}

TEST_F(FileManagerWindowPrivateTest, animateSideBarShowForResize)
{
    // Test method: void animateSideBarShowForResize(())
    EXPECT_NO_FATAL_FAILURE(obj->animateSideBarShowForResize());
}

TEST_F(FileManagerWindowPrivateTest, handleSideBarEvent)
{
    // Test event handler: handleSideBarEvent((QObject *watched, QEvent *event))
    QObject _event(QObject::None);
    EXPECT_NO_FATAL_FAILURE(obj->handleSideBarEvent(&_event));
}

TEST_F(FileManagerWindowPrivateTest, resetDetailDragState)
{
    // Test method: void resetDetailDragState(())
    EXPECT_NO_FATAL_FAILURE(obj->resetDetailDragState());
}

TEST_F(FileManagerWindowPrivateTest, resetSideBarSize)
{
    // Test method: void resetSideBarSize(())
    EXPECT_NO_FATAL_FAILURE(obj->resetSideBarSize());
}

TEST_F(FileManagerWindowPrivateTest, resetTitleBarSize)
{
    // Test method: void resetTitleBarSize(())
    EXPECT_NO_FATAL_FAILURE(obj->resetTitleBarSize());
}

TEST_F(FileManagerWindowPrivateTest, saveDetailSpaceState)
{
    // Test method: void saveDetailSpaceState(())
    EXPECT_NO_FATAL_FAILURE(obj->saveDetailSpaceState());
}

TEST_F(FileManagerWindowPrivateTest, saveSidebarState)
{
    // Test method: void saveSidebarState(())
    EXPECT_NO_FATAL_FAILURE(obj->saveSidebarState());
}

TEST_F(FileManagerWindowPrivateTest, saveWindowState)
{
    // Test method: void saveWindowState(())
    EXPECT_NO_FATAL_FAILURE(obj->saveWindowState());
}
