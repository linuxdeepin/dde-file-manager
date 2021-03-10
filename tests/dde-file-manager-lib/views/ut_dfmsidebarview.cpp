/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include <QApplication>
#include <QDropEvent>
#include <QTest>
#include <stub.h>
#include <models/computermodel.h>
#include "../stub-ext/stubext.h"
#define private public
#define protected public

#include "models/dfmsidebarmodel.h"
#include "views/dfmsidebaritemdelegate.h"
#include "views/dfmsidebarview.h"
#include "models/trashfileinfo.h"
#include "interfaces/private/dabstractfileinfo_p.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "interfaces/dfmsidebaritem.h"
#include "models/computermodel.h"
#include "interfaces/dfmstandardpaths.h"

DFM_USE_NAMESPACE
using namespace testing;
using namespace stub_ext;
namespace  {
    class TestDFMSideBarView : public Test
    {
    public:
        virtual void SetUp() override {
            testPath = QCoreApplication::applicationDirPath();
            m_view = new DFMSideBarView;
        }

        virtual void TearDown() override {
            delete m_view;
        }

        DFMSideBarView * m_view;
        QString testPath;
    };
}

//TEST_F(TestDFMSideBarView, get_previous_index)
//{
//    ASSERT_NE(m_view, nullptr);

//    QModelIndex result = m_view->getPreviousIndex();
//    EXPECT_EQ(result, m_view->m_previous);
//}

//TEST_F(TestDFMSideBarView, get_current_index)
//{
//    ASSERT_NE(m_view, nullptr);

//    QModelIndex result = m_view->getCurrentIndex();
//    EXPECT_EQ(result, m_view->m_current);
//}

//TEST_F(TestDFMSideBarView, set_current_changed)
//{
//    ASSERT_NE(m_view, nullptr);

//    QModelIndex newMode;
//    m_view->currentChanged(newMode);

//    EXPECT_EQ(newMode, m_view->m_previous);
//}

//TEST_F(TestDFMSideBarView, get_item_at)
//{
//    ASSERT_NE(m_view, nullptr);

//    QPoint pt(1,1);
//    DFMSideBarItem *result = m_view->itemAt(pt);
//    EXPECT_EQ(result, nullptr);
//}

//TEST_F(TestDFMSideBarView, get_index_at)
//{
//    ASSERT_NE(m_view, nullptr);

//    m_view->indexAt(QPoint(1,1));
//}

//TEST_F(TestDFMSideBarView, get_drop_data)
//{
//    ASSERT_NE(m_view, nullptr);

//    Stub stub;
//    static bool myCallOpen = false;
//    void (*ut_openNewTab)() = [](){myCallOpen = true;};
//    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

//    DFileManagerWindow window;
//    EXPECT_TRUE(myCallOpen);

//    const DFMSideBar *bar = window.getLeftSideBar();
//    DFMSideBarView *t_p = bar->m_sidebarView;

//    DUrlList srcUrls;
//    DUrl url1("test1");
//    DUrl url2("test2");
//    url1.setScheme(BURN_SCHEME);
//    url2.setScheme(BURN_SCHEME);
//    srcUrls << url1 << url2;
//    DUrl dstUrl("/home/");

//    // replace dfileservice::createfileinfo
//    const DAbstractFileInfoPointer (*st_createFileInfo)(const QObject, const DUrl &) =
//            [](const QObject, const DUrl &)->const DAbstractFileInfoPointer
//    {
//        return QExplicitlySharedDataPointer<DAbstractFileInfo>(new DFileInfo("/home"));
//    };

//    stub.set(&DFileService::createFileInfo, st_createFileInfo);

//    // replace dfileservice::pastfile
//    DUrlList (*st_pasteFile)(const QObject *, DFMGlobal::ClipboardAction, const DUrl &, const DUrlList &) =
//            [](const QObject *, DFMGlobal::ClipboardAction, const DUrl &, const DUrlList &)->DUrlList {
//        return DUrlList();
//    };
//    stub.set(&DFileService::pasteFile, st_pasteFile);

//    bool result = t_p->onDropData(srcUrls, dstUrl, Qt::IgnoreAction);
//    EXPECT_FALSE(result);

//    result = t_p->onDropData(srcUrls, dstUrl, Qt::CopyAction);
//    EXPECT_TRUE(result);

//    result = t_p->onDropData(srcUrls, dstUrl, Qt::LinkAction);
//    EXPECT_TRUE(result);

//    result = t_p->onDropData(srcUrls, dstUrl, Qt::MoveAction);
//    EXPECT_TRUE(result);
//}

//TEST_F(TestDFMSideBarView, drop_mime_data)
//{
//    ASSERT_NE(m_view, nullptr);

//    m_view->m_urlsForDragEvent.clear();
//    Qt::DropAction result = m_view->canDropMimeData(nullptr, nullptr, Qt::IgnoreAction);
//    EXPECT_EQ(result, Qt::IgnoreAction);

//    Stub stub;
//    static bool myCallOpen = false;
//    void (*ut_openNewTab)() = [](){myCallOpen = true;};
//    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

//    DFileManagerWindow window;
//    EXPECT_TRUE(myCallOpen);

//    const DFMSideBar *bar = window.getLeftSideBar();
//    DFMSideBarView *t_p = bar->m_sidebarView;
//    DFMSideBarItem *item = DFMSideBarItem::createSeparatorItem(QString("Trash"));
//    ASSERT_NE(item, nullptr);

//    DUrlList srcUrls;
//    srcUrls << DUrl("test1")<<DUrl("test2");
//    DUrl dstUrl("/home/test/");
//    TrashFileInfo *info = new TrashFileInfo(dstUrl);
//    Q_UNUSED(info)

//    QMimeData data;
//    t_p->m_urlsForDragEvent << QUrl("/home/test/aaa") << QUrl("/home/test/bbb");

//    const DAbstractFileInfoPointer (*st_createFileInfo)(const QObject, const DUrl &) =
//            [](const QObject, const DUrl &)->const DAbstractFileInfoPointer
//    {
//        return QExplicitlySharedDataPointer<DAbstractFileInfo>(new DFileInfo("/home"));
//    };

//    stub.set(&DFileService::createFileInfo, st_createFileInfo);

//    result = t_p->canDropMimeData(item, &data, Qt::IgnoreAction);
//    EXPECT_EQ(result, Qt::IgnoreAction);
//}

//TEST_F(TestDFMSideBarView, get_acceptte_drag)
//{
//    ASSERT_NE(m_view, nullptr);

//    QPointF pos(1,1);
//    Qt::DropActions actions = Qt::MoveAction;
//    QMimeData data;
//    Qt::MouseButtons buttons = Qt::LeftButton;
//    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
//    QDropEvent event(pos, actions, &data, buttons, modifiers);

//    Stub stub;
//    static bool myCallOpen = false;
//    void (*ut_openNewTab)() = [](){myCallOpen = true;};
//    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

//    DFileManagerWindow window;
//    EXPECT_TRUE(myCallOpen);

//    const DFMSideBar *bar = window.getLeftSideBar();
//    DFMSideBarView *t_p = bar->m_sidebarView;
//    ASSERT_NE(t_p, nullptr);

//    bool result = t_p->isAccepteDragEvent(&event);
//    EXPECT_FALSE(result);

//    Qt::DropAction (*st_canDropMimeData)(DFMSideBarItem *, const QMimeData *, Qt::DropActions) =
//            [](DFMSideBarItem *, const QMimeData *, Qt::DropActions)->Qt::DropAction {
//        return Qt::CopyAction;
//    };

//    stub.set(ADDR(DFMSideBarView, canDropMimeData), st_canDropMimeData);
//    result = t_p->isAccepteDragEvent(&event);
//    EXPECT_TRUE(result);
//}

//TEST_F(TestDFMSideBarView, get_row_changed)
//{
//    ASSERT_NE(m_view, nullptr);

//    Stub stub;
//    static bool myCallOpen = false;
//    void (*ut_openNewTab)() = [](){myCallOpen = true;};
//    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

//    DFileManagerWindow window;
//    EXPECT_TRUE(myCallOpen);

//    const DFMSideBar *bar = window.getLeftSideBar();
//    DFMSideBarView *t_p = bar->m_sidebarView;
//    ASSERT_NE(t_p, nullptr);

//    if(t_p->m_strItemUniqueKey.isEmpty()) {
//        t_p->m_strItemUniqueKey = QString("testKey");
//    }
//    t_p->onRowCountChanged();
//}

//TEST_F(TestDFMSideBarView, featch_drag_memory)
//{
//    ASSERT_NE(m_view, nullptr);

//    Stub stub;
//    static bool myCallOpen = false;
//    void (*ut_openNewTab)() = [](){myCallOpen = true;};
//    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

//    DFileManagerWindow window;
//    EXPECT_TRUE(myCallOpen);

//    const DFMSideBar *bar = window.getLeftSideBar();
//    DFMSideBarView *t_p = bar->m_sidebarView;
//    ASSERT_NE(t_p, nullptr);

//    EXPECT_NO_FATAL_FAILURE(t_p->fetchDragEventUrlsFromSharedMemory());

//    bool (*st_isAttached)() = []()->bool {
//        return true;
//    };

//    stub.set(ADDR(QSharedMemory, isAttached), st_isAttached);
//    EXPECT_TRUE(t_p->fetchDragEventUrlsFromSharedMemory());
//}

//TEST_F(TestDFMSideBarView, check_op_time)
//{
//    ASSERT_NE(m_view, nullptr);

//    Stub stub;
//    static bool myCallOpen = false;
//    void (*ut_openNewTab)() = [](){myCallOpen = true;};
//    stub.set(ADDR(DFileManagerWindow, openNewTab), ut_openNewTab);

//    DFileManagerWindow window;
//    EXPECT_TRUE(myCallOpen);

//    const DFMSideBar *bar = window.getLeftSideBar();
//    DFMSideBarView *t_p = bar->m_sidebarView;
//    ASSERT_NE(t_p, nullptr);

//    QThread::msleep(300);
//    bool result = t_p->checkOpTime();
//    EXPECT_TRUE(result);
//    result = t_p->checkOpTime();
//    EXPECT_FALSE(result);
//}

//TEST_F(TestDFMSideBarView, tst_mousePressEvent)
//{
//    QSharedPointer<QMouseEvent> event = dMakeEventPointer<QMouseEvent>(
//                QMouseEvent::KeyPress, QPointF(0,0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    m_view->mousePressEvent(event.get());

//    QSharedPointer<QMouseEvent> event2 = dMakeEventPointer<QMouseEvent>(
//                QMouseEvent::KeyPress, QPointF(0,0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
//    EXPECT_NO_FATAL_FAILURE(m_view->mousePressEvent(event2.get()));
//}

//TEST_F(TestDFMSideBarView, tst_mouseMoveEvent)
//{
//    QSharedPointer<QMouseEvent> event = dMakeEventPointer<QMouseEvent>(
//                QMouseEvent::MouseMove, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
//    m_view->mouseMoveEvent(event.get());

//    QSharedPointer<QMouseEvent> event2 = dMakeEventPointer<QMouseEvent>(
//                QMouseEvent::MouseMove, QPointF(0,0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    m_view->setState(DFMSideBarView::DraggingState);
//    EXPECT_NO_FATAL_FAILURE(m_view->mouseMoveEvent(event2.get()));
//}

//TEST_F(TestDFMSideBarView, tst_dragEnterEvent)
//{
//    QSharedPointer<QDragEnterEvent> event = dMakeEventPointer<QDragEnterEvent>(
//                QPoint(), Qt::CopyAction, nullptr, Qt::NoButton, Qt::NoModifier);

//    ComputerModel computerModel;
//    m_view->setModel((QAbstractItemModel*)&computerModel);

//    bool (*st_isAccepteDragEvent_true)(DFMDragEvent *event) = [](DFMDragEvent *event)->bool {
//        Q_UNUSED(event)
//        return true;
//    };

//    bool (*st_isAccepteDragEvent_false)(DFMDragEvent *event) = [](DFMDragEvent *event)->bool {
//        Q_UNUSED(event)
//        return false;
//    };

//    Stub stub;
//    stub.set(&DFMSideBarView::isAccepteDragEvent, st_isAccepteDragEvent_true);
//    m_view->dragEnterEvent(event.get());

//    stub.set(&DFMSideBarView::isAccepteDragEvent, st_isAccepteDragEvent_false);
//    EXPECT_NO_FATAL_FAILURE(m_view->dragEnterEvent(event.get()));
//}

//TEST_F(TestDFMSideBarView, tst_dragMoveEvent)
//{
//    QSharedPointer<QDragEnterEvent> event = dMakeEventPointer<QDragEnterEvent>(
//                QPoint(), Qt::CopyAction, nullptr, Qt::NoButton, Qt::NoModifier);
//    EXPECT_NO_FATAL_FAILURE(m_view->dragMoveEvent(event.get()));
//}

//TEST_F(TestDFMSideBarView, tst_dropEvent)
//{
//    QSharedPointer<QDropEvent> event = dMakeEventPointer<QDropEvent>(
//                QPointF(), Qt::CopyAction, nullptr, Qt::NoButton, Qt::NoModifier);
//    m_view->dropEvent(event.get());

//    DFMSideBarItem *(*st_itemAt)(const QPoint &pt) = [](const QPoint &pt)->DFMSideBarItem* {
//        return new DFMSideBarItem();
//    };

//    // replace QDropEvent::mimeData
//    Stub stub;
//    stub.set(&DFMSideBarView::itemAt, st_itemAt);

//    const QMimeData *(*st_mimeData)() = []()->const QMimeData *{
//        return new QMimeData();
//    };
//    stub.set(&QDropEvent::mimeData, st_mimeData);
//    m_view->dropEvent(event.get());

//    // replace DFMSideBarView::visualRect
//    QRect (*st_visualRect)(const QModelIndex &index) = [](const QModelIndex &index)->QRect{
//        Q_UNUSED(index)
//        return QRect(-1, -1, 10, 10);
//    };
//    stub_ext::StubExt st;
//    st.set(VADDR(DFMSideBarView, visualRect), st_visualRect);

//    // replace QMimeData::urls
//    QList<QUrl> (*st_urls)() = []()->QList<QUrl>{
//        QList<QUrl> urls;
//        urls << QUrl("/home");
//        urls << QUrl(DFMVAULT_ROOT);
//        urls << DFMStandardPaths::location(DFMStandardPaths::MusicPath);
//        return urls;
//    };
//    stub.set(ADDR(QMimeData, urls), st_urls);

//    // replace DFMSideBarView::onDropData
//    bool (*st_onDropData)(DUrlList, DUrl, Qt::DropAction) = [](DUrlList, DUrl, Qt::DropAction)->bool{
//        // do nothing.
//        return true;
//    };
//    stub.set(ADDR(DFMSideBarView, onDropData), st_onDropData);

//    EXPECT_NO_FATAL_FAILURE(m_view->dropEvent(event.get()));
//}
