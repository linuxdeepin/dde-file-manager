// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-sidebar/treemodels/sidebarmodel.h"
#include "plugins/filemanager/core/dfmplugin-sidebar/treeviews/sidebaritem.h"

#include <dfm-base/utils/systempathutil.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPSIDEBAR_USE_NAMESPACE

class UT_SideBarModel : public testing::Test
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
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (model) {
            model->clear();
            delete model;
        }
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

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarModel, AppendGroupItem)
{
    SideBarItem *item = createGroupItem(QString("group3"));
    EXPECT_TRUE(model->rowCount() == 2);
    model->appendRow(item);
    EXPECT_TRUE(model->rowCount() == 3);
}

TEST_F(UT_SideBarModel, AppendSubItem)
{
    SideBarItem *item = createSubItem("item", QUrl("test/url6"), QString("group1"));
    EXPECT_TRUE(model->rowCount(model->index(0, 0)) == 2);
    model->appendRow(item);   //item is append under group1
    EXPECT_TRUE(model->rowCount(model->index(0, 0)) == 3);
}

TEST_F(UT_SideBarModel, InsertSubRow)
{
    SideBarItem *item = createSubItem("item123456", QUrl("test/url6"), QString("group1"));
    EXPECT_TRUE(model->rowCount(model->index(0, 0)) == 2);

    model->insertRow(1, item);   //item would be insert under group1 and before the second item.
    EXPECT_TRUE(model->rowCount(model->index(0, 0)) == 3);
    EXPECT_TRUE(model->index(1, 0, model->index(0, 0)).data().toString() == "item123456");
}

TEST_F(UT_SideBarModel, RemoveRowWithUrl)
{
    EXPECT_TRUE(model->rowCount(model->index(0, 0)) == 2);
    bool re = model->removeRow(QUrl("test/url3"));
    EXPECT_TRUE(re);
    EXPECT_TRUE(model->rowCount(model->index(0, 0)) == 1);
}
