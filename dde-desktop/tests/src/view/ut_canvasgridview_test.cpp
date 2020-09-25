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

TEST_F(CanvasGridViewTest, CanvasGridViewTest_contextMenuEvent){
    //根据Desktop文件夹下文件以及桌面排序情况可能会触发两种右键菜单的某一种
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
          QWidget tempWdg;
          tempWdg.show();
          tempWdg.close();
    });
    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(100, 100),m_canvasGridView->mapToGlobal(QPoint(100, 100)),Qt::KeyboardModifiers());
    timer.start(2000);
    m_canvasGridView->contextMenuEvent(&ent);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_showNormalMenu){
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
          QWidget tempWdg;
          tempWdg.show();
          tempWdg.close();
    });

    auto tempFlags = (Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);// (0x00a7)
    auto localFile = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    auto tempIndex = m_canvasGridView->model()->index(DUrl(localFile));
    timer.start(2000);
    m_canvasGridView->showNormalMenu(tempIndex, tempFlags);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_showEmptyAreaMenu){
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
          QWidget tempWdg;
          tempWdg.show();
          tempWdg.close();
    });

    auto tempFlags = (Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);// (0x00a5)
    timer.start(2000);
    m_canvasGridView->showEmptyAreaMenu(tempFlags);
}


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

TEST_F(CanvasGridViewTest, CanvasGridViewTest_firstIndex)
{
    auto localFile = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    auto tempIndex = m_canvasGridView->model()->index(DUrl(localFile));
    auto tempIndexCanvs = m_canvasGridView->firstIndex();
    bool theSameOne = tempIndex == tempIndexCanvs;
    EXPECT_TRUE(theSameOne);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_lastIndex)
{
    auto localFile = GridManager::instance()->lastItemTop(m_canvasGridView->m_screenNum);
    auto tempIndex = m_canvasGridView->model()->index(DUrl(localFile));
    auto tempIndexCanvs = m_canvasGridView->lastIndex();
    bool theSameOne = tempIndex == tempIndexCanvs;
    EXPECT_TRUE(theSameOne);
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_select){
    ASSERT_NE(m_canvasGridView, nullptr);
    //todo
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_setSelection){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->d->mousePressed = false;
    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->setSelection(QRect(QPoint(2, 3),QPoint(6, 8)), QItemSelectionModel::Select);
}


TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedIndexCount){
    ASSERT_NE(m_canvasGridView, nullptr);
    //todo
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedUrls){
    ASSERT_NE(m_canvasGridView, nullptr);
    //todo
}


