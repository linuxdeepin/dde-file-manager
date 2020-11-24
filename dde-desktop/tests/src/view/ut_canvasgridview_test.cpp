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
#include "../dde-desktop/presenter/gridmanager.h"
#include <private/canvasviewprivate.h>
#include "util/xcb/xcb.h"
#include <QWidget>
#include <QTest>
#include <QEventLoop>
#include "view/desktopitemdelegate.h"
#include "../dde-desktop/desktop.h"
#include "../dde-desktop/desktop.cpp"
#include <QThread>
#include <QProcess>
#include <QScrollBar>
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
    QString tgPath1 = desktopPath + "/ut_testFile.txt";
    QString tgPath2 = desktopPath + "/ut_File.txt";
    QFile file1(tgPath1);
    if(file1.exists())
        QProcess::execute("rm " + tgPath1);
    QFile file2(tgPath1);
    if(file2.exists())
        QProcess::execute("rm " + tgPath2);

    QProcess::execute("touch " + tgPath1);
    QProcess::execute("mv " + tgPath1 +" "  + tgPath2);
    QProcess::execute("rm " + tgPath2);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setCurrentUrl){
    //file:///home/lee/Desktop
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

    if(m_canvasGridView->setCurrentUrl(desktopUrl)){
        EXPECT_TRUE(desktopUrl == m_canvasGridView->model()->rootUrl());
    }
}

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

    m_canvasGridView->d->showSelectRect = true;
    QMouseEvent me2(QEvent::MouseMove, QPoint(500, 600), m_canvasGridView->mapToGlobal(QPoint(500, 600)), Qt::NoButton, Qt::NoButton, Qt::KeyboardModifiers());
    m_canvasGridView->mouseMoveEvent(&me2);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_delayModelRefresh){
    ASSERT_NE(m_canvasGridView, nullptr);
    auto temp = m_canvasGridView->model()->rowCount();
    if(temp){
        m_canvasGridView->delayModelRefresh(0);
        auto temp2 = m_canvasGridView->model()->rowCount();
        EXPECT_FALSE(0 != temp2);
    }
    //增加覆盖
    if (m_canvasGridView->m_refreshTimer == nullptr) {
        m_canvasGridView->m_refreshTimer = new QTimer;
        m_canvasGridView->m_refreshTimer->start(300);
    }

    m_canvasGridView->delayModelRefresh(10);//will reset timer
    int interval = m_canvasGridView->m_refreshTimer->interval();
    EXPECT_EQ(interval, 10);
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
    ASSERT_NE(m_canvasGridView, nullptr);
    //根据Desktop文件夹下文件以及桌面排序情况可能会触发两种右键菜单的某一种
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
          QWidget tempWdg;
          tempWdg.show();
          tempWdg.close();
    });
    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(10, 10),m_canvasGridView->mapToGlobal(QPoint(100, 100)),Qt::KeyboardModifiers());
    timer.start(2000);
    m_canvasGridView->contextMenuEvent(&ent);

    QTimer timer2;
    QObject::connect(&timer2, &QTimer::timeout, [&]{
        timer.stop();
          QWidget tempWdg;
          tempWdg.show();
          tempWdg.close();
    });
    QContextMenuEvent ent2(QContextMenuEvent::Reason::Mouse,QPoint(60, 60),m_canvasGridView->mapToGlobal(QPoint(100, 100)),Qt::KeyboardModifiers());
    timer2.start(2000);
    m_canvasGridView->contextMenuEvent(&ent2);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_showNormalMenu){
    ASSERT_NE(m_canvasGridView, nullptr);
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
    ASSERT_NE(m_canvasGridView, nullptr);
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
    ASSERT_NE(m_canvasGridView, nullptr);
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
    ASSERT_NE(m_canvasGridView, nullptr);
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
    ASSERT_NE(m_canvasGridView, nullptr);
    //new
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();
    //Up
    QKeyEvent keyPressEvt_Up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Up);

    //Tab
    QKeyEvent keyPressEvt_Tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Tab);

    //F1//暂时不覆盖这个，show出来不知道怎么关掉
//    QKeyEvent keyPressEvt_F1(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
//    m_canvasGridView->keyPressEvent(&keyPressEvt_F1);

    //ESC
    QKeyEvent keyPressEvt_Esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Esc);

    //Key_pad
    //Enter//会的打开文件或者文件夹暂时也不覆盖
//    QKeyEvent keyPressEvt_Enter(QEvent::KeyPress, Qt::Key_Enter, Qt::KeypadModifier);
//    m_canvasGridView->keyPressEvent(&keyPressEvt_Enter);

    //pad F5
    QKeyEvent keyPressEvt_padF5(QEvent::KeyPress, Qt::Key_F5, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_padF5);

    //pad Delete
    QKeyEvent keyPressEvt_Key_padDelete(QEvent::KeyPress, Qt::Key_Delete, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_padDelete);

    //pad Space
    QKeyEvent keyPressEvt_Key_Space(QEvent::KeyPress, Qt::Key_Space, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Space);

    //Qt::ShiftModifier:
    //Delete
    QKeyEvent keyPressEvt_Key_ShiftDelete(QEvent::KeyPress, Qt::Key_Delete, Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_ShiftDelete);
    //T
    QKeyEvent keyPressEvt_Key_ShiftT(QEvent::KeyPress, Qt::Key_T, Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_ShiftT);

    //Qt::ControlModifier:
    //Key_H
    QKeyEvent keyPressEvt_Key_CrlH(QEvent::KeyPress, Qt::Key_H, Qt::ControlModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_CrlH);

    //Qt::AltModifier:
    //Key_M
    QKeyEvent keyPressEvt_Key_AltM(QEvent::KeyPress, Qt::Key_M, Qt::AltModifier);
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
          QWidget tempWdg;
          tempWdg.show();
          tempWdg.close();
    });
    timer.start(1000);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_AltM);
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_moveCursorGrid)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    //由于测试桌面的图标排列以及图标个数不明确所以这里不做判断，暂时只是调用

    QAbstractItemView::CursorAction  tpCursorActon;
    Qt::KeyboardModifiers tpModifiers;
    tpModifiers = Qt::NoModifier;

    QList<DUrl> lst;
    QString urlPath = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    lst << DUrl(urlPath);
    m_canvasGridView->select(lst);
    qApp->processEvents();

    auto current = m_canvasGridView->d->currentCursorIndex;
    if (!current.isValid()) {
        current = m_canvasGridView->firstIndex();
        m_canvasGridView->d->currentCursorIndex = current;
    }

    tpCursorActon = QAbstractItemView::CursorAction::MoveLeft;
    m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    tpCursorActon = QAbstractItemView::CursorAction::MoveRight;
    m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    tpCursorActon = QAbstractItemView::CursorAction::MoveUp;
    m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    tpCursorActon = QAbstractItemView::CursorAction::MoveDown;
    m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    tpCursorActon = QAbstractItemView::CursorAction::MovePageUp;
    m_canvasGridView->moveCursorGrid(tpCursorActon, Qt::ShiftModifier);
    tpCursorActon = QAbstractItemView::CursorAction::MovePageDown;
    m_canvasGridView->moveCursorGrid(tpCursorActon, Qt::ShiftModifier);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_initConnection_animation_finish)
{

}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_dragMoveEvent)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    //但是会有一个问题。需要桌面对应这两个点位必须有图标才行
    //先设置选中
    QList<DUrl> lst;
    QString urlPath = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    lst << DUrl(urlPath);
    m_canvasGridView->select(lst);
    qApp->processEvents();
    //模拟拖拽数据
    auto tgIndex = m_canvasGridView->model()->index(DUrl(urlPath));
    QModelIndexList tpIndexes;
    tpIndexes.append(tgIndex);
    QMimeData *tgData = m_canvasGridView->model()->mimeData(tpIndexes);
    //虽然能伪造mimedata但是无法设置drag的source
    //(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction) (0x0007)
//    m_canvasGridView->startDrag(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
    QDragMoveEvent devent1(QPoint(66, 59), Qt::DropAction::MoveAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
    m_canvasGridView->dragMoveEvent(&devent1);
    QDragMoveEvent devent2(QPoint(66, 200), Qt::DropAction::MoveAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
    m_canvasGridView->dragMoveEvent(&devent2);
    QDropEvent dropevent(QPoint(64, 150), Qt::DropAction::IgnoreAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
    m_canvasGridView->dropEvent(&dropevent);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_dropEvent)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    //但是会有一个问题。需要桌面对应这两个点位必须有图标才行
    //先设置选中
    QList<DUrl> lst;
    QString urlPath = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    lst << DUrl(urlPath);
    m_canvasGridView->select(lst);
    qApp->processEvents();
    //模拟拖拽数据
    auto tgIndex = m_canvasGridView->model()->index(DUrl(urlPath));
    QModelIndexList tpIndexes;
    tpIndexes.append(tgIndex);
    QMimeData *tgData = m_canvasGridView->model()->mimeData(tpIndexes);
    //虽然能伪造mimedata但是无法设置drag的source
    //(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction) (0x0007)
//    m_canvasGridView->startDrag(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);

    QDropEvent dropevent(QPoint(64, 200), Qt::MoveAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
    m_canvasGridView->dropEvent(&dropevent);

}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_toggleEntryExpandedState)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    DUrl url = m_canvasGridView->currentUrl();
    url.setScheme(DFMMD_SCHEME);
    if (!url.isValid() || url.scheme() != DFMMD_SCHEME) {
        EXPECT_TRUE(false);
    }
    m_canvasGridView->toggleEntryExpandedState(url);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_moveCursor)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->d->currentCursorIndex = QModelIndex();
    m_canvasGridView->moveCursor(QAbstractItemView::MoveDown, Qt::KeyboardModifier::MetaModifier);
    EXPECT_TRUE(m_canvasGridView->firstIndex() == m_canvasGridView->d->currentCursorIndex);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_firstIndex)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    auto localFile = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    auto tempIndex = m_canvasGridView->model()->index(DUrl(localFile));
    auto tempIndexCanvs = m_canvasGridView->firstIndex();
    bool theSameOne = tempIndex == tempIndexCanvs;
    EXPECT_TRUE(theSameOne);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_lastIndex)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    auto localFile = GridManager::instance()->lastItemTop(m_canvasGridView->m_screenNum);
    auto tempIndex = m_canvasGridView->model()->index(DUrl(localFile));
    auto tempIndexCanvs = m_canvasGridView->lastIndex();
    bool theSameOne = tempIndex == tempIndexCanvs;
    EXPECT_TRUE(theSameOne);
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_select){
    ASSERT_NE(m_canvasGridView, nullptr);

    m_canvasGridView->selectionModel()->clearSelection();
    auto count1 = m_canvasGridView->selectionModel()->selectedCount();
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();
    auto count2 = m_canvasGridView->selectionModel()->selectedCount();
    auto expectValue = count1 != count2;
    EXPECT_TRUE(expectValue);
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_setSelection){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->d->mousePressed = false;
    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->setSelection(QRect(QPoint(2, 3),QPoint(6, 8)), QItemSelectionModel::Select);

    m_canvasGridView->d->mousePressed = false;
    m_canvasGridView->d->showSelectRect = true;
    QTest::keyPress(m_canvasGridView, Qt::Key_Shift);
    QTest::mouseClick(m_canvasGridView, Qt::MouseButton::LeftButton, Qt::ShiftModifier,QPoint(6, 8));
//    auto tpp = DFMGlobal::keyShiftIsPressed();
    m_canvasGridView->setSelection(QRect(QPoint(2, 3),QPoint(6, 8)), QItemSelectionModel::Select);

    m_canvasGridView->d->mousePressed = true;
    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->setSelection(QRect(QPoint(2, 3),QPoint(6, 8)), QItemSelectionModel::Select);
}


TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedIndexCount){
    ASSERT_NE(m_canvasGridView, nullptr);

    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();
    auto tempCnt1 = m_canvasGridView->selectionModel()->selectedIndexes().size();
    qApp->processEvents();
    auto tempCnt2= GridManager::instance()->allItems().size();
    auto expectValue = tempCnt1 == tempCnt2;
    EXPECT_TRUE(expectValue);
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedUrls){
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->selectionModel()->clearSelection();
    auto count1 = m_canvasGridView->selectionModel()->selectedCount();
    EXPECT_TRUE( count1 == 0);
    qApp->processEvents();
    m_canvasGridView->selectAll();

    qApp->processEvents();
    auto count2 = m_canvasGridView->selectedUrls().size();
    if(GridManager::instance()->allItems().size() != 0)
        EXPECT_TRUE( count2 != count1);
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_itemPaintGeomertys)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    auto tpUrl = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    auto tpIndex = m_canvasGridView->model()->index(DUrl(tpUrl));
    auto tpGeomertrys =  m_canvasGridView->itemPaintGeomertys(tpIndex);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_visualRect)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    auto tpUrl = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    auto tpIndex = m_canvasGridView->model()->index(DUrl(tpUrl));
    auto tgRct = m_canvasGridView->visualRect(tpIndex);
    auto expectValue = tgRct != QRect(0, 0, 0, 0);
    EXPECT_TRUE(expectValue);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_updateHiddenItems)
{
    m_canvasGridView->updateHiddenItems();
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_horizontalOffset)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    auto hoset1 = m_canvasGridView->horizontalScrollBar()->value();
    auto hoset2 = m_canvasGridView->horizontalOffset();
    auto expectValue = hoset1 == hoset2;
    EXPECT_TRUE(expectValue);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_verticalOffset)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    auto voset1 = m_canvasGridView->verticalScrollBar()->value();
    auto  voset2= m_canvasGridView->verticalOffset();
    auto expectValue = voset1 == voset2;
    EXPECT_TRUE(expectValue);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_delayArrage)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setAutoMerge(false);
    EXPECT_TRUE(!GridManager().instance()->autoMerge());

    //why twice?
    m_canvasGridView->delayArrage(5);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_delayAutoMerge_refresh)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->setAutoMerge(true);
    EXPECT_TRUE(GridManager().instance()->autoMerge());

    //why twice?
    m_canvasGridView->delayAutoMerge(0);
    m_canvasGridView->delayAutoMerge(0);
    EXPECT_EQ(nullptr, m_canvasGridView->m_refreshTimer);
    //why twice?
    m_canvasGridView->m_refreshTimer = new QTimer();
    m_canvasGridView->delayModelRefresh(0);
    m_canvasGridView->delayModelRefresh(0);
    EXPECT_EQ(nullptr, m_canvasGridView->m_refreshTimer);
    m_canvasGridView->delayModelRefresh(50);
    QEventLoop loop;
    QTimer::singleShot(100, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_delayAutoMerge_autoMergeSelectedUrls)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    m_canvasGridView->selectAll();
    qApp->processEvents();
    auto tpLst = m_canvasGridView->autoMergeSelectedUrls();
    qApp->processEvents();
}

//new

TEST_F(CanvasGridViewTest, canvasGridViewTest_startDrag)
{
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();
    m_canvasGridView->d->touchTimer.stop();
    QTimer timer;
    QTimer::singleShot(500, &timer, [this]{
        QTest::mousePress(m_canvasGridView, Qt::LeftButton);
        QTest::mouseMove(m_canvasGridView, QPoint(300, 100));
        QTest::qSleep(1000);
        QTest::mouseRelease(m_canvasGridView, Qt::LeftButton);
    });
    timer.start(200);
    m_canvasGridView->startDrag(Qt::MoveAction);
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_fetchDragEventUrlsFromSharedMemory)
{
    bool ret = m_canvasGridView->fetchDragEventUrlsFromSharedMemory();
    EXPECT_EQ(ret, true);
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_delayCustom)
{
    GridManager::instance()->setAutoMerge(false);
    m_canvasGridView->delayCustom(0);
    m_canvasGridView->delayCustom(50);
    QEventLoop loop;
    QTimer::singleShot(100, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_handleContextMenuAction)
{
    m_canvasGridView->handleContextMenuAction(CanvasGridView::WallpaperSettings);
    QEventLoop loop;
    QTimer::singleShot(200, &loop, [this, &loop]{
        Desktop* desktop = Desktop::instance();
        char *base = (char *)(desktop->d.data());
        char *wpVar = base + sizeof (void *) * 2;
        desktop->preInit();
        QTest::qSleep(1000);
        void *wallpaper1 = (void *)*(long * )(wpVar);
        void *wallpaper2 = (void *)*(long * )(wpVar);
        Frame *wallpaper = (Frame *)wallpaper2;
        if (wallpaper)
           wallpaper->hide();
        loop.exit();
    });
    loop.exec();


    m_canvasGridView->handleContextMenuAction(CanvasGridView::DisplaySettings);
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    path = path + '/' + "test.txt";
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }
    m_canvasGridView->handleContextMenuAction(DFMGlobal::Name);
    m_canvasGridView->handleContextMenuAction(CanvasGridView::IconSize4);
    int level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_EQ(level, CanvasGridView::IconSize4 - CanvasGridView::IconSize0);
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_paintEvent)
{
    m_canvasGridView->d->startDodge = true;
    m_canvasGridView->d->dodgeAnimationing = false;
    m_canvasGridView->d->colCount = 2;
    m_canvasGridView->d->rowCount = 2;
    QPaintEvent event(QRect(0,0,200,200));
    m_canvasGridView->paintEvent(&event);
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_autoMergeSelectedUrls)
{
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();
    DUrlList list = m_canvasGridView->selectedUrls();
    QString str = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QFile file;
    str = str + '/' + "test.txt";
    file.setFileName(str);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite|QIODevice::NewOnly);
        file.close();
    }
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();

    QTest::qSleep(5050);
    m_canvasGridView->autoMergeSelectedUrls();
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_moveCursor)
{
    QModelIndex index = m_canvasGridView->d->currentCursorIndex;
    QModelIndex ret;
    ret = m_canvasGridView->moveCursor(QAbstractItemView::CursorAction::MoveDown, Qt::ControlModifier);
    EXPECT_EQ(ret, index);

    m_canvasGridView->d->currentCursorIndex = m_canvasGridView->firstIndex();
    ret = m_canvasGridView->moveCursor(QAbstractItemView::CursorAction::MoveDown, Qt::NoModifier);
    EXPECT_EQ(ret, m_canvasGridView->firstIndex());
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_decreaseIcon)
{
    m_canvasGridView->itemDelegate()->setIconSizeByIconSizeLevel(2);
    m_canvasGridView->decreaseIcon();
    EXPECT_EQ(m_canvasGridView->itemDelegate()->iconSizeLevel(), 1);
    m_canvasGridView->d->dodgeDelayTimer.start(10);
    QEventLoop loop;
    QTimer::singleShot(100, &loop, [&loop, this]{
        this->m_canvasGridView->d->dodgeDelayTimer.stop();
        loop.exit();
    });
    loop.exec();
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_gridat)
{
    QPoint pos(300, 200);
    QPoint point = m_canvasGridView->gridAt(QPoint(300, 200));
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();

    DUrlList ulist = m_canvasGridView->selectedUrls();
    if (ulist.size() > 0) {
        const QModelIndex &index = m_canvasGridView->model()->index(ulist[0]);
        QRect rect = m_canvasGridView->visualRect(index);

        auto row = (pos.x() - m_canvasGridView->d->viewMargins.left()) / rect.width();
        auto col = (pos.y() - m_canvasGridView->d->viewMargins.top()) / rect.height();

        EXPECT_EQ(row, point.x());
        EXPECT_EQ(col, point.y());
    }
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_syncIconLevel)
{
    int level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    m_canvasGridView->syncIconLevel(level);
    EXPECT_EQ(level, m_canvasGridView->itemDelegate()->iconSizeLevel());
    if (level > 0) level--;
    else level++;
    m_canvasGridView->syncIconLevel(level);
    EXPECT_EQ(level, m_canvasGridView->itemDelegate()->iconSizeLevel());
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_openUrls)
{
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();

    QTest::qSleep(1000);
    DUrlList ulist = m_canvasGridView->selectedUrls();
    QList<DUrl> list;
    m_canvasGridView->openUrls(list);

    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    path = path + '/' + "test.txt";
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }
    if (ulist.size() > 0) list << ulist[0];
    if (list.size() <= 0) list << path;
    if (list.size() > 0) {
        m_canvasGridView->openUrls(list);
        const QModelIndex &index = m_canvasGridView->model()->index(list[0]);
        QRect rect = m_canvasGridView->rectForIndex(index);
        EXPECT_EQ(rect.width(), m_canvasGridView->d->cellWidth);
    }
    QString temp = m_canvasGridView->canvansScreenName();
    EXPECT_EQ(temp, m_canvasGridView->m_screenName);
}
