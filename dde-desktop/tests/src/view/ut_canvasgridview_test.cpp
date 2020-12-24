#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QThread>
#include <QProcess>
#include <QScrollBar>
#include <QWidget>
#include <QTest>
#include <QEventLoop>
#include <QAbstractScrollArea>
#include <QEvent>
#include <QWidget>

#include <dfilemenu.h>
#include <DFileDragClient>
#include "../stub-ext/stubext.h"


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
#include "view/desktopitemdelegate.h"
#include "../dde-desktop/desktop.h"
#include "../dde-desktop/desktop.cpp"
#include "dfileviewhelper.h"
#include "../third-party/cpp-stub/addr_pri.h"
#include "../third-party/cpp-stub/stub.h"
#include "diconitemdelegate.h"
#include "dfmsettings.h"
#include "dfmglobal.h"
#include "dfmapplication.h"
#include "dfileservices.h"
#include "../models/desktopfileinfo.h"
#include "qdbusabstractinterface.h"
#include "qabstractitemview.h"
#include "dapplication.h"
#include "dfiledragclient.h"
using namespace std;
using namespace stub_ext;

class QAbstractItemModel;
class CanvasGridViewTest : public testing::Test
{
public:
    CanvasGridViewTest():Test(){
    }
    virtual void SetUp() override{
        //以防桌面没文件
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        QFile file(path + '/' + tstFile);
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.write("test");
        file.close();

        QDir dir;
        dir.mkpath(path + '/' + tstDir);

        m_cvmgr.reset(new CanvasViewManager(new BackgroundManager()));
        for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
            if(1 == tpCanvas->screenNum()){
                m_canvasGridView = tpCanvas.data();
                break;
            }
        }
    }

    virtual void TearDown()override{
        QDir dir(path);
        dir.remove(tstFile);
        dir.rmpath(dir.filePath(tstDir));
    }
public:
    QScopedPointer<CanvasViewManager> m_cvmgr;
    CanvasGridView *m_canvasGridView{nullptr};
    QString tstFile = "testutxxxxxxxxxxx.txt";
    QString tstDir = "test-dir-utxxxxxxxxxx";
    QString path;
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
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(GridManager, delaySyncAllProfile), [](){return;});
    stu.set_lamda(ADDR(CanvasViewManager, onSyncOperation), [](){return;});
    m_canvasGridView->m_refreshTimer->start(1);
    qApp->processEvents();
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
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(), QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier, Qt::MouseEventSynthesizedByQt);
    bool(*select)() = [](){return true;};
    Stub stu;
    stu.set(ADDR(CanvasGridView, isSelected), select);
    m_canvasGridView->mousePressEvent(&me);
    m_canvasGridView->mousePressEvent(&event);
    EXPECT_TRUE(m_canvasGridView->d->mousePressed);
}


TEST_F(CanvasGridViewTest, CanvasGridViewTest_mouseReleaseEvent){
    ASSERT_NE(m_canvasGridView, nullptr);
    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    QMouseEvent(QEvent::MouseButtonRelease, QPoint(30, 30), m_canvasGridView->mapToGlobal(QPoint(30, 30)), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());

    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->d->selectRect = QRect(QPoint(0, 0), QPoint(100, 100));
    bool(*judge)() = [](){return true;};
    static QModelIndex index = m_canvasGridView->property("lastPressedIndex").toModelIndex();
    QModelIndex(*indx)() = [](){return index;};

    Stub stu,stu1;
    stu.set(ADDR(QModelIndex, isValid), judge);
    stu.set(ADDR(DFMGlobal, keyCtrlIsPressed), judge);
    stu.set(ADDR(CanvasGridView, isSelected), judge);
    m_canvasGridView->mouseReleaseEvent(&me);
    EXPECT_EQ(m_canvasGridView->d->selectRect, QRect());

}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_contextMenuEvent_empty){
    ASSERT_NE(m_canvasGridView, nullptr);

    stub_ext::StubExt st;
    st.set_lamda(VADDR(CanvasGridView,indexAt),[](){return QModelIndex();});
    st.set_lamda(ADDR(DFileViewHelper,isEmptyArea),[](){return true;});

    int menu = 0;
    st.set_lamda(ADDR(CanvasGridView,showEmptyAreaMenu),[&menu](){menu = 1;return;});
    st.set_lamda(ADDR(CanvasGridView,showNormalMenu),[&menu](){menu = 2;return;});

    //根据Desktop文件夹下文件以及桌面排序情况可能会触发两种右键菜单的某一种
    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(10, 10),m_canvasGridView->mapToGlobal(QPoint(100, 100)),Qt::KeyboardModifiers());
    m_canvasGridView->contextMenuEvent(&ent);
    EXPECT_EQ(1,menu);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_contextMenuEvent_normal){
    ASSERT_NE(m_canvasGridView, nullptr);

    while (m_canvasGridView->model()->state() == DFileSystemModel::Busy) {
        qApp->processEvents();
    }

    auto urls = m_canvasGridView->model()->sortedUrls();
    if (urls.isEmpty())
        return;

    auto index = m_canvasGridView->model()->index(urls.first());
    if (!index.isValid())
        return;

    stub_ext::StubExt st;
    st.set_lamda(VADDR(CanvasGridView,indexAt),[=](){return index;});
    st.set_lamda(ADDR(DFileViewHelper,isEmptyArea),[](){return false;});

    int menu = 0;
    st.set_lamda(ADDR(CanvasGridView,showEmptyAreaMenu),[&menu](){menu = 1;return;});
    st.set_lamda(ADDR(CanvasGridView,showNormalMenu),[&menu](){menu = 2;return;});

    //根据Desktop文件夹下文件以及桌面排序情况可能会触发两种右键菜单的某一种
    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(10, 10),m_canvasGridView->mapToGlobal(QPoint(100, 100)),Qt::KeyboardModifiers());
    m_canvasGridView->contextMenuEvent(&ent);
    EXPECT_EQ(2,menu);
}

static int stubRet = 0;
TEST_F(CanvasGridViewTest, CanvasGridViewTest_showNormalMenu){
    ASSERT_NE(m_canvasGridView, nullptr);
    auto tempFlags = (Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);// (0x00a7)
    auto localFile = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    ASSERT_FALSE(localFile.isEmpty());
    while (m_canvasGridView->model()->state() == DFileSystemModel::Busy) {
        qApp->processEvents();
    }
    auto tempIndex = m_canvasGridView->model()->index(DUrl(localFile));
    ASSERT_TRUE(tempIndex.isValid());

    stubRet = 0;
    Stub st;
    auto exec_stub = (QAction* (*)())([](){
        stubRet = 1;
        return (QAction*)nullptr;
    });
    auto exec_foo = (QAction* (DFileMenu::*)())&DFileMenu::exec;
    st.set(exec_foo, exec_stub);

    auto noneWld = (bool(*)())([](){return false;});
    st.set(ADDR(DesktopInfo,waylandDectected), noneWld);
    m_canvasGridView->showNormalMenu(tempIndex, tempFlags);
    EXPECT_EQ(stubRet, 1);

    stubRet = 0;
    st.reset(ADDR(DesktopInfo,waylandDectected));
    auto wld = (bool(*)())([](){return true;});
    st.set(ADDR(DesktopInfo,waylandDectected), wld);

    auto execlop_stub = (int (*)())([](){stubRet = 1; return 0;});
    st.set(ADDR(QEventLoop,exec), execlop_stub);
    m_canvasGridView->showNormalMenu(tempIndex, tempFlags);
    EXPECT_EQ(stubRet, 1);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_showEmptyAreaMenu){
    ASSERT_NE(m_canvasGridView, nullptr);
    auto tempFlags = (Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);// (0x00a5)

    stubRet = 0;
    Stub st;
    auto exec_stub = (QAction* (*)())([](){
        stubRet = 1;
        return (QAction*)nullptr;
    });
    auto exec_foo = (QAction* (DFileMenu::*)())&DFileMenu::exec;
    st.set(exec_foo, exec_stub);

    auto noneWld = (bool(*)())([](){return false;});
    st.set(ADDR(DesktopInfo,waylandDectected), noneWld);
    m_canvasGridView->showEmptyAreaMenu(tempFlags);
    EXPECT_EQ(stubRet, 1);

    stubRet = 0;
    st.reset(ADDR(DesktopInfo,waylandDectected));
    auto wld = (bool(*)())([](){return true;});
    st.set(ADDR(DesktopInfo,waylandDectected), wld);

    auto execlop_stub = (int (*)())([](){stubRet = 1; return 0;});
    st.set(ADDR(QEventLoop,exec), execlop_stub);
    m_canvasGridView->showEmptyAreaMenu(tempFlags);
    EXPECT_EQ(stubRet, 1);

    DFileMenu* menu = new DFileMenu;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DFileMenuManager, genereteMenuByKeys), [&menu](){return menu;});
    stu.set_lamda(ADDR(DesktopInfo, waylandDectected), [](){return true;});
    QAction action;
    stu.set_lamda(ADDR(CanvasGridView, handleContextMenuAction), [](){return;});
    emit menu->triggered(&action);
    emit menu->QMenu::aboutToHide();
    qApp->processEvents();
    m_canvasGridView->showEmptyAreaMenu(tempFlags);


    stu.set_lamda(ADDR(QWidget, parentWidget), [](){return nullptr;});
    qApp->processEvents();
    m_canvasGridView->showEmptyAreaMenu(tempFlags);

    stu.set_lamda(ADDR(QSize, width), [](){return 10;});
    stu.set_lamda(ADDR(QSize, height), [](){return 10;});
    stu.set_lamda(ADDR(QRect, bottom), [](){return 5;});
    stu.set_lamda(ADDR(QRect, right), [](){return 5;});
    m_canvasGridView->showEmptyAreaMenu(tempFlags);
    if (menu) delete menu;
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setIconByLevel)
{
    ASSERT_NE(m_canvasGridView, nullptr);

    m_canvasGridView->setIconByLevel(1);
    DesktopItemDelegate* p =m_canvasGridView->itemDelegate();
    ASSERT_EQ(1, p->iconSizeLevel());

    StubExt st;
    int iconLevel = 0;
    st.set_lamda(ADDR(Presenter,OnIconLevelChanged),[&iconLevel](Presenter *,int lv){iconLevel = lv;});
    m_canvasGridView->setIconByLevel(1);

    EXPECT_EQ(1, p->iconSizeLevel());
    EXPECT_EQ(0, iconLevel);

    m_canvasGridView->setIconByLevel(3);
    EXPECT_EQ(3, p->iconSizeLevel());
    EXPECT_EQ(3, iconLevel);
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
    //添加文件
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    path = path + '/' + "test.txt";
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }

    static QModelIndexList mlist;
    mlist << m_canvasGridView->firstIndex() << QModelIndex() << m_canvasGridView->model()->index(DUrl::fromLocalFile(path));
    QModelIndexList(*list)() = [](){return mlist;};

    stub_ext::StubExt stu;

    stu.set(ADDR(DFileSelectionModel, selectedIndexes), list);

    m_canvasGridView->keyPressEvent(&keyPressEvt_Up);

    //Tab
    QKeyEvent keyPressEvt_Tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Tab);

    //F1//暂时不覆盖这个，show出来不知道怎么关掉
//    QKeyEvent keyPressEvt_F1(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
//    m_canvasGridView->keyPressEvent(&keyPressEvt_F1);

    //ESC
    QKeyEvent keyPressEvt_Esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QString str = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    QList<QUrl> lurl;
    lurl << QUrl::fromLocalFile(str);
    bool isfetch = false;
    stu.set_lamda(ADDR(DFMGlobal, fetchUrlsFromClipboard), [lurl, &isfetch](){isfetch = true; return lurl;});
    stu.set_lamda(VADDR(DAbstractFileInfo, path), [str](){return str;});
    m_canvasGridView->keyPressEvent(&keyPressEvt_Esc);
    EXPECT_TRUE(isfetch);

    //Key_pad
    //Enter
    QKeyEvent keyPressEvt_Enter(QEvent::KeyPress, Qt::Key_Enter, Qt::KeypadModifier);
    //为了避免打开太多文件,对openfiles进行函数转换
    {
        stub_ext::StubExt tub;
        tub.set_lamda(ADDR(DFileService, openFiles), [](){return true;});
        tub.set_lamda(VADDR(DAbstractFileInfo, isVirtualEntry), [](){return true;});
        m_canvasGridView->keyPressEvent(&keyPressEvt_Enter);

        bool myValid = false;
        tub.set_lamda(VADDR(QModelIndex, isValid), [&myValid](){myValid = true; return true;});
        m_canvasGridView->keyPressEvent(&keyPressEvt_Enter);
    }
    //pad F5
    QKeyEvent keyPressEvt_padF5(QEvent::KeyPress, Qt::Key_F5, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_padF5);

    //pad Delete
    QKeyEvent keyPressEvt_Key_padDelete(QEvent::KeyPress, Qt::Key_Delete, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_padDelete);

    //pad Space
    {
        stub_ext::StubExt tub;
        QKeyEvent keyPressEvt_Key_Space(QEvent::KeyPress, Qt::Key_Space, Qt::KeypadModifier);
        static bool spacejudge = false;
        void(*myspace)() = [](){spacejudge = true;};
        tub.set(ADDR(DFMGlobal, showFilePreviewDialog), myspace);
        m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Space);
        EXPECT_TRUE(spacejudge);
    }

    //other 直接break
    QKeyEvent keyPressEvt_Key_D(QEvent::KeyPress, Qt::Key_D, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_D);

    //Qt::ShiftModifier:
    //Delete
    QKeyEvent keyPressEvt_Key_ShiftDelete(QEvent::KeyPress, Qt::Key_Delete, Qt::ShiftModifier);
    QString depath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    depath = depath + '/' + "test.txt";
    QFile dfile(depath);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }
    mlist.clear();
    mlist << m_canvasGridView->model()->index(DUrl::fromLocalFile(depath));
    stu.set(ADDR(DFileSelectionModel, selectedIndexes), list);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_ShiftDelete);

    //T
    QKeyEvent keyPressEvt_Key_ShiftT(QEvent::KeyPress, Qt::Key_T, Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_ShiftT);

    //Qt::ControlModifier:
    //Key_H
    QKeyEvent keyPressEvt_Key_CrlH(QEvent::KeyPress, Qt::Key_H, Qt::ControlModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_CrlH);

    //Key_Minus
    QKeyEvent keyPressEvt_Key_Minus(QEvent::KeyPress, Qt::Key_Minus, Qt::ControlModifier);
    m_canvasGridView->setIconByLevel(3);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Minus);
    int level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_TRUE(2 == level);

    //Key_Equal
    QKeyEvent keyPressEvt_Key_Equal(QEvent::KeyPress, Qt::Key_Equal, Qt::ControlModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Equal);
    level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_TRUE(3 == level);

    //Key_I
    QKeyEvent keyPressEvt_Key_I(QEvent::KeyPress, Qt::Key_I, Qt::ControlModifier);
    void(*dig)() = [](){return;};
    stu.set(ADDR(DFMGlobal, showPropertyDialog), dig);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_I);


    //Key_7
    QKeyEvent keyPressEvt_Key_7(QEvent::KeyPress, Qt::Key_7, Qt::ControlModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_7);

    //Qt::ControlModifier | Qt::ShiftModifier
    //Qt::Key_N
    QKeyEvent keyPressEvt_Key_N(QEvent::KeyPress, Qt::Key_N, Qt::ControlModifier | Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_N);

    //Qt::Key_Question
    QKeyEvent keyPressEvt_Key_Question(QEvent::KeyPress, Qt::Key_Question, Qt::ControlModifier | Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Question);

    //Qt::Qt::Key_Plus
    QKeyEvent keyPressEvt_Key_Plus(QEvent::KeyPress, Qt::Key_Plus, Qt::ControlModifier | Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Plus);
    level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_TRUE(4 == level);

    //Qt::AltModifier:
    //Key_M
    {
        stub_ext::StubExt stu;
        QKeyEvent keyPressEvt_Key_AltM(QEvent::KeyPress, Qt::Key_M, Qt::AltModifier);
        QTimer timer;
        QObject::connect(&timer, &QTimer::timeout, [&]{
            timer.stop();
              QWidget tempWdg;
              tempWdg.show();
              tempWdg.close();
        });
        timer.start(1000);
        stu.set_lamda(ADDR(QVariant, isValid), [](){return true;});

        bool judge = false;
        stu.set_lamda(ADDR(CanvasGridView, showNormalMenu),[&judge](){judge = !judge; return;});
        stu.set_lamda(ADDR(CanvasGridView, showEmptyAreaMenu),[&judge](){judge = !judge; return;});
        m_canvasGridView->keyPressEvent(&keyPressEvt_Key_AltM);
        EXPECT_TRUE(judge);

        bool isgset = false;
        stu.set_lamda(ADDR(GridManager, isGsettingShow), [&isgset](){isgset = true; return false;});
        m_canvasGridView->keyPressEvent(&keyPressEvt_Key_AltM);
        EXPECT_TRUE(isgset);

        stu.set_lamda(ADDR(QVariant, isValid), [](){return false;});
        m_canvasGridView->keyPressEvent(&keyPressEvt_Key_AltM);
    }
    //key_10
    QKeyEvent keyPressEvt_Key_R(QEvent::KeyPress, Qt::Key_R, Qt::AltModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_R);

    //
    {
        stub_ext::StubExt stu;
        QKeyEvent keyPressEvt_Key(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        stu.set_lamda(ADDR(QVariant, toBool), [](){return true;});
        m_canvasGridView->keyPressEvent(&keyPressEvt_Key);
        QKeyEvent keyPressEvt_Key_8(QEvent::KeyPress, Qt::Key_8, Qt::NoModifier);
        m_canvasGridView->keyPressEvent(&keyPressEvt_Key_8);
    }
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

    tpCursorActon = QAbstractItemView::CursorAction::MoveHome;
    m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
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
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_dropEvent)
{
    ASSERT_NE(m_canvasGridView, nullptr);

    QString urlPath = "file://" + path + "/" + tstFile;
    DUrl urlFile(urlPath);
    DUrl urlDir("file://" + path + "/" + tstDir);
    //模拟拖拽数据
    auto tgIndex = m_canvasGridView->model()->index(urlFile);
    QModelIndexList tpIndexes;
    tpIndexes.append(tgIndex);
    QMimeData *tgData = m_canvasGridView->model()->mimeData(tpIndexes);

    //回收站,计算机等
    {
        auto dropIndex = m_canvasGridView->model()->index(urlDir);
        StubExt stu;
        QDropEvent dropevent(QPoint(64, 150), Qt::DropAction::CopyAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
        stu.set_lamda(ADDR(DFileSelectionModel,selectedIndexes),[=](){return tpIndexes;});
        stu.set_lamda(VADDR(CanvasGridView,indexAt),[=](){return dropIndex;});
        m_canvasGridView->m_urlsForDragEvent.append(DesktopFileInfo::computerDesktopFileUrl());
        m_canvasGridView->m_urlsForDragEvent.append(DesktopFileInfo::homeDesktopFileUrl());
        m_canvasGridView->m_urlsForDragEvent.append(DesktopFileInfo::trashDesktopFileUrl());
        m_canvasGridView->dropEvent(&dropevent);
        EXPECT_EQ(dropevent.dropAction(), Qt::IgnoreAction);
        m_canvasGridView->m_urlsForDragEvent.clear();
    }

    //外部拖入
    {
        StubExt stu;
        stu.set_lamda(&QDropEvent::source,[this](){return nullptr;});
        stu.set_lamda(VADDR(CanvasGridView,indexAt),[=](){return QModelIndex();});
        stu.set_lamda(ADDR(DFileSelectionModel,selectedIndexes),[=](){return tpIndexes;});
        bool droped = false;
        stu.set_lamda(VADDR(DFileSystemModel,dropMimeData),[&](){droped = true;return true;});
        stu.set_lamda(&DFileDragClient::checkMimeData,[](){return false;});

        {
            QDropEvent dropevent(QPoint(64, 200), Qt::MoveAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
            m_canvasGridView->dropEvent(&dropevent);
            EXPECT_FALSE(tgData->property("DirectSaveUrl").isValid());
            EXPECT_TRUE(droped);
        }

        //DirectSaveMode
        {
            droped = false;
            QDropEvent dropevent(QPoint(64, 200), Qt::MoveAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
            stu.reset(&DFileDragClient::checkMimeData);
            bool checkMimeData = false;
            stu.set_lamda(&DFileDragClient::checkMimeData,[&checkMimeData](){checkMimeData = true;return true;});
            tgData->setProperty("IsDirectSaveMode",true);
            m_canvasGridView->dropEvent(&dropevent);

            EXPECT_TRUE(checkMimeData);
            EXPECT_TRUE(tgData->property("DirectSaveUrl").isValid());
            EXPECT_FALSE(droped);
            tgData->setProperty("IsDirectSaveMode",false);
        }
    }

    //内部拖动
    {
        QDropEvent dropevent(QPoint(64, 200), Qt::MoveAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
        StubExt stu;
        stu.set_lamda(&QDropEvent::source,[this](){return m_canvasGridView;});
        stu.set_lamda(VADDR(CanvasGridView,indexAt),[=](){return QModelIndex();});
        stu.set_lamda(ADDR(DFileSelectionModel,selectedIndexes),[=](){return tpIndexes;});

        bool move = false;
        stu.set_lamda(ADDR(GridManager,sigSyncOperation),[&move](){move = true;});

        bool checkMimeData = false;
        stu.set_lamda(&DFileDragClient::checkMimeData,[&checkMimeData](){checkMimeData = true;return true;});

        m_canvasGridView->dropEvent(&dropevent);
        EXPECT_TRUE(move);
        EXPECT_FALSE(checkMimeData);
    }
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
    m_canvasGridView->delayArrage(5);//第一次内部创建Static Qtimer

    m_canvasGridView->delayArrage(0);//实现覆盖
    QEventLoop loop;
    QTimer::singleShot(50, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();
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
    stub_ext::StubExt stu;
    DFileSelectionModel* model = new DFileSelectionModel;
    model->m_currentCommand = QItemSelectionModel::SelectionFlags(QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    stu.set_lamda(ADDR(CanvasGridView, selectionModel), [model](){return model;});
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
    bool judge = false;
    stub_ext::StubExt stu;
    stu.set_lamda(VADDR(QAbstractItemView, startDrag), [&judge](){judge = !judge; return;});
    m_canvasGridView->startDrag(Qt::MoveAction);
    EXPECT_TRUE(judge);
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_fetchDragEventUrlsFromSharedMemory)
{
    bool ret = m_canvasGridView->fetchDragEventUrlsFromSharedMemory();
    EXPECT_EQ(ret, true);
    stub_ext::StubExt stu;

    stu.set_lamda(ADDR(QSharedMemory, attach), [](){return false;});
    ret = m_canvasGridView->fetchDragEventUrlsFromSharedMemory();
    EXPECT_EQ(ret, false);
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

    stub_ext::StubExt stu;
    bool ismerge = false;
    stu.set_lamda(ADDR(GridManager, autoMerge), [&ismerge](){ismerge = true; return true;});
}

TEST_F(CanvasGridViewTest, canvasGridViewTest_handleContextMenuAction)
{
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(Desktop, showWallpaperSettings), [&judge](){judge = !judge; return;});
    m_canvasGridView->handleContextMenuAction(CanvasGridView::WallpaperSettings);
    EXPECT_TRUE(judge);
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

    stu.set_lamda(ADDR(QDBusAbstractInterface, asyncCall), [&judge]{judge = !judge; return QDBusPendingCall(nullptr);});
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

    QTest::qSleep(500);
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

    bool(*mymode)() = [](){return true;};
    Stub tub;
    tub.set(ADDR(QModelIndex, isValid), mymode);
    ret = m_canvasGridView->moveCursor(QAbstractItemView::CursorAction::MoveDown, Qt::NoModifier);
    QModelIndex inx = m_canvasGridView->moveCursorGrid(QAbstractItemView::CursorAction::MoveDown, Qt::NoModifier);
    EXPECT_EQ(ret, inx);
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
        stub_ext::StubExt stu;
        bool judge = false;
        stu.set_lamda(ADDR(DFileService, openFiles), [&judge](){judge = true; return false;});
        m_canvasGridView->openUrls(list);
        const QModelIndex &index = m_canvasGridView->model()->index(list[0]);
        QRect rect = m_canvasGridView->rectForIndex(index);
        EXPECT_EQ(rect.width(), m_canvasGridView->d->cellWidth);
    }
    QString temp = m_canvasGridView->canvansScreenName();
    EXPECT_EQ(temp, m_canvasGridView->m_screenName);
}

static bool increase = false;
static bool decrease = false;
static bool keyctrlispressed()
{
    return true;
}

static void increaseFunc()
{
    increase = true;
}

static void decreaseFunc()
{
    decrease = true;
}

TEST_F(CanvasGridViewTest, test_wheelEvent)
{
    Stub stu;
    stu.set(ADDR(DFMGlobal,keyCtrlIsPressed), keyctrlispressed);
    stu.set(ADDR(CanvasGridView,increaseIcon), increaseFunc);
    stu.set(ADDR(CanvasGridView,decreaseIcon), decreaseFunc);

    QWheelEvent event(QPointF(), 1, Qt::LeftButton, Qt::NoModifier);
    m_canvasGridView->wheelEvent(&event);
    EXPECT_TRUE(increase);
    EXPECT_FALSE(decrease);

    increase = false;
    QWheelEvent event1(QPointF(), -1, Qt::LeftButton, Qt::NoModifier);
    m_canvasGridView->wheelEvent(&event1);
    EXPECT_TRUE(decrease);
    EXPECT_FALSE(increase);
}

TEST_F(CanvasGridViewTest, test_mouseDoubleClickEvent)
{
    QMouseEvent event(QEvent::MouseButtonDblClick,{0,0},Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    m_canvasGridView->mouseDoubleClickEvent(&event);
}

TEST_F(CanvasGridViewTest, test_setscreenname)
{
    QString string = m_canvasGridView->m_screenName;
    m_canvasGridView->setScreenName("test");

    EXPECT_NE(string, m_canvasGridView->m_screenName);
    m_canvasGridView->setScreenName(string);
}

TEST_F(CanvasGridViewTest, test_increaseIcon)
{
    m_canvasGridView->itemDelegate()->setIconSizeByIconSizeLevel(4);
    m_canvasGridView->increaseIcon();
    m_canvasGridView->itemDelegate()->setIconSizeByIconSizeLevel(2);
    m_canvasGridView->increaseIcon();
    int level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_EQ(3, level);
}

TEST_F(CanvasGridViewTest, test_decreaseIcon)
{
    m_canvasGridView->itemDelegate()->setIconSizeByIconSizeLevel(2);
    m_canvasGridView->decreaseIcon();
    int level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_EQ(1, level);
}
// test keyboardSearch
static int bjudge = false;
TEST_F(CanvasGridViewTest, test_keyboardSearch)
{
    void (*keysearch)(char) = [](char tmp){
        bjudge = true;
    };
    EXPECT_EQ(bjudge, false);
    Stub stu;
    stu.set(ADDR(DFileViewHelper, keyboardSearch), keysearch);
    m_canvasGridView->keyboardSearch("test");
    EXPECT_TRUE(bjudge);
}

//test Size
static QSize size;
static bool judsize = false;

TEST_F(CanvasGridViewTest, test_size)
{
    QSize (*mysize)() = [](){
        judsize = true;
        return size;
    };
    Stub stu;
    stu.set(ADDR(GridManager, gridSize), mysize);
    m_canvasGridView->Size();
    EXPECT_TRUE(judsize);
}

TEST_F(CanvasGridViewTest, test_dragLeaveEvent)
{
    QDragLeaveEvent event;
    m_canvasGridView->dragLeaveEvent(&event);
    EXPECT_FALSE(m_canvasGridView->d->startDodge);
    EXPECT_TRUE(!m_canvasGridView->d->dodgeDelayTimer.isActive());
}

TEST_F(CanvasGridViewTest, test_updateEntryExpandedState)
{
    DUrl url("file:///");
    m_canvasGridView->updateEntryExpandedState(url);
    DUrl ret = m_canvasGridView->currentUrl();
    EXPECT_EQ(url, ret);
}

//Stub
static bool test_dump = false;

TEST_F(CanvasGridViewTest, test_dump)
{
    void (*mydump)() = [](){
        test_dump = true;
        return;
    };
    Stub tub;
    tub.set(ADDR(GridManager, dump), mydump);
    QString string = m_canvasGridView->Dump();
    EXPECT_EQ(string, "");
    EXPECT_TRUE(test_dump);
}

TEST_F(CanvasGridViewTest, test_DumpPos)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    path = path + '/' + "test.txt";
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }

    QString temp = m_canvasGridView->DumpPos(0, 0);
    EXPECT_TRUE(!temp.isEmpty());
    m_canvasGridView->Refresh();
}

TEST_F(CanvasGridViewTest, test_currentCursorFile)
{
    QCursor cur;
    cur.setPos(10, 10);
    m_canvasGridView->currentCursorFile();
}

TEST_F(CanvasGridViewTest, test_itemIconGeomerty)
{
    QModelIndex index = m_canvasGridView->firstIndex();
    if (index.isValid()) {
        QRect rect = m_canvasGridView->itemIconGeomerty(index);
        EXPECT_NE(QRect(), rect);
    }
}

TEST_F(CanvasGridViewTest, test_fakeDropEvent)
{
    m_canvasGridView->d->dodgeDelayTimer.start(100);
    m_canvasGridView->fakeDropEvent();
    EXPECT_FALSE(m_canvasGridView->d->startDodge);
    EXPECT_TRUE(!m_canvasGridView->d->dodgeDelayTimer.isActive());
}

TEST_F(CanvasGridViewTest, test_dragEnterEvent)
{
    QPoint pos(10, 10);
    QMimeData data;
    QDragEnterEvent event(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    m_canvasGridView->dragEnterEvent(&event);
    data.setData("XdndDirectSave0", "XdndDirectSave0");
    m_canvasGridView->dragEnterEvent(&event);

    stub_ext::StubExt stu;
    bool ischeck = false;
    stu.set_lamda(ADDR(DFileDragClient, checkMimeData), [&ischeck](){ischeck = true; return true;});
    m_canvasGridView->dragEnterEvent(&event);
    EXPECT_TRUE(ischeck);
}

TEST_F(CanvasGridViewTest, test_openUrl)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    path = path + '/' + "test.txt";
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }
    DUrlList ulist = m_canvasGridView->selectedUrls();
    DUrl url(DUrl::fromLocalFile(path));
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DFileService, openFile), [&judge](){judge = true; return true;});
    m_canvasGridView->openUrl(url);
    EXPECT_TRUE(judge);
}

TEST_F(CanvasGridViewTest, test_screenName)
{
    QString name = m_canvasGridView->screenName();
    EXPECT_EQ(name, m_canvasGridView->m_screenName);
}

TEST_F(CanvasGridViewTest, test_dragMoveEvent)
{
    QMimeData data;
    QPoint pos(100, 100);
    QDragMoveEvent event(pos, Qt::MoveAction, &data, Qt::LeftButton, Qt::NoModifier);
    m_canvasGridView->dragMoveEvent(&event);
}

//stub
QWidget* mywidget = nullptr;
static bool judge = false;
QRect myviewport()
{
    QRect rect;
    if (!judge) {
        rect.setTop(10000);
        rect.setBottom(10000);
        rect.setLeft(10000);
        rect.setRight(10000);
    } else {
        rect.setTop(-1);
        rect.setBottom(-1);
        rect.setLeft(-1);
        rect.setRight(-1);
    }
    judge = true;
    return rect;
}

TEST_F(CanvasGridViewTest, test_scrollTo)
{
    Stub stu;
    stu.set(ADDR(QWidget, rect), myviewport);
    QModelIndex index = m_canvasGridView->firstIndex();
    m_canvasGridView->scrollTo(index);
    m_canvasGridView->scrollTo(index);
    EXPECT_TRUE(judge);
    delete mywidget;
}

TEST_F(CanvasGridViewTest, test_indexAt)
{
    static QModelIndex index = m_canvasGridView->firstIndex();
    QRect rect = m_canvasGridView->visualRect(index);
    QPoint pos(rect.x() + 10, rect.y() + 10);
    static QModelIndexList mlist;
    static QWidget* widget = nullptr;
    mlist << index;
    QString myf = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    myf += '/' + "new.txt";
    QFile file(myf);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }
    QModelIndexList(*mylist)() = [](){
        return mlist;
    };
    QModelIndex(*myindex)() = [](){
        return index;
    };
    bool(*judge)() = [](){
        return true;
    };
    QWidget*(*mywidget)() = [](){
        widget = new QWidget;
        widget->setGeometry(0, 0, 1600, 1000);
        return widget;
    };
    bool(*visible)() = [](){
        return true;
    };
    QRect(*myrect)() = [](){
        QRect rect(QPoint(0,0), QPoint(1000, 800));
        return rect;
    };
    Stub tub, tub1;
    typedef int (*fplist)(DIconItemDelegate*, QModelIndexList);
    fplist list = (fplist)(&DIconItemDelegate::hasWidgetIndexs);

    typedef int (*fpwidget)(CanvasGridView*, QWidget*);
    fpwidget mwidget = (fpwidget)(&CanvasGridView::indexWidget);
    tub.set(list, mylist);
    tub.set(mwidget, mywidget);
    tub.set(ADDR(QWidget, isVisible), visible);
    m_canvasGridView->indexAt(pos);

    tub1.set(ADDR(DIconItemDelegate, editingIndex), myindex);
    QModelIndex ret = m_canvasGridView->indexAt(pos);
    EXPECT_TRUE(index == index);
}

TEST_F(CanvasGridViewTest, test_dropEvent)
{
    QMimeData data;
    QDropEvent event(QPointF(), Qt::DropAction::MoveAction, &data, Qt::LeftButton, Qt::NoModifier);
    stub_ext::StubExt tub;
    static QModelIndexList indexlist;
    indexlist << m_canvasGridView->firstIndex();
    tub.set_lamda(ADDR(DFileSelectionModel, selectedIndexes), [](){return indexlist;});
    tub.set_lamda(ADDR(QModelIndex, isValid), [](){return true;});
    tub.set_lamda(ADDR(QModelIndex, isValid), [](){return false;});
    m_canvasGridView->dropEvent(&event);


    static DUrl url = DesktopFileInfo::homeDesktopFileUrl();
    tub.set_lamda(VADDR(DAbstractFileInfo, fileUrl), [](){return url;});
    tub.set_lamda(ADDR(QModelIndex, isValid), [](){return true;});
    m_canvasGridView->m_urlsForDragEvent << url;
    m_canvasGridView->dropEvent(&event);
    EXPECT_EQ(event.dropAction(), Qt::IgnoreAction);
}


