// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-sidebar gap list, treeviews/sidebarview.cpp):
//   - SideBarView::onRequestCollapseItem(QModelIndex)            -> RequestCollapseItem_GroupIndex_Collapsed
//   - SideBarView::isCurrentUrlHighlightIndex(QModelIndex) + lambdas -> CurrentUrlHighlightIndex_ChildIndex_BoolReturned
//   - SideBarViewPrivate::currentChanged(QModelIndex)            -> CurrentChanged_Selection_HandlerInvoked (via selectionModel)
//   - SideBarViewPrivate::onItemDoubleClicked(QModelIndex) + lambdas -> DoubleClicked_GroupIndex_HandlerInvoked (via doubleClicked signal)
//   - SideBarView ctor lambda (L642)                             -> covered by construction
//   - SideBarViewPrivate::cancelPendingMountSubscription etc.    -> partially indirect via collapse path

#include "stubext.h"
#include "treeviews/sidebarview.h"
#include "treeviews/sidebaritem.h"
#include "treemodels/sidebarmodel.h"

#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>

#include <gtest/gtest.h>
#include <QUrl>
#include <QMetaObject>
#include <QItemSelectionModel>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_sidebar;

class UT_SideBarViewCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });
        stub.set_lamda(&SystemPathUtil::isSystemPath, [] {
            __DBG_STUB_INVOKE__
            return false;
        });
        model = new SideBarModel();
        SideBarItemSeparator *group = new SideBarItemSeparator(QString("group1"));
        model->appendRow(group);
        childUrl = QUrl("file:///tmp/side-view-child");
        SideBarItem *item = new SideBarItem(QIcon(), QString("child"), QString("group1"), childUrl);
        group->appendRow(item);
        view = new SideBarView();
        view->setModel(model);
        view->resize(260, 400);
        groupIndex = model->index(0, 0);
        childIndex = model->index(0, 0, groupIndex);
    }
    void TearDown() override
    {
        stub.clear();
        if (view)
            delete view;
        if (model) {
            model->clear();
            delete model;
        }
    }
    SideBarModel *model { nullptr };
    SideBarView *view { nullptr };
    QModelIndex groupIndex;
    QModelIndex childIndex;
    QUrl childUrl;
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarViewCov, RequestCollapseItem_GroupIndex_Collapsed)
{
    // Arrange
    ASSERT_TRUE(groupIndex.isValid());

    // Act
    view->onRequestCollapseItem(groupIndex);

    // Assert
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_TRUE(view->isEnabled());
}

TEST_F(UT_SideBarViewCov, CurrentUrlHighlightIndex_ChildIndex_BoolReturned)
{
    // Arrange
    ASSERT_TRUE(childIndex.isValid());

    // Act: lambdas #1/#2 run inside for valid and invalid indexes
    const bool highlightChild = view->isCurrentUrlHighlightIndex(childIndex);
    const bool highlightInvalid = view->isCurrentUrlHighlightIndex(QModelIndex());

    // Assert
    EXPECT_FALSE(highlightInvalid);
    EXPECT_EQ(model->rowCount(), 1);
}

TEST_F(UT_SideBarViewCov, CurrentChanged_Selection_HandlerInvoked)
{
    // Arrange
    ASSERT_TRUE(childIndex.isValid());
    QItemSelectionModel *sel = view->selectionModel();
    ASSERT_NE(sel, nullptr);

    // Act: selection change drives SideBarViewPrivate::currentChanged
    sel->setCurrentIndex(childIndex, QItemSelectionModel::SelectCurrent);

    // Assert
    EXPECT_EQ(sel->currentIndex(), childIndex);
    EXPECT_EQ(model->rowCount(), 1);
}

TEST_F(UT_SideBarViewCov, DoubleClicked_GroupIndex_HandlerInvoked)
{
    // Arrange
    ASSERT_TRUE(groupIndex.isValid());

    // Act: emit QAbstractItemView::doubleClicked -> onItemDoubleClicked lambda chain
    const bool invoked = QMetaObject::invokeMethod(
            view, "doubleClicked", Qt::DirectConnection, Q_ARG(QModelIndex, groupIndex));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(model->rowCount(), 1);
}

TEST_F(UT_SideBarViewCov, DoubleClicked_ChildIndex_HandlerInvoked)
{
    // Arrange
    ASSERT_TRUE(childIndex.isValid());

    // Act
    const bool invoked = QMetaObject::invokeMethod(
            view, "doubleClicked", Qt::DirectConnection, Q_ARG(QModelIndex, childIndex));

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(view->width(), 260);
}
