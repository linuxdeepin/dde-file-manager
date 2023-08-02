// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/operator/shortcutoper.h"
#include "view/canvasview_p.h"
#include "view/canvasview.h"
#include "model/canvasproxymodel_p.h"
#include "canvasmanager.h"
#include "utils/fileutil.h"
#include "view/operator/fileoperatorproxy.h"
#include "delegate/canvasitemdelegate.h"
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-framework/event/event.h>

#include "stubext.h"
#include <gtest/gtest.h>

#include <QApplication>
#include <QPaintEvent>
#include <QClipboard>

using namespace testing;
using namespace ddplugin_canvas;
using namespace dfmbase;
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_ShortcutOper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        canvas = new CanvasView;
        sc = new ShortcutOper(canvas);
    }
    virtual void TearDown() override
    {
        delete sc;
        delete canvas;
        stub.clear();

    }
    CanvasView *canvas = nullptr;
    ShortcutOper *sc = nullptr;
    stub_ext::StubExt stub ;
};

class TestFileInfo : public FileInfo
{
public:
    using FileInfo::FileInfo;
    void refresh() override {re = true;}
    bool isAttributes(const FileIsType type) const {
        if (type == OptInfoType::kIsHidden)
            return hidden;
        return FileInfo::isAttributes(type);
    }
public:
    bool re = false;
    bool hidden = true;
};

TEST_F(UT_ShortcutOper, keyPressed)
{
    CanvasManager  manager;
    CanvasProxyModel proxymodel ;
    CanvasSelectionModel model (&proxymodel,nullptr);
    canvas->setSelectionModel(&model);
    KeySelector keyselector(canvas);
    canvas->d->keySelector = &keyselector;
    typedef void(*fun_type)();
    stub.set_lamda((fun_type)(&CanvasView::selectAll),[](){
        __DBG_STUB_INVOKE__
    });

    bool disableShortcut = true;
    stub.set_lamda(&ShortcutOper::disableShortcut,[&disableShortcut](){
        __DBG_STUB_INVOKE__
        return disableShortcut;
    });

    stub.set_lamda(&ShortcutOper::previewFiles,[](){
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&CanvasView::refresh,[](CanvasView *self,bool){
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&ShortcutOper::swichHidden,[](){
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&ShortcutOper::showMenu,[](){
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&KeySelector::toggleSelect,[](){
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&ShortcutOper::tabToFirst,[](){
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&ShortcutOper::clearClipBoard,[](){
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&CanvasManager::onChangeIconLevel,[](CanvasManager*,bool increase){
        __DBG_STUB_INVOKE__
    });

    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier,"", true);
        EXPECT_TRUE(sc->keyPressed(&key));
    }
    {
        QKeyEvent key (QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier,"", true);
        {
            key.k = Qt::Key_F1;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_Tab;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_Escape;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
    }
    disableShortcut = false;
    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier,"", true);
        {
            key.k = Qt::Key_F1;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_Tab;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_Escape;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
    }
    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Tab, Qt::KeypadModifier,"", true);
        {
            stub.set_lamda(&CanvasSelectionModel::selectedUrls,[](){
                __DBG_STUB_INVOKE__
                        return QList<QUrl>{QUrl("temp")};
            });
            key.k = Qt::Key_Enter;
            key.autor = 0;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_Space;
            key.autor = 0;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_F5;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_Delete;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
    }
    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Tab, Qt::ShiftModifier,"", true);
        {
            key.k  = Qt::Key_Delete;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k  = Qt::Key_T;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
    }
    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Tab, Qt::ControlModifier,"", true);
        {
            key.k = Qt::Key_Minus;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_Equal;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_H;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k = Qt::Key_I;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k  = Qt::Key_A;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k  = Qt::Key_C;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k  = Qt::Key_X;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k  = Qt::Key_V;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
        {
            key.k  = Qt::Key_Z;
            EXPECT_TRUE(sc->keyPressed(&key));
        }
    }
    {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Tab, Qt::AltModifier,"", true);
        key.k = Qt::Key_M;
        EXPECT_TRUE(sc->keyPressed(&key));
    }
    {
        QKeyEvent key (QEvent::KeyPress, Qt::Key_Tab, (Qt::ControlModifier | Qt::ShiftModifier),"", true);
        key.k = Qt::Key_I;
        EXPECT_TRUE(sc->keyPressed(&key));
    }
}

TEST_F(UT_ShortcutOper, showMenu)
{
    typedef bool(*fun_type)();
    stub.set_lamda((fun_type)&CanvasViewMenuProxy::disableMenu,[](){
                 __DBG_STUB_INVOKE__
                 return false;
    });
    QModelIndex index ;
    stub.set_lamda(&CanvasSelectionModel::selectedIndexesCache,[&index](){
        __DBG_STUB_INVOKE__
                QModelIndexList res;
                res.push_back(index);
                return res;
    });
    CanvasProxyModel canvasproxymodel;
    CanvasSelectionModel canvassmodel(&canvasproxymodel,nullptr);
    stub.set_lamda(&CanvasView::selectionModel,[&canvassmodel](){
        __DBG_STUB_INVOKE__
                return &canvassmodel;
    });
    CanvasView view ;
    CanvasItemDelegate  resDelegate(&view);
    stub.set_lamda(&CanvasView::itemDelegate,[&resDelegate](){
        __DBG_STUB_INVOKE__
                return &resDelegate;
    });
    QModelIndex rootIndex ;
    CanvasProxyModel proxymodel ;
    CanvasSelectionModel model(&proxymodel,nullptr);
    QStyledItemDelegate delegate ;
    canvas->setSelectionModel(&model);
    canvas->setModel(&proxymodel);
    canvas->setItemDelegate(&delegate);
    QModelIndex curIndex ;
    canvas->setCurrentIndex(curIndex);
    canvas->setRootIndex(rootIndex);
    sc->showMenu();
    EXPECT_FALSE(canvas->isPersistentEditorOpen(curIndex));
}

TEST_F(UT_ShortcutOper, clearClipBoard)
{
    stub.set_lamda(&CanvasProxyModel::rootUrl,[](){
        __DBG_STUB_INVOKE__
                return QUrl("file:/home/user/file.txt");
    });
    stub.set_lamda(&ClipBoard::clipboardFileUrlList,[](){
        __DBG_STUB_INVOKE__
                return QList<QUrl>{QUrl("temp")};
    });

    typedef QString(*fun_type)(const FileInfo::FilePathInfoType);
    stub.set_lamda((fun_type)(&FileInfo::pathOf),[](){
        __DBG_STUB_INVOKE__
                return QString("/home/user/file.txt");
    });

    stub.set_lamda(&DesktopFileCreator::createFileInfo,[](DesktopFileCreator *slef,const QUrl &url,
                   dfmbase::Global::CreateFileInfoType cache){
        __DBG_STUB_INVOKE__
                  FileInfoPointer file(new TestFileInfo(QUrl("file:/home/user/file.txt")));
                 return file;
    } );
    CanvasProxyModel proxymodel ;
    canvas->setModel(&proxymodel);
    qApp->clipboard()->setText("temp_text");
    sc->clearClipBoard();
    EXPECT_EQ(qApp->clipboard()->text(),"");
}

TEST_F(UT_ShortcutOper, swichHidden)
{
    stub.set_lamda(&CanvasProxyModel::refresh,
                   [](CanvasProxyModel *self,const QModelIndex &, bool, int, bool){
        __DBG_STUB_INVOKE__
    });
    CanvasProxyModel proxymodel;
    canvas->setModel(&proxymodel);
    sc->swichHidden();
    EXPECT_TRUE(proxymodel.d->filters &QDir::Hidden);
    sc->swichHidden();
    EXPECT_FALSE(proxymodel.d->filters &QDir::Hidden);
}

TEST_F(UT_ShortcutOper, previewFiles)
{
    bool call = false;
    stub.set_lamda(&CanvasProxyModel::files,[&call](){
        __DBG_STUB_INVOKE__
                call = true;
        return QList<QUrl>{QUrl("temp")};
    });
    stub.set_lamda(&CanvasSelectionModel::selectedUrls,[](){
        __DBG_STUB_INVOKE__
                return QList<QUrl>{QUrl("temp")};
    });
    CanvasProxyModel proxymodel;
    CanvasSelectionModel model(&proxymodel,nullptr);
    canvas->setSelectionModel(&model);
    canvas->setModel(&proxymodel);
    sc->previewFiles();
    EXPECT_TRUE(call);
}
