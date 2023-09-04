// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/operator/keyselector.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "utils/keyutil.h"
#include "grid/canvasgrid.h"

#include "stubext.h"

#include <gtest/gtest.h>



using namespace ddplugin_canvas;

class UT_KeySelector : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        view = new CanvasView();
        viewP = new CanvasViewPrivate(view);
        key = new KeySelector(view);

    }
    virtual void TearDown() override
    {
        delete key;
        delete viewP;
        delete view;
        stub.clear();
    }
    CanvasView *view = nullptr;
    CanvasViewPrivate *viewP = nullptr;
    KeySelector *key = nullptr;
    stub_ext::StubExt stub ;
};

TEST_F(UT_KeySelector, keyPressed)
{
    typedef bool(*fun_type)();
    stub.set_lamda((fun_type)(&QPersistentModelIndex::isValid),[](){
        __DBG_STUB_INVOKE__
                return true;
    });
    bool callincrementSelect = false;
    bool callsingleSelect = false;
    bool callcontinuesSelect = false;

    stub.set_lamda((fun_type)&QModelIndex::isValid,[](){
        __DBG_STUB_INVOKE__
                return true;
    });
    stub.set_lamda(&KeySelector::incrementSelect,[&callincrementSelect](KeySelector*,const QModelIndex &){
        __DBG_STUB_INVOKE__
                callincrementSelect = true;
    });
    stub.set_lamda(&KeySelector::singleSelect,[&callsingleSelect](KeySelector*,const QModelIndex &){
        __DBG_STUB_INVOKE__
                callsingleSelect = true;
    });
    stub.set_lamda(&ClickSelector::continuesSelect,[&callcontinuesSelect](ClickSelector*,const QModelIndex &){
        __DBG_STUB_INVOKE__
                callcontinuesSelect = true;
    });
    Qt::KeyboardModifier filter =  Qt::NoModifier;
    stub.set_lamda(&KeySelector::moveCursor,[&filter](KeySelector*, QKeyEvent *event){
        __DBG_STUB_INVOKE__
                event->setModifiers(filter);
        return QPersistentModelIndex();

    });
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    filter = Qt::ControlModifier;
    key->keyPressed(&event);
    EXPECT_TRUE(callincrementSelect);

    filter = Qt::ShiftModifier;
    event.setModifiers(Qt::NoModifier);
    key->keyPressed(&event);
    EXPECT_TRUE(callcontinuesSelect);

    filter = Qt::NoModifier;
    event.setModifiers(Qt::NoModifier);
    key->keyPressed(&event);
    EXPECT_TRUE(callsingleSelect);
}

TEST_F(UT_KeySelector, filterKeys)
{
    view->setTabKeyNavigation(true);
    QList<Qt::Key> res = key->filterKeys();
    EXPECT_TRUE(res.contains(Qt::Key_Tab));
    EXPECT_TRUE(res.contains(Qt::Key_Backtab));
}

TEST_F(UT_KeySelector, keyboardSearch)
{
    typedef bool(*fun_type)();
    stub.set_lamda((fun_type)&QModelIndex::isValid,[](){
        __DBG_STUB_INVOKE__
                return true;
    });
    bool callsingleSelect = false;
    stub.set_lamda(&KeySelector::singleSelect,[&callsingleSelect](KeySelector*,const QModelIndex &){
         __DBG_STUB_INVOKE__
                 callsingleSelect = true;
    });
    stub.set_lamda(&CanvasViewPrivate::findIndex,
                   [](CanvasViewPrivate*, const QString &key, bool matchStart, const QModelIndex &current, bool reverseOrder, bool excludeCurrent){
        __DBG_STUB_INVOKE__
                return QModelIndex();
    });
    QString search = QString("temp_str");
    key->keyboardSearch(search);
    EXPECT_TRUE(key->searchTimer->isActive());
    EXPECT_TRUE(callsingleSelect);
}

TEST_F(UT_KeySelector, moveCursor)
{
    typedef QModelIndex(*fun_type)(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    stub.set_lamda((fun_type)(&CanvasView::moveCursor),[](QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers){
        __DBG_STUB_INVOKE__
                return QModelIndex();
    });
     view->setTabKeyNavigation(true);
     QKeyEvent event(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
     {
         event.k = Qt::Key_Down;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_Up;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_Left;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_Right;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_Home;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_End;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_PageUp;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_PageDown;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_Tab;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }
     {
         event.k = Qt::Key_Backtab;
         EXPECT_NO_FATAL_FAILURE(key->moveCursor(&event));
     }

}
TEST_F(UT_KeySelector, singleSelect)
{
    CanvasProxyModel * md = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *sm = new CanvasSelectionModel(md,nullptr);
    stub.set_lamda(&CanvasView::selectionModel, [sm]() -> CanvasSelectionModel* {
        __DBG_STUB_INVOKE__
        return sm;
    });
    const QModelIndex index ;
    view->d->state.view=view;
    key->singleSelect(index);
    EXPECT_EQ(view->d->operState().current(),index);
    EXPECT_EQ(view->d->operState().contBegin,index);
    delete md;
    delete sm;
}
TEST_F(UT_KeySelector, incrementSelect)
{
    typedef bool(*fun_type)();
    stub.set_lamda((fun_type)(&QModelIndex::isValid),[](){
        __DBG_STUB_INVOKE__
                return true;
    });
    CanvasProxyModel * md = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *sm = new CanvasSelectionModel(md,nullptr);
    stub.set_lamda(&CanvasView::selectionModel, [sm]() -> CanvasSelectionModel* {
        __DBG_STUB_INVOKE__
        return sm;
    });
    const QModelIndex index;
    view->d->state.view=view;
    key->incrementSelect(index);
    EXPECT_EQ(view->d->operState().current(),index);
    EXPECT_EQ(view->d->operState().contBegin,index);
    delete md;
    delete sm;
}
TEST_F(UT_KeySelector, toggleSelect)
{
    CanvasProxyModel * md = new CanvasProxyModel(nullptr);
    CanvasSelectionModel *sm = new CanvasSelectionModel(md,nullptr);
    QModelIndex index;
    sm->selectedCache.push_back(index);
    stub.set_lamda(&CanvasView::selectionModel, [sm]() -> CanvasSelectionModel* {
        __DBG_STUB_INVOKE__
        return sm;
    });
    stub.set_lamda(&CanvasView::model, [md]() -> CanvasProxyModel* {
        __DBG_STUB_INVOKE__
        return md;
    });
    typedef int(*fun_type)(const QModelIndex&);
    stub.set_lamda((fun_type)(&CanvasProxyModel::rowCount),[](const QModelIndex&){
        __DBG_STUB_INVOKE__
                return 1;
    });
    EXPECT_NO_FATAL_FAILURE(key->toggleSelect());
    delete sm;
    delete md;
}
