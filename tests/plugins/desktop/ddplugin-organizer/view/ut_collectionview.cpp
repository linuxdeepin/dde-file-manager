/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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


#include "view/collectionview_p.h"
#include "view/collectionview.h"
#include "delegate/collectionitemdelegate.h"
#include "interface/canvasmanagershell.h"
#include "mode/normalized/type/typeclassifier.h"
#include "models/fileproxymodel_p.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <DApplication>

#include <QMenu>
#include <QItemSelection>
#include <QItemSelectionRange>

using namespace testing;
using namespace ddplugin_organizer;

class CollectionViewPrivateTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

class CollectionViewTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(CollectionViewPrivateTest, helpAction) {

    using namespace Dtk::Widget;

    typedef void (*fptr)(DApplication*);
    fptr utHandleHelp = (fptr)((void(DApplication::*)())&DApplication::handleHelpAction);

    bool isCall = false;
    stub.set_lamda(utHandleHelp, [&] () {
        isCall = true;
    });

    EXPECT_FALSE(isCall);

    QString testUuid("testUuid");
    CollectionView view(testUuid, nullptr);
    view.d->helpAction();
    EXPECT_TRUE(isCall);
}

TEST_F(CollectionViewPrivateTest, onIconSizeChanged) {

    QString testUuid("testUuid");
    CollectionView view(testUuid, nullptr);

    int setLevel = -1;
    stub.set_lamda(ADDR(CollectionItemDelegate, setIconLevel), [&] (CollectionItemDelegate *obj, int lv) {
        Q_UNUSED(obj)

        setLevel = lv;
        return lv;
    });

    view.d->onIconSizeChanged(1);

    EXPECT_EQ(setLevel, 1);
}

TEST_F(CollectionViewTest, setCanvasManagerShell) {

    QString testUuid("testUuid");
    CollectionView view(testUuid, nullptr);

    CanvasManagerShell *sh = nullptr;

    EXPECT_EQ(view.d->canvasManagerShell, nullptr);

    view.setCanvasManagerShell(sh);
    EXPECT_EQ(view.d->canvasManagerShell, nullptr);

    int setLevel = -1;
    bool isCallSetIconLevel = false;
    stub.set_lamda(ADDR(CollectionItemDelegate, setIconLevel), [&] (CollectionItemDelegate *obj, int lv) {
        Q_UNUSED(obj)

        isCallSetIconLevel = true;
        setLevel = lv;
        return lv;
    });

    bool isCallIconLevel = false;
    stub.set_lamda(ADDR(CanvasManagerShell, iconLevel), [&] () {
        isCallIconLevel = true;
        return 3;
    });

    sh = new CanvasManagerShell;
    view.setCanvasManagerShell(sh);
    EXPECT_TRUE(isCallIconLevel);
    EXPECT_TRUE(isCallSetIconLevel);

    isCallIconLevel = false;
    isCallSetIconLevel = false;
    view.setCanvasManagerShell(nullptr);
    EXPECT_FALSE(isCallIconLevel);
    EXPECT_FALSE(isCallSetIconLevel);

    delete sh;
    sh = nullptr;
}

TEST_F(CollectionViewTest, scrollContentsBy)
{
    CollectionView view("dd", nullptr);
    int dx = -1;
    int dy = -1;
    QWidget *port = nullptr;
    stub.set_lamda((void (QWidget::*)(int, int))&QWidget::scroll, [&](QWidget *self, int x, int y) {
        port = self;
        dx = x;
        dy = y;
        return ;
    });

    view.scrollContentsBy(0, 100);
    EXPECT_EQ(port, view.viewport());
    EXPECT_EQ(dx, 0);
    EXPECT_EQ(dy, 100);
}

class TestProvider : public CollectionDataProvider
{
public:
    TestProvider() : CollectionDataProvider(nullptr)
    {

    }
protected:
    QString replace(const QUrl &oldUrl, const QUrl &newUrl) {return "";}
    QString append(const QUrl &) {return "";}
    QString prepend(const QUrl &){return "";}
    void insert(const QUrl &, const QString &, const int) {}
    QString remove(const QUrl &) {return "";}
    QString change(const QUrl &) {return "";}
};

TEST_F(CollectionViewTest, sort)
{
    TestProvider test;
    CollectionView view("dd", &test);
    bool callLess = false;
    stub.set_lamda(&CollectionView::lessThan, [&callLess]() {
        callLess = true;
        return false;
    });

    ASSERT_EQ(view.d->sortRole, (int)DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    ASSERT_EQ(view.d->sortOrder, Qt::DescendingOrder);

    view.sort(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_EQ(view.d->sortRole, (int)DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_EQ(view.d->sortOrder, Qt::AscendingOrder);

    view.sort(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_EQ(view.d->sortRole, (int)DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_EQ(view.d->sortOrder, Qt::DescendingOrder);

    view.sort(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole);
    EXPECT_EQ(view.d->sortRole, (int)DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole);
    EXPECT_EQ(view.d->sortOrder, Qt::AscendingOrder);
}

TEST_F(CollectionViewTest, updateSelection)
{
    TestProvider test;
    QUrl url1("file:///usr");
    QUrl url2("file:///etc");

    stub.set_lamda(VADDR(TestProvider, items), [url1,url2]() {
        return QList<QUrl>{url2};
    });

    CollectionView view("dd", &test);
    FileProxyModel model;
    model.d->fileList = {url1, url2};
    model.d->fileMap.insert(url1, nullptr);
    model.d->fileMap.insert(url2, nullptr);

    QModelIndex idx1(0, 0, (void *)nullptr, &model);
    QModelIndex idx2(1, 0, (void *)nullptr, &model);

    stub.set_lamda(VADDR(QAbstractProxyModel,flags), [](){
        return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    });

    view.setModel(&model);
    {
        QItemSelection sel;
        sel << QItemSelectionRange(idx1);
        sel << QItemSelectionRange(idx2);

        view.selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
        auto idxs = view.selectedIndexes();
        ASSERT_EQ(idxs.size(), 2);
        ASSERT_EQ(idxs.first(), idx1);
        ASSERT_EQ(idxs.last(), idx2);
        ASSERT_TRUE(idx1.isValid());
        ASSERT_TRUE(idx2.isValid());

        // make sure model::index is working.
        QItemSelection s1;
        s1 << QItemSelectionRange(idx1);
        QItemSelection newSelection = s1;
        newSelection.merge(s1, QItemSelectionModel::Deselect);
        ASSERT_EQ(newSelection.size(), 0);
    }

    view.d->updateSelection();

    auto idxs = view.selectedIndexes();
    ASSERT_EQ(idxs.size(), 1);
    ASSERT_EQ(idxs.first(), idx2);
}

TEST_F(CollectionViewTest, keyPressEvent_space)
{
    TestProvider test;
    CollectionView view("dd", &test);
    bool preview = false;
    stub.set_lamda(&CollectionViewPrivate::previewFiles, [&preview](){
        preview = true;;
    });

    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier,"", true);
        view.keyPressEvent(&key);
        EXPECT_FALSE(preview);
    }

    {
        preview = false;
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier,"", false);
        view.keyPressEvent(&key);
        EXPECT_TRUE(preview);
    }
}
