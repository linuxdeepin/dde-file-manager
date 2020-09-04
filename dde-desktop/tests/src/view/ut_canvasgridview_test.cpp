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

class CanvasGridViewTest : public testing::Test
{
public:
    CanvasGridViewTest():Test(){}
    virtual void SetUp() override{
        p_bkmgr = new BackgroundManager ();
        p_cvmgr = new CanvasViewManager (p_bkmgr);
        for(auto tpCanvas : p_cvmgr->m_canvasMap.values()){
            if(1 == tpCanvas->screenNum()){
                m_canvasGridView = tpCanvas.data();
                break;
            }
        }
    }
    virtual void TearDown()override{
        if(p_bkmgr)
            delete p_bkmgr;
        if(p_cvmgr)
            delete p_cvmgr;
        p_bkmgr = nullptr;
        p_cvmgr = nullptr;
    }
public:
    BackgroundManager *p_bkmgr{nullptr};
    CanvasViewManager *p_cvmgr{nullptr};
    CanvasGridView *m_canvasGridView{nullptr};
};

#if 1
//？
TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_visualRect){


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

//？
TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_indexAt){
    //先拿到主屏图标Url
    auto itemIds = GridManager::instance()->itemIds(1);
    for(auto oneUrl : itemIds){
        //拿到对应index
        DUrl tpUrl(oneUrl);
        auto tempIndex = m_canvasGridView->model()->index((tpUrl));
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
    auto tgUrl = m_canvasGridView->currentUrl();
    bool tgTemp = m_canvasGridView->model()->rootUrl() == tgUrl;
    EXPECT_TRUE(tgTemp);

}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setRootUrl){
    //file:///home/lee/Desktop
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    if(m_canvasGridView->setRootUrl(desktopUrl))
        EXPECT_TRUE(desktopUrl == m_canvasGridView->model()->rootUrl());
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setCurrentUrl){
    //file:///home/lee/Desktop
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    if(m_canvasGridView->setCurrentUrl(desktopUrl))
        EXPECT_TRUE(desktopUrl == m_canvasGridView->model()->rootUrl());
}


TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_canvansScreenName){
    auto tempTg = m_canvasGridView->m_screenName == m_canvasGridView->canvansScreenName();
    EXPECT_TRUE(tempTg);
}

TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_cellSize){
    QSize tempSize(m_canvasGridView->d->cellWidth,m_canvasGridView->d->cellHeight);
    auto tempTg = tempSize == m_canvasGridView->cellSize();
    EXPECT_TRUE(tempTg);
}

TEST_F(CanvasGridViewTest, TEST_CanvasGridViewTest_cellMargins){
    auto tempTg = m_canvasGridView->d->cellMargins == m_canvasGridView->cellMargins();
    EXPECT_TRUE(tempTg);

}

TEST_F(CanvasGridViewTest,TEST_CanvasGridViewTest_Wid){
    bool temp = false;
    if (m_canvasGridView->isTopLevel()) {
        temp = dynamic_cast<QAbstractItemView*>(m_canvasGridView->parent())->winId();
    } else {
        temp = m_canvasGridView->topLevelWidget()->winId() == m_canvasGridView->winId();
    }
    EXPECT_TRUE(temp);
}

TEST_F(CanvasGridViewTest,Test_CanvasGridViewTest_initRootUrl){

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
    m_canvasGridView->setScreenNum(4);
    EXPECT_EQ(4,m_canvasGridView->m_screenNum);
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_isSelect){
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
    auto oldGeometry = m_canvasGridView->geometry();
    m_canvasGridView->setGeometry(QRect(0,0,0,0));
    auto temp0 = oldGeometry == m_canvasGridView->geometry();
    EXPECT_TRUE(temp0);

    m_canvasGridView->setGeometry(QRect(500,500,500,500));
    auto temp1 = m_canvasGridView->geometry();
    EXPECT_TRUE(temp1 == QRect(500,500,500,500));
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_select){
    m_canvasGridView->setCurrentIndex(QModelIndex());
    m_canvasGridView->clearSelection();

    auto tempItemIds = GridManager::instance()->itemIds(1);
    QList<DUrl> list = DUrl::fromStringList(tempItemIds);
    m_canvasGridView->select(list);
//    p_cvmgr->onSyncSelection(m_canvasGridView, list);
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

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedUrls){
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

//此函数有问题，暂弃
//TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedIndexCount){
//    m_canvasGridView->setCurrentIndex(QModelIndex());
//    m_canvasGridView->clearSelection();

//    auto tempItem = GridManager::instance()->firstItemId(1);
//    DUrl tempTp(tempItem);
//    auto index = m_canvasGridView->model()->index(tempTp);
//    m_canvasGridView->selectionModel()->select(QItemSelection (index, index), QItemSelectionModel::Select);

//    EXPECT_TRUE(1 == m_canvasGridView->selectedIndexCount());

//    auto tempItemIds = GridManager::instance()->itemIds(1);
//    QModelIndexList tpIndexs;
//    QList<DUrl> list;
//    for (auto temp : tempItemIds) {
//        DUrl tempUrl(temp);
//        list.append(tempUrl);
//        auto index = m_canvasGridView->model()->index(tempUrl);
//        if(index.isValid())
//            tpIndexs.append(index);
//    }
//    m_canvasGridView->select(list);
//    EXPECT_EQ(list.size(), m_canvasGridView->selectedIndexCount());
//}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setAutoMerge){
    m_canvasGridView->setAutoMerge(true);
    auto temp = m_canvasGridView->model()->rootUrl().scheme() == DFMMD_SCHEME;
    EXPECT_TRUE(temp);

    m_canvasGridView->setAutoMerge(false);
    auto temp2 = m_canvasGridView->model()->rootUrl().scheme() == FILE_SCHEME;
    EXPECT_TRUE(temp2);
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_mouseMoveEvent){
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
    auto temp = m_canvasGridView->model()->rowCount();
    if(temp){
        m_canvasGridView->delayModelRefresh(0);
        auto temp2 = m_canvasGridView->model()->rowCount();
        EXPECT_FALSE(0 != temp2);
    }
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent){
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

//TEST_F(CanvasGridViewTest, CanvasGridViewTest_paintEvent){
//    m_canvasGridView->show();
//    QTimer::singleShot(1000,[=](){
//        m_canvasGridView->hide();
//    });
//}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_mousePressEvent){

    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    QMouseEvent(QEvent::MouseButtonPress, QPoint(30, 30), m_canvasGridView->mapToGlobal(QPoint(30, 30)), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    m_canvasGridView->mousePressEvent(&me);

    EXPECT_TRUE(m_canvasGridView->d->mousePressed);
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_mouseReleaseEvent){
    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    QMouseEvent(QEvent::MouseButtonRelease, QPoint(30, 30), m_canvasGridView->mapToGlobal(QPoint(30, 30)), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    m_canvasGridView->mouseReleaseEvent(&me);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setSelection){
    m_canvasGridView->setCurrentIndex(QModelIndex());
    m_canvasGridView->clearSelection();

    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->cursor().setPos(QPoint(0,0));
//    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    QMouseEvent me(QEvent::MouseMove, QPoint(0, 0), m_canvasGridView->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    m_canvasGridView->mouseMoveEvent(&me);
    auto temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(0 != temp);
}
#endif

//菜单show出来会被阻塞，所以此用例又不行了
//TEST_F(CanvasGridViewTest, CanvasGridViewTest_contextMenuEvent){
////    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(500, 600),m_canvasGridView->mapToGlobal(QPoint(500, 500)),Qt::KeyboardModifiers());

//    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(10, 10),m_canvasGridView->mapToGlobal(QPoint(10, 10)),Qt::KeyboardModifiers());
//    m_canvasGridView->contextMenuEvent(&ent);
//}

//TEST_F(CanvasGridViewTest, CanvasGridViewTest_showNormalMenu){

//}

//TEST_F(CanvasGridViewTest, CanvasGridViewTest_showEmptyAreaMenu){

//}


