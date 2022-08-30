/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-sidebar/treeviews/sidebarview.h"
#include "plugins/filemanager/core/dfmplugin-sidebar/treemodels/sidebarmodel.h"
#include "plugins/filemanager/core/dfmplugin-sidebar/treeviews/sidebaritem.h"

#include "dfm-base/utils/systempathutil.h"

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPSIDEBAR_USE_NAMESPACE

class UT_SidebarView : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        model = new SideBarModel;
        SideBarItem *item1 = createGroupItem(QString("group1"));
        SideBarItem *item2 = createGroupItem(QString("group2"));
        model->appendRow(item1);
        model->appendRow(item2);   //two groups added.

        SideBarItem *group1_item1 = createSubItem("item1", QUrl("test/url3"), QString("group1"));
        SideBarItem *group1_item2 = createSubItem("item2", QUrl("test/url4"), QString("group1"));

        model->appendRow(group1_item1);
        model->appendRow(group1_item2);   //two items under group1

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

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SidebarView, FindItemIndex)
{
    QModelIndex index1 = view->findItemIndex(QUrl("test/url3"));
    EXPECT_TRUE(index1.row() == 0);

    QModelIndex index2 = view->findItemIndex(QUrl("test/url4"));
    EXPECT_TRUE(index2.row() == 1);
}
