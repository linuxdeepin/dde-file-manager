// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "treeviews/sidebarview.h"
#include "treeviews/private/sidebarview_p.h"
#include "treeviews/sidebaritem.h"
#include "treemodels/sidebarmodel.h"
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/fileutils.h>

#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_sidebar;

class UT_SidebarView : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        model = new SideBarModel;
        SideBarItem *item1 = createGroupItem(QString("group1"));
        SideBarItem *item2 = createGroupItem(QString("group2"));
        model->appendRow(item1);
        model->appendRow(item2);   // two groups added.

        SideBarItem *group1_item1 = createSubItem("item1", QUrl("test/url3"), QString("group1"));
        SideBarItem *group1_item2 = createSubItem("item2", QUrl("test/url4"), QString("group1"));

        model->appendRow(group1_item1);
        model->appendRow(group1_item2);   // two items under group1

        view = new SideBarView;
        view->setModel(model);
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (model) {
            model->clear();
            delete model;
        }
        if (view)
            delete view;
    }
    SideBarItem *createGroupItem(const QString &group)
    {
        SideBarItem *item = new SideBarItemSeparator(group);
        return item;
    }
    SideBarItem *createSubItem(const QString &name, const QUrl &url, const QString &group)
    {
        QString iconName { SystemPathUtil::instance()->systemPathIconName(name) };
        QString text { name };
        if (!iconName.contains("-symbolic"))
            iconName.append("-symbolic");

        SideBarItem *item = new SideBarItem(QIcon::fromTheme(iconName),
                                            text,
                                            group,
                                            url);
        return item;
    }
    SideBarModel *model = nullptr;
    SideBarView *view = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_SidebarView, FindItemIndex)
{
    QModelIndex index1 = view->findItemIndex(QUrl("test/url3"));
    EXPECT_TRUE(index1.row() == 0);

    QModelIndex index2 = view->findItemIndex(QUrl("test/url4"));
    EXPECT_TRUE(index2.row() == 1);
}

TEST_F(UT_SidebarView, testDragEnterEvent)
{
    bool isCall { false };
    stub.set_lamda(&QDropEvent::setDropAction, []() {
        return;
    });

    stub.set_lamda(&QDropEvent::ignore, [&]() {
        isCall = true;
        return;
    });

    QDragEnterEvent event(QPoint(10, 10), Qt::IgnoreAction, nullptr, Qt::LeftButton, Qt::NoModifier);

    // action1
    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return {};
    });

    QMimeData data;
    data.setData(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey, "file:///home");
    stub.set_lamda(&QDropEvent::mimeData, [&]{
        return &data;
    });

    view->dragEnterEvent(&event);
    EXPECT_FALSE(isCall);

    // action2
    isCall = false;
    stub.set_lamda(&QMimeData::urls, []() -> QList<QUrl> {
        return { QUrl("/home/uos") };
    });

    stub.set_lamda(&FileUtils::isContainProhibitPath, []() -> bool {
        return true;
    });

    view->dragEnterEvent(&event);
    EXPECT_TRUE(isCall);
}

TEST_F(UT_SidebarView, testDragLeaveEvent)
{
    bool isCall { false };
    stub.set_lamda(&QAbstractItemView::setCurrentIndex, []() {
        return;
    });
    stub.set_lamda(&QAbstractItemView::setState, []() {
        return;
    });

    auto upDate = static_cast<void (QWidget::*)()>(&QWidget::update);
    stub.set_lamda(upDate, [&]() {
        __DBG_STUB_INVOKE__
        isCall = true;
        return;
    });

    QDragLeaveEvent event;

    view->dragLeaveEvent(&event);
    EXPECT_FALSE(isCall);
}
