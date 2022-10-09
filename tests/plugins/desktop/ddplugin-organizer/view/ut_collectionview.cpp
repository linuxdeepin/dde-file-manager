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
#include "models/collectionmodel_p.h"

#include "dfm-base/dfm_global_defines.h"
#include "stubext.h"

#include <gtest/gtest.h>
#include <DApplication>

#include <QMenu>
#include <QItemSelection>
#include <QItemSelectionRange>

using namespace testing;
using namespace dfmbase;
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

TEST_F(CollectionViewTest, keyPressEvent_I)
{
    TestProvider test;
    CollectionView view("dd", &test);
    bool show = false;
    stub.set_lamda(&CollectionViewPrivate::showFilesProperty, [&show](){
        show = true;;
    });

    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_I, Qt::ControlModifier,"", false);
        view.keyPressEvent(&key);
        EXPECT_TRUE(show);
    }
}

TEST_F(CollectionViewTest, dataProvider)
{
    TestProvider test;
    CollectionView view("dd", &test);
    EXPECT_EQ(&test, view.dataProvider());
}

TEST_F(CollectionViewTest, keyboardSearch)
{
    TestProvider test;
    CollectionView view("dd", &test);
    bool find = false;
    stub.set_lamda(&CollectionViewPrivate::findIndex, [&find](){
        find = true;
        return QModelIndex();
    });

    ASSERT_NE(view.d->searchTimer, nullptr);
    view.d->searchKeys.clear();

    view.keyboardSearch("");
    EXPECT_TRUE(view.d->searchKeys.isEmpty());
    EXPECT_FALSE(find);
    EXPECT_FALSE(view.d->searchTimer->isActive());

    view.d->searchTimer->stop();
    view.d->searchKeys.clear();
    find = false;

    view.keyboardSearch("1");
    QTimer wait;
    QEventLoop loop;
    loop.connect(&wait, &QTimer::timeout, &loop, &QEventLoop::quit);
    wait.start(220);
    EXPECT_EQ(view.d->searchKeys, QString("1"));
    EXPECT_TRUE(find);
    ASSERT_TRUE(view.d->searchTimer->isActive());

    loop.exec();
    EXPECT_TRUE(view.d->searchKeys.isEmpty());
}

TEST_F(CollectionViewTest, isIndexHidden)
{
    TestProvider test;
    QUrl url1("file:///usr");
    QUrl url2("file:///etc");

    stub.set_lamda(VADDR(TestProvider, contains), [url2](CollectionDataProvider *self, const QString &key, const QUrl &url) {
        return key == "dd" && url == url2;
    });

    CollectionView view("dd", &test);
    CollectionModel model;
    model.d->fileList = {url1, url2};
    model.d->fileMap.insert(url1, nullptr);
    model.d->fileMap.insert(url2, nullptr);

    view.setModel(&model);

    QModelIndex idx1(0, 0, (void *)nullptr, &model);
    QModelIndex idx2(1, 0, (void *)nullptr, &model);

    EXPECT_TRUE(view.isIndexHidden(idx1));
    EXPECT_FALSE(view.isIndexHidden(idx2));
}

TEST_F(CollectionViewTest, selectedIndexes)
{
    TestProvider test;
    QUrl url1("file:///usr");
    QUrl url2("file:///etc");

    stub.set_lamda(VADDR(TestProvider, items), [url1,url2]() {
        return QList<QUrl>{url2};
    });

    stub.set_lamda(VADDR(TestProvider, contains), [url2](CollectionDataProvider *self, const QString &key, const QUrl &url) {
        return key == "dd" && url == url2;
    });

    CollectionView view("dd", &test);
    CollectionModel model;
    model.d->fileList = {url1, url2};
    model.d->fileMap.insert(url1, nullptr);
    model.d->fileMap.insert(url2, nullptr);

    QModelIndex idx1(0, 0, (void *)nullptr, &model);
    QModelIndex idx2(1, 0, (void *)nullptr, &model);

    stub.set_lamda(VADDR(QAbstractProxyModel,flags), [](){
        return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    });

    view.setModel(&model);

    view.selectionModel()->select(QItemSelection(idx1, idx2), QItemSelectionModel::ClearAndSelect);
    ASSERT_EQ(view.selectionModel()->selectedIndexes().size(), 2);

    auto idxs = view.selectedIndexes();
    ASSERT_EQ(idxs.size(), 1);
    ASSERT_EQ(idxs.first(), idx2);
}

TEST_F(CollectionViewTest, selectRect)
{
    TestProvider test;
    CollectionView view("dd", &test);
    CollectionModel model;
    view.setModel(&model);

    stub.set_lamda(&CollectionViewPrivate::selection,[](){
        return QItemSelection();
    });

    QItemSelectionModel::SelectionFlags flag;
    stub.set_lamda((void (*)(QItemSelectionModel *, const QItemSelection &, QItemSelectionModel::SelectionFlags))
                   ((void (QItemSelectionModel::*)(const QItemSelection &, QItemSelectionModel::SelectionFlags))&QItemSelectionModel::select),
                   [&flag](QItemSelectionModel *self, const QItemSelection &, QItemSelectionModel::SelectionFlags cmd){
        flag = cmd;
    });

    Qt::KeyboardModifiers key = Qt::NoModifier;
    stub.set_lamda(&QGuiApplication::keyboardModifiers,[&key](){
        return key;
    });

    QRect r(QPoint(0,0), view.size());
    view.d->selectRect(r);
    EXPECT_EQ(flag, QItemSelectionModel::ClearAndSelect);

    key = Qt::ControlModifier;
    view.d->selectRect(r);
    EXPECT_EQ(flag, QItemSelectionModel::ToggleCurrent);

    key = Qt::ShiftModifier;
    view.d->selectRect(r);
    EXPECT_EQ(flag, QItemSelectionModel::SelectCurrent);
}

TEST_F(CollectionViewTest, findIndex)
{
    TestProvider test;
    QUrl url1("file:///usr");
    QUrl url2("file:///un");

    stub.set_lamda(VADDR(TestProvider, items), [url1,url2]() {
        return QList<QUrl>{url1, url2};
    });

    CollectionView view("dd", &test);
    CollectionModel model;
    model.d->fileList = {url1, url2};
    model.d->fileMap.insert(url1, nullptr);
    model.d->fileMap.insert(url2, nullptr);

    view.setModel(&model);

    QModelIndex idx1(0, 0, (void *)nullptr, &model);
    QModelIndex idx2(1, 0, (void *)nullptr, &model);

    stub.set_lamda(VADDR(CollectionModel, data), [idx1,idx2](CollectionModel *, const QModelIndex &index, int role) {
        QVariant var;
        if (role ==  Global::ItemRoles::kItemFilePinyinNameRole) {
           if (index == idx1)
               var = QVariant::fromValue(QString("usr"));
           else if (index == idx2)
               var = QVariant::fromValue(QString("un"));
        }
        return var;
    });

    EXPECT_EQ(view.d->findIndex("u", true, QModelIndex(), false, false), idx1);
    EXPECT_EQ(view.d->findIndex("u", true, idx1, false, true), idx2);
}
