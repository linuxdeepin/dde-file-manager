#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define private public
#define protected public
#include "screen/screenhelper.h"
#include <view/canvasviewmanager.h>
#include <view/backgroundmanager.h>
#include <view/canvasgridview.h>
#include <presenter/gridmanager.h>
#include <dfilesystemmodel.h>
#include "../model/dfileselectionmodel.h"
#include <private/canvasviewprivate.h>
#include "util/xcb/xcb.h"
#include <QWidget>
#include <QTest>
#include <QEventLoop>
#include "../../view/desktopitemdelegate.h"
#include <QThread>
using namespace std;


class QAbstractItemModel;
class CanvasGridViewTest : public testing::Test
{
public:
    CanvasGridViewTest():Test(){
        for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
            if(1 == tpCanvas->screenNum()){
                m_canvasGridView = tpCanvas.data();
                break;
            }
        }
    }
    virtual void SetUp() override{
    }
    virtual void TearDown()override{
    }
public:
    QScopedPointer<CanvasViewManager> m_cvmgr{new CanvasViewManager(new BackgroundManager())};
    CanvasGridView *m_canvasGridView{nullptr};
};


TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_visualRect){

    ASSERT_NE(m_cvmgr, nullptr);
    auto tempUrl = GridManager::instance()->firstItemId(1);
    QPair<int, QPoint> tempPos;
    GridManager::instance()->find(tempUrl,tempPos);

    auto x = tempPos.second.x() * m_canvasGridView->d->cellWidth + m_canvasGridView->d->viewMargins.left();
    auto y = tempPos.second.y() * m_canvasGridView->d->cellHeight + m_canvasGridView->d->viewMargins.top();
    QRect tempRec(x, y, m_canvasGridView->d->cellWidth,m_canvasGridView->d->cellHeight);

    DUrl tpUrl(tempUrl);
    auto tempIndex = m_canvasGridView->model()->index((tpUrl));
    if(tempIndex.isValid()){
        auto tgRect = m_canvasGridView->visualRect(tempIndex);
        bool tempTg = tempRec == tgRect;
        EXPECT_TRUE(tempTg) << tempRec.x() << "," << tempRec.y()<<","<<tempRec.width()<< "," <<tempRec.height()<< " <==> "
                                       << tgRect.x() << "," << tgRect.y()<<","<<tgRect.width()<< "," <<tgRect.height();
    }

//    ASSERT_TRUE(tempRec == tgRect);
}

TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_indexAt){
    ASSERT_NE(m_canvasGridView, nullptr);
    auto utModel = m_canvasGridView->model();
    ASSERT_NE(utModel, nullptr);
    //先拿到主屏图标Url
    auto itemIds = GridManager::instance()->itemIds(1);
    for(auto oneUrl : itemIds){
        //拿到对应index
        DUrl tpUrl(oneUrl);
        auto tempIndex = utModel->index((tpUrl));
        if(tempIndex.isValid()){
            //拿到对应Rect
            auto tgRect = m_canvasGridView->visualRect(tempIndex);
            QPoint tpPos(tgRect.x() + 5,tgRect.y() + 5);
            auto tgIndex = m_canvasGridView->indexAt(tpPos);
            if(tgIndex.isValid()){
                auto tempTg = tempIndex == tgIndex;
                EXPECT_TRUE(tempTg) << tempIndex.isValid() << " === " <<tpUrl.toString().toStdString();
            }
        }
    }
}

TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_currentUrl){
    ASSERT_NE(m_canvasGridView, nullptr);
    auto tgUrl = m_canvasGridView->currentUrl();
    bool tgTemp = m_canvasGridView->model()->rootUrl() == tgUrl;
    EXPECT_TRUE(tgTemp);

}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setRootUrl){
    //file:///home/lee/Desktop
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    if(m_canvasGridView->setRootUrl(desktopUrl)){
        EXPECT_TRUE(desktopUrl == m_canvasGridView->model()->rootUrl());
    }
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setCurrentUrl){
    //file:///home/lee/Desktop
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    if(m_canvasGridView->setCurrentUrl(desktopUrl)){
        EXPECT_TRUE(desktopUrl == m_canvasGridView->model()->rootUrl());
    }
}


TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_canvansScreenName){
    ASSERT_NE(m_canvasGridView, nullptr);
    auto tempTg = m_canvasGridView->m_screenName == m_canvasGridView->canvansScreenName();
    EXPECT_TRUE(tempTg);
}

TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_cellSize){
    ASSERT_NE(m_canvasGridView, nullptr);
    QSize tempSize(m_canvasGridView->d->cellWidth,m_canvasGridView->d->cellHeight);
    auto tempTg = tempSize == m_canvasGridView->cellSize();
    EXPECT_TRUE(tempTg);
}

TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_cellMargins){
    ASSERT_NE(m_canvasGridView, nullptr);
    ASSERT_NE(m_canvasGridView->d, nullptr);
    auto tempTg = m_canvasGridView->d->cellMargins == m_canvasGridView->cellMargins();
    EXPECT_TRUE(tempTg);

}

TEST_F(CanvasGridViewTest,TEST_CanvasGridViewTest_Wid){
    ASSERT_NE(m_canvasGridView, nullptr);
    bool temp = false;
    if (m_canvasGridView->isTopLevel()) {
        temp = dynamic_cast<QAbstractItemView*>(m_canvasGridView->parent())->winId();
    } else {
        temp = m_canvasGridView->topLevelWidget()->winId() == m_canvasGridView->winId();
    }
    EXPECT_TRUE(temp);
}

TEST_F(CanvasGridViewTest,Test_CanvasGridViewTest_initRootUrl){
    ASSERT_NE(m_canvasGridView, nullptr);
    GridManager::instance()->setAutoMerge(true);
    m_canvasGridView->initRootUrl();
    auto temp = m_canvasGridView->model()->rootUrl().scheme() == DFMMD_SCHEME;
    EXPECT_TRUE(temp);

    GridManager::instance()->setAutoMerge(false);
    m_canvasGridView->initRootUrl();
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);
    auto temp2 = m_canvasGridView->model()->rootUrl() == desktopUrl;
    EXPECT_TRUE(temp2);
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_setScreenNum){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setScreenNum(4);
    EXPECT_EQ(4,m_canvasGridView->m_screenNum);
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_isSelect){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setCurrentIndex(QModelIndex());
    m_canvasGridView->clearSelection();
    auto tempItem = GridManager::instance()->firstItemId(1);
    DUrl tempUrl(tempItem);
    auto index = m_canvasGridView->model()->index(tempUrl);
    if(index.isValid()){
        m_canvasGridView->selectionModel()->select(QItemSelection (index, index), QItemSelectionModel::Select);
        EXPECT_TRUE(m_canvasGridView->isSelected(index));
    }
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_setGeometry){
    ASSERT_NE(m_canvasGridView, nullptr);
    auto oldGeometry = m_canvasGridView->geometry();
    m_canvasGridView->setGeometry(QRect(0,0,0,0));
    auto temp0 = oldGeometry == m_canvasGridView->geometry();
    EXPECT_TRUE(temp0);

    m_canvasGridView->setGeometry(QRect(500,500,500,500));
    auto temp1 = m_canvasGridView->geometry();
    EXPECT_TRUE(temp1 == QRect(500,500,500,500));
}


#if 0
//select相关无法设置成功
//todo
TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_select){
    ASSERT_NE(m_canvasGridView, nullptr);

    m_canvasGridView->setCurrentIndex(QModelIndex());
    m_canvasGridView->clearSelection();

    auto tempItemIds = GridManager::instance()->itemIds(1);
    QList<DUrl> list = DUrl::fromStringList(tempItemIds);
    m_canvasGridView->select(list);
    m_canvasGridView->show();

//    m_cvmgr->onSyncSelection(m_canvasGridView, list);
    auto temppp = m_canvasGridView->selectedUrls().size();
    EXPECT_EQ(temppp, list.size());
    auto tempIdxs = m_canvasGridView->selectionModel()->selectedIndexes();
    for(auto tpIdx : tempIdxs){
        auto info = m_canvasGridView->model()->fileInfo(tpIdx);
        if (info) {
            auto one = info->fileUrl().toString();
            if(tempItemIds.contains(one))
                tempItemIds.removeOne(one);
        }
    }
    EXPECT_TRUE(0 == tempItemIds.size());
}

//select相关无法设置成功
TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedUrls){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setCurrentIndex(QModelIndex());
    m_canvasGridView->clearSelection();
    auto temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(0 == temp);

    auto tempItemIds = GridManager::instance()->itemIds(1);
    QList<DUrl> list = DUrl::fromStringList(tempItemIds);
    m_canvasGridView->select(list);

    auto tempp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(list.size() == tempp);
}

//select相关无法设置成功
TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedIndexCount){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setCurrentIndex(QModelIndex());
    m_canvasGridView->clearSelection();

    auto tempItem = GridManager::instance()->firstItemId(1);
    DUrl tempTp(tempItem);
    auto index = m_canvasGridView->model()->index(tempTp);
    m_canvasGridView->selectionModel()->select(QItemSelection (index, index), QItemSelectionModel::Select);

    EXPECT_TRUE(1 == m_canvasGridView->selectedIndexCount());

    auto tempItemIds = GridManager::instance()->itemIds(1);
    QModelIndexList tpIndexs;
    QList<DUrl> list;
    for (auto temp : tempItemIds) {
        DUrl tempUrl(temp);
        list.append(tempUrl);
        auto index = m_canvasGridView->model()->index(tempUrl);
        if(index.isValid())
            tpIndexs.append(index);
    }
    m_canvasGridView->select(list);
    EXPECT_EQ(list.size(), m_canvasGridView->selectedIndexCount());
}
#endif

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setAutoMerge){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setAutoMerge(true);
    auto temp = m_canvasGridView->model()->rootUrl().scheme() == DFMMD_SCHEME;
    EXPECT_TRUE(temp);

    m_canvasGridView->setAutoMerge(false);
    auto temp2 = m_canvasGridView->model()->rootUrl().scheme() == FILE_SCHEME;
    EXPECT_TRUE(temp2);
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_mouseMoveEvent){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->d->_debug_show_grid = true;
    auto current1 = m_canvasGridView->mapToGlobal(m_canvasGridView->cursor().pos());
//    QTest::mouseMove(m_canvasGridView,QPoint(0,0));
    m_canvasGridView->cursor().setPos(QPoint(0,0));
//    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    QMouseEvent me(QEvent::MouseMove, QPoint(0, 0), m_canvasGridView->mapToGlobal(QPoint(0, 0)), Qt::NoButton, Qt::NoButton, Qt::KeyboardModifiers());
    m_canvasGridView->mouseMoveEvent(&me);
    auto current2 = m_canvasGridView->mapToGlobal(m_canvasGridView->cursor().pos());
    EXPECT_FALSE(current1 == current2);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_delayModelRefresh){
    ASSERT_NE(m_canvasGridView, nullptr);
    auto temp = m_canvasGridView->model()->rowCount();
    if(temp){
        m_canvasGridView->delayModelRefresh(0);
        auto temp2 = m_canvasGridView->model()->rowCount();
        EXPECT_FALSE(0 != temp2);
    }
}

#if 0
//select相关无法设置成功
TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setCurrentIndex(QModelIndex());
    m_canvasGridView->clearSelection();
    auto temp0 = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(0 == temp0);

    QTest::keyClick(m_canvasGridView, Qt::Key_H, Qt::ControlModifier);
    QTest::keyClick(m_canvasGridView, Qt::Key_A, Qt::ControlModifier);
    auto temp1 = m_canvasGridView->selectedUrls().size();
    auto temp2 = GridManager::instance()->itemIds(1).size();
    EXPECT_TRUE(temp1 == temp2);

    //    QTest::keyClick(m_canvasGridView, Qt::Key_F5, Qt::KeypadModifier);
    //    QTest::keyClick(m_canvasGridView, Qt::Key_M, Qt::AltModifier);
    //    QTest::keyClick(m_canvasGridView, Qt::Key_Escape, Qt::NoModifier);

}
#endif

//TEST_F(CanvasGridViewTest, CanvasGridViewTest_paintEvent){
//    m_canvasGridView->show();
//    QTimer::singleShot(1000,[=](){
//        m_canvasGridView->hide();
//    });
//}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_mousePressEvent){
    ASSERT_NE(m_canvasGridView, nullptr);

    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    QMouseEvent(QEvent::MouseButtonPress, QPoint(30, 30), m_canvasGridView->mapToGlobal(QPoint(30, 30)), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    m_canvasGridView->mousePressEvent(&me);

    EXPECT_TRUE(m_canvasGridView->d->mousePressed);
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_mouseReleaseEvent){
    ASSERT_NE(m_canvasGridView, nullptr);
    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    QMouseEvent(QEvent::MouseButtonRelease, QPoint(30, 30), m_canvasGridView->mapToGlobal(QPoint(30, 30)), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    m_canvasGridView->mouseReleaseEvent(&me);
}

//与select相关会有问题
//TEST_F(CanvasGridViewTest, CanvasGridViewTest_setSelection){
//    ASSERT_NE(m_canvasGridView, nullptr);
//    m_canvasGridView->setCurrentIndex(QModelIndex());
//    m_canvasGridView->clearSelection();

//    m_canvasGridView->d->showSelectRect = true;
//    m_canvasGridView->cursor().setPos(QPoint(0,0));
////    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
//    QMouseEvent me(QEvent::MouseMove, QPoint(0, 0), m_canvasGridView->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
//    m_canvasGridView->mouseMoveEvent(&me);
//    auto temp = m_canvasGridView->selectedUrls().size();
//    EXPECT_TRUE(0 != temp);
//}

//菜单show出来会被阻塞，所以此用例又不行了
//TEST_F(CanvasGridViewTest, CanvasGridViewTest_contextMenuEvent){
////    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(500, 600),m_canvasGridView->mapToGlobal(QPoint(500, 500)),Qt::KeyboardModifiers());

//    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(10, 10),m_canvasGridView->mapToGlobal(QPoint(10, 10)),Qt::KeyboardModifiers());
//    m_canvasGridView->contextMenuEvent(&ent);
//}

//TEST_F(CanvasGridViewTest, CanvasGridViewTest_showNormalMenu){
//todo
//}

//TEST_F(CanvasGridViewTest, CanvasGridViewTest_showEmptyAreaMenu){
//todo
//}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setIconByLevel)
{
    m_canvasGridView->setIconByLevel(3);
    QTimer timer;
    timer.start(500);
    QEventLoop loop;
    QObject::connect(m_canvasGridView, &CanvasGridView::changeIconLevel, [&]{
        loop.exit();
    });
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
        loop.exit();
    });
    loop.exec();
    DesktopItemDelegate* p =m_canvasGridView->itemDelegate();
    EXPECT_EQ(3, p->iconSizeLevel());
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_handleContextMenuAction)
{
    m_canvasGridView->m_screenName = "1";
    m_canvasGridView->handleContextMenuAction(MenuAction::SelectAll);
    m_canvasGridView->handleContextMenuAction(CanvasGridView::AutoMerge);
    QTimer timer;
    timer.start(500);
    QEventLoop loop;
    QObject::connect(m_canvasGridView, &CanvasGridView::autoMergeToggled, [&]{
        EXPECT_TRUE(true);
        loop.exit();
    });
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
        loop.exit();
    });
    loop.exec();
    m_canvasGridView->handleContextMenuAction(MenuAction::LastModifiedDate);
    EXPECT_TRUE(m_canvasGridView->model()->enabledSort());
    EXPECT_EQ(DFileSystemModel::FileLastModifiedRole, m_canvasGridView->model()->sortRole());
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent)
{
     QKeyEvent test(QEvent::KeyPress, Qt::Key_Return,  Qt::KeyboardModifier::KeypadModifier);
     m_canvasGridView->keyPressEvent(&test);
     QString name = qApp->applicationName();
     QKeyEvent test1(QEvent::KeyPress, Qt::Key_F1,  Qt::NoModifier);
     EXPECT_EQ(name, qApp->applicationName());
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setSelection)
{
    EXPECT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->d->mousePressed = false;
    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->setSelection(QRect(QPoint(2, 3),QPoint(6, 8)), QItemSelectionModel::Select);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_delayAutoMerge_refresh)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setAutoMerge(true);
    EXPECT_TRUE(GridManager().instance()->autoMerge());

    m_canvasGridView->delayAutoMerge(0.5);
    m_canvasGridView->delayAutoMerge(0.5);
    EXPECT_EQ(nullptr, m_canvasGridView->m_refreshTimer);

    m_canvasGridView->m_refreshTimer = new QTimer();
    m_canvasGridView->delayModelRefresh(0.1);
    m_canvasGridView->delayModelRefresh(0.1);
    EXPECT_EQ(nullptr, m_canvasGridView->m_refreshTimer);
//    QTimer timer;
//    timer.start(500);
//    QEventLoop loop;
//    QObject::connect(GridManager().instance(), &GridManager::sigSyncOperation, [&]{
//        timer.stop();
//        loop.exit();
//    });
//    QObject::connect(&timer, &QTimer::timeout, [&]{
//        EXPECT_TRUE(false);
//        timer.stop();
//        loop.exit();
//    });
//    loop.exec();
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_moveCursorGrid)
{
    auto selectionModel = m_canvasGridView->selectionModel();
    auto headIndex = m_canvasGridView->firstIndex();
    auto tailIndex = m_canvasGridView->lastIndex();
    QModelIndex *temp = new QModelIndex();
    temp->r = 2;
    temp->c = 2;
    temp->m = new DFileSystemModel(nullptr);
    m_canvasGridView->d->currentCursorIndex = *temp;
    QModelIndex current = m_canvasGridView->d->currentCursorIndex;
    QString mycurrent = current.data().toString();
    QList<DUrl> lists;
    lists << DUrl(mycurrent);
    m_canvasGridView->select(lists);
//    if (!current.isValid() || !selectionModel->isSelected(current)) {
//        EXPECT_TRUE(false);
//    }
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_dragMoveEvent)
{
    QDragMoveEvent devent(QPoint(2,1), Qt::DropAction::MoveAction, new QMimeData(), Qt::MouseButton::BackButton, Qt::KeyboardModifier::AltModifier);
    m_canvasGridView->dragMoveEvent(&devent);
    QDropEvent dropevent(QPoint(4,2), Qt::DropAction::IgnoreAction, new QMimeData(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_canvasGridView->dropEvent(&dropevent);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_toggleEntryExpandedState)
{
    DUrl url = m_canvasGridView->currentUrl();
    url.setScheme(DFMMD_SCHEME);
    if (!url.isValid() || url.scheme() != DFMMD_SCHEME) {
        EXPECT_TRUE(false);
    }
    m_canvasGridView->toggleEntryExpandedState(url);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_moveCursor)
{
    m_canvasGridView->d->currentCursorIndex = QModelIndex();
    m_canvasGridView->moveCursor(QAbstractItemView::MoveDown, Qt::KeyboardModifier::MetaModifier);
    EXPECT_TRUE(m_canvasGridView->firstIndex() == m_canvasGridView->d->currentCursorIndex);
}

//TEST_F(CanvasGridViewTest, CanvasGridViewTest_contextMenuEvent)
//{
//    QContextMenuEvent textevent(QContextMenuEvent::Mouse, QPoint(2,2));
//    m_canvasGridView->contextMenuEvent(&textevent);
//    QEventLoop loop;
//    QTimer::singleShot(100,&loop,[&](){
//        m_canvasGridView->hide();
//        loop.exit();
//    });
//    loop.exec();
//    EXPECT_TRUE(0 == m_canvasGridView->selectedIndexCount());
//}

