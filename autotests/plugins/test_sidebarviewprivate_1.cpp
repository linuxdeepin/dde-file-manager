// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarviewprivate_1.cpp
 * @brief Unit tests for SideBarViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebarview.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarViewPrivateTest, canEnter)
{
    // Test method: bool canEnter((QDragEnterEvent *event))
    auto result = obj->canEnter(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewPrivateTest, cancelPendingMountSubscription)
{
    // Test method: void cancelPendingMountSubscription((const QUrl &deviceUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->cancelPendingMountSubscription(_arg0));
}

TEST_F(SideBarViewPrivateTest, checkOpTime)
{
    // Test bool getter: checkOpTime()
    bool result = obj->checkOpTime();
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewPrivateTest, checkTargetEnable)
{
    // Test method: bool checkTargetEnable((const QUrl &targetUrl))
    QUrl _arg0{};
    auto result = obj->checkTargetEnable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewPrivateTest, currentChanged)
{
    // Test method: DFMBASE_USE_NAMESPACE currentChanged((const QModelIndex &curIndex))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj->currentChanged(_arg0); });
}

TEST_F(SideBarViewPrivateTest, expandItem)
{
    // Test method: void expandItem((const QModelIndex &index, const QList<QUrl> &subFolders))
    QModelIndex _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->expandItem(_arg0, _arg1));
}

TEST_F(SideBarViewPrivateTest, isCursorInsideIndex)
{
    // Test method: bool isCursorInsideIndex((const QModelIndex &index, const QPoint &fallbackPos))
    QModelIndex _arg0{};
    QPoint _arg1{};
    auto result = obj->isCursorInsideIndex(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewPrivateTest, notifyOrderChanged)
{
    // Test method: void notifyOrderChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->notifyOrderChanged());
}

TEST_F(SideBarViewPrivateTest, restorePalette)
{
    // Test method: void restorePalette(())
    EXPECT_NO_FATAL_FAILURE(obj->restorePalette());
}

TEST_F(SideBarViewPrivateTest, setDragSourceIndex)
{
    // Test setter: void setDragSourceIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDragSourceIndex(_arg0));
}

TEST_F(SideBarViewPrivateTest, setTransparentPalette)
{
    // Test method: void setTransparentPalette(())
    EXPECT_NO_FATAL_FAILURE(obj->setTransparentPalette());
}

TEST_F(SideBarViewPrivateTest, updateDFMMimeData)
{
    // Test method: void updateDFMMimeData((const QDropEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->updateDFMMimeData(nullptr));
}

TEST_F(SideBarViewPrivateTest, updateHoverIndex)
{
    // Test method: void updateHoverIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateHoverIndex(_arg0));
}
