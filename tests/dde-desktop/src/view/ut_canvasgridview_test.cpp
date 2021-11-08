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
#include <QMouseEvent>
#include <QDrag>
#include <QPainter>

#include <dfilemenu.h>
#include <DFileDragClient>
#include "../stub-ext/stubext.h"
#include "../dde-file-manager-lib/testhelper.h"

#define private public
#define protected public

#include "screen/screenhelper.h"
#include <view/canvasviewmanager.h>
#include <view/backgroundmanager.h>
#include <view/canvasgridview.h>
#include <view/private/canvasviewprivate.h>
#include <presenter/gridmanager.h>
#include <dfilesystemmodel.h>
#include "../model/dfileselectionmodel.h"
#include "../dde-desktop/presenter/gridmanager.h"
#include <private/canvasviewprivate.h>
#include "view/desktopitemdelegate.h"
#include "../dde-desktop/desktop.h"
#include "../dde-desktop/desktop.cpp"
#include "dfileviewhelper.h"
#include "addr_pri.h"
#include "stub.h"
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
        //以防桌面没文件
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        m_cvmgr.reset(new CanvasViewManager(new BackgroundManager()));
        for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
            if(1 == tpCanvas->screenNum()){
                m_canvasGridView = tpCanvas.data();
            }
            tpCanvas->hide();
        }
    }

    virtual void SetUp() override{
        m_canvasGridView->clearSelection();
    }

    virtual void TearDown()override{

    }
public:
    QScopedPointer<CanvasViewManager> m_cvmgr;
    CanvasGridView *m_canvasGridView{nullptr};
    static QString tstFile;
    static QString tstDir;
    QString path;
};

QString CanvasGridViewTest::tstFile = "testutxxxxxxxxxxx.txt";
QString CanvasGridViewTest::tstDir = "test-dir-utxxxxxxxxxx";

namespace {
void waitDataEvent(CanvasGridView *view)
{
    if (!view)
        return;

    auto model = view->model();
    if (!model)
        return;

    QEventLoop loop;
    QObject::connect(model, &DFileSystemModel::sigJobFinished, &loop,&QEventLoop::quit,Qt::QueuedConnection);
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    view->delayCustom(0);
}
}

#define waitData(view) waitDataEvent(view); \
ASSERT_EQ(m_canvasGridView->model()->rowCount(), GridManager::instance()->allItems().size());

TEST(CanvasGridViewTest_begin, beginTest)
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QFile file(path + '/' + CanvasGridViewTest::tstFile);
    file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
    file.write("test");
    file.close();

    QDir dir;
    dir.mkpath(path + '/' + CanvasGridViewTest::tstDir);

    ASSERT_TRUE(QFileInfo::exists(path + '/' + CanvasGridViewTest::tstFile));
    ASSERT_TRUE(QFileInfo::exists(path + '/' + CanvasGridViewTest::tstDir));
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

    QMouseEvent mv(QEvent::MouseMove,QPointF(0,0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    m_canvasGridView->mouseMoveEvent(&mv);

    m_canvasGridView->d->showSelectRect = true;
    QMouseEvent mv2(QEvent::MouseMove,QPointF(500,600), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_canvasGridView->mouseMoveEvent(&mv2);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_paintEvent)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    waitData(m_canvasGridView);

    CanvasViewPrivate *vp = m_canvasGridView->d.data();
    vp->dodgeAnimationing = true;
    vp->_debug_show_grid = true;

    vp->dodgeItems.append(path + '/' + CanvasGridViewTest::tstFile);
    vp->dodgeTargetGrid = GridManager::instance()->core();

    QPaintEvent event(QRect(0,0,200,200));
    m_canvasGridView->paintEvent(&event);

    delete vp->dodgeTargetGrid;
    vp->dodgeTargetGrid = nullptr;
}

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

    QMouseEvent me(QEvent::User, QPoint(),
                   Qt::LeftButton,
                   Qt::LeftButton,
                   Qt::KeyboardModifiers());

    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->d->selectRect = QRect(QPoint(0, 0), QPoint(100, 100));

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(QModelIndex, isValid), [](){return true;});
    stu.set_lamda(ADDR(DFMGlobal, keyCtrlIsPressed), [](){return true;});
    stu.set_lamda(ADDR(CanvasGridView, isSelected), [](){return true;});
    m_canvasGridView->mouseReleaseEvent(&me);
    qApp->processEvents();
    waitData(m_canvasGridView);
    EXPECT_EQ(m_canvasGridView->d->selectRect, QRect());
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_contextMenuEvent_empty)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    qApp->processEvents();
    waitData(m_canvasGridView);

    stub_ext::StubExt st;
    st.set_lamda(VADDR(CanvasGridView,indexAt),[](){return QModelIndex();});
    st.set_lamda(ADDR(DFileViewHelper,isEmptyArea),[](){return true;});

    int menu = 0;
    st.set_lamda(ADDR(CanvasGridView,showEmptyAreaMenu),[&menu](){menu = 1;return;});
    st.set_lamda(ADDR(CanvasGridView,showNormalMenu),[&menu](){menu = 2;return;});

    //根据Desktop文件夹下文件以及桌面排序情况可能会触发两种右键菜单的某一种
    QContextMenuEvent ent(QContextMenuEvent::Reason::Mouse,QPoint(10, 10),
                          m_canvasGridView->mapToGlobal(QPoint(100, 100)),Qt::KeyboardModifiers());
    m_canvasGridView->contextMenuEvent(&ent); //内部触发update
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

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Up)
{
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

    QModelIndexList list;
    list << m_canvasGridView->firstIndex()
          << QModelIndex()
          << m_canvasGridView->model()->index(DUrl::fromLocalFile(path));

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DFileSelectionModel, selectedIndexes), [list](){return list;});
    m_canvasGridView->keyPressEvent(&keyPressEvt_Up);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Tab)
{
    //Tab
    QKeyEvent keyPressEvt_Tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Tab);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_ESC)
{
    //ESC
    QKeyEvent keyPressEvt_Esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QString str = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    QList<QUrl> lurl;
    lurl << QUrl::fromLocalFile(str);
    bool isfetch = false;

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DFMGlobal, fetchUrlsFromClipboard), [lurl, &isfetch](){isfetch = true; return lurl;});
    stu.set_lamda(VADDR(DAbstractFileInfo, path), [str](){return str;});
    m_canvasGridView->keyPressEvent(&keyPressEvt_Esc);
    EXPECT_TRUE(isfetch);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Enter)
{
    //Enter
    QKeyEvent keyPressEvt_Enter(QEvent::KeyPress, Qt::Key_Enter, Qt::KeypadModifier);
    //为了避免打开太多文件,对openfiles进行函数转换
    stub_ext::StubExt tub;
    tub.set_lamda(ADDR(DFileService, openFiles), [](){return true;});
    tub.set_lamda(VADDR(DAbstractFileInfo, isVirtualEntry), [](){return true;});
    m_canvasGridView->keyPressEvent(&keyPressEvt_Enter);

    bool myValid = false;
    tub.set_lamda(VADDR(QModelIndex, isValid), [&myValid](){myValid = true; return true;});
    m_canvasGridView->keyPressEvent(&keyPressEvt_Enter);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_F5)
{
    //F5
    QKeyEvent keyPressEvt_padF5(QEvent::KeyPress, Qt::Key_F5, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_padF5);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Delete)
{
    //Delete
    QKeyEvent keyPressEvt_Key_padDelete(QEvent::KeyPress, Qt::Key_Delete, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_padDelete);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Space)
{
    stub_ext::StubExt tub;
    QKeyEvent keyPressEvt_Key_Space(QEvent::KeyPress, Qt::Key_Space, Qt::KeypadModifier);
    static bool spacejudge = false;
    void(*myspace)() = [](){spacejudge = true;};
    tub.set(ADDR(DFMGlobal, showFilePreviewDialog), myspace);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Space);
    EXPECT_TRUE(spacejudge);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_KeyD)
{
    QKeyEvent keyPressEvt_Key_D(QEvent::KeyPress, Qt::Key_D, Qt::KeypadModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_D);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_ShiftDelete)
{
    QKeyEvent keyPressEvt_Key_ShiftDelete(QEvent::KeyPress, Qt::Key_Delete, Qt::ShiftModifier);
    QString depath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    depath = depath + '/' + "test.txt";
    QFile file(depath);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }

    QModelIndexList list;
    list << m_canvasGridView->model()->index(DUrl::fromLocalFile(depath));

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(DFileSelectionModel, selectedIndexes), [list](){return list;});
    stub.set_lamda(ADDR(DFileService, deleteFiles), [](){return true;});
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_ShiftDelete);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_ShiftT)
{
    //T
    QKeyEvent keyPressEvt_Key_ShiftT(QEvent::KeyPress, Qt::Key_T, Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_ShiftT);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_CrtlH)
{
    //Key_H
    QKeyEvent keyPressEvt_Key_CrtlH(QEvent::KeyPress, Qt::Key_H, Qt::ControlModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_CrtlH);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Minus)
{
    //Ctrl + -
    QKeyEvent keyPressEvt_Key_Minus(QEvent::KeyPress, Qt::Key_Minus, Qt::ControlModifier);
    m_canvasGridView->setIconByLevel(3);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Minus);
    int level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_TRUE(2 == level);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Equal)
{
    //ctrl + =
    QKeyEvent keyPressEvt_Key_Equal(QEvent::KeyPress, Qt::Key_Equal, Qt::ControlModifier);
    m_canvasGridView->setIconByLevel(2);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Equal);
    int level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_TRUE(3 == level);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Plus)
{
    //ctrl + shift + +
    QKeyEvent keyPressEvt_Key_Plus(QEvent::KeyPress, Qt::Key_Plus, Qt::ControlModifier | Qt::ShiftModifier);
    m_canvasGridView->setIconByLevel(3);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Plus);
    int level = m_canvasGridView->itemDelegate()->iconSizeLevel();
    EXPECT_TRUE(4 == level);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_KeyI)
{
    //Key_I
    QKeyEvent keyPressEvt_Key_I(QEvent::KeyPress, Qt::Key_I, Qt::ControlModifier);
    void(*dig)() = [](){return;};
    stub_ext::StubExt stub;
    stub.set(ADDR(DFMGlobal, showPropertyDialog), dig);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_I);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Key7)
{//Key_7
    QKeyEvent keyPressEvt_Key_7(QEvent::KeyPress, Qt::Key_7, Qt::ControlModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_7);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_KeyN)
{
    //Qt::Key_N
    QKeyEvent keyPressEvt_Key_N(QEvent::KeyPress, Qt::Key_N, Qt::ControlModifier | Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_N);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Question)
{
    //Qt::Key_Question
    QKeyEvent keyPressEvt_Key_Question(QEvent::KeyPress, Qt::Key_Question, Qt::ControlModifier | Qt::ShiftModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_Question);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_AltM)
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

    stu.reset(&QVariant::isValid);
    stu.set_lamda(ADDR(QVariant, isValid), [](){return false;});
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_AltM);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_KeyR)
{
    QKeyEvent keyPressEvt_Key_R(QEvent::KeyPress, Qt::Key_R, Qt::AltModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_R);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Up2)
{
    stub_ext::StubExt stu;
    QKeyEvent keyPressEvt_Key(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    stu.set_lamda(ADDR(QVariant, toBool), [](){return true;});
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_keyPressEvent_Key8)
{
    QKeyEvent keyPressEvt_Key_8(QEvent::KeyPress, Qt::Key_8, Qt::NoModifier);
    m_canvasGridView->keyPressEvent(&keyPressEvt_Key_8);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_SelectAll)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_moveCursorGrid)
{
    ASSERT_NE(m_canvasGridView, nullptr);

    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    path = path + '/' + "test.txt";
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::NewOnly);
        file.close();
    }

    QAbstractItemView::CursorAction  tpCursorActon;
    Qt::KeyboardModifiers tpModifiers;
    tpModifiers = Qt::NoModifier;

    QList<DUrl> lst;
    QString urlPath = GridManager::instance()->firstItemId(m_canvasGridView->m_screenNum);
    lst << DUrl(urlPath);
    m_canvasGridView->select(lst);
    waitData(m_canvasGridView);

    tpCursorActon = QAbstractItemView::CursorAction::MoveLeft;
    m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);

    auto current = m_canvasGridView->d->currentCursorIndex;
    if (!current.isValid()) {
        current = m_canvasGridView->firstIndex();
        m_canvasGridView->d->currentCursorIndex = current;
    }

    //主屏第一个index,如果主屏没有图标index为无效，反之为第一个图标index
    auto firstIndex = m_canvasGridView->firstIndex();
    ASSERT_TRUE(firstIndex.isValid()) << "primary screen no valid index";

    bool leftIndex = false;
    bool rightIndex = false;
    bool upIndex = false;
    bool downIndex = false;

    m_canvasGridView->d->currentCursorIndex = firstIndex;
    tpCursorActon = QAbstractItemView::CursorAction::MoveLeft;
    auto targetLeft = m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    if(!targetLeft.isValid()) {
        auto expectLeft = m_canvasGridView->firstIndex() == targetLeft;
        EXPECT_TRUE(expectLeft);
    }else {
        leftIndex = true;
    }

    m_canvasGridView->d->currentCursorIndex = firstIndex;
    tpCursorActon = QAbstractItemView::CursorAction::MoveRight;
    auto targetRight = m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    if (!targetRight.isValid()) {
        auto expectRight = m_canvasGridView->firstIndex() == targetRight;
        EXPECT_TRUE(expectRight);
    }else {
        rightIndex = true;
    }


    m_canvasGridView->d->currentCursorIndex = m_canvasGridView->firstIndex();
    tpCursorActon = QAbstractItemView::CursorAction::MoveUp;
    auto targetUp = m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    if (!targetUp.isValid()) {
        auto expectUp = m_canvasGridView->firstIndex() == targetUp;
        EXPECT_TRUE(expectUp);
    }else {
        upIndex = true;
    }

    m_canvasGridView->d->currentCursorIndex = firstIndex;
    tpCursorActon = QAbstractItemView::CursorAction::MoveDown;
    auto targetDown = m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    if (!targetDown.isValid()) {
        auto expectDown = m_canvasGridView->firstIndex() == targetDown;
        EXPECT_TRUE(expectDown);
    }else {
        downIndex = true;
    }

    auto url = m_canvasGridView->model()->getUrlByIndex(current);
    auto pos = GridManager::instance()->position(1, url.toString());
    auto newCoord = Coordinate(pos);
    auto row = m_canvasGridView->d->rowCount;
    auto colCount = m_canvasGridView->d->colCount;
    int conut = 1;
    bool inThere = false;
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(QModelIndex, isValid), []{
        return true;});

    if(leftIndex) {
        stub.set_lamda(ADDR(Coordinate, moveLeft), [&inThere, &newCoord]{
            inThere = true;
            return Coordinate(newCoord.d.x - 1, newCoord.d.y);
        });
        stub.set_lamda(ADDR(Coordinate, position), [&colCount, &conut]{
            conut++;
            if (conut >= 3)
                return QPoint(colCount, colCount);
            return QPoint(0, 1);
        });
        m_canvasGridView->select({url});
        tpCursorActon = QAbstractItemView::CursorAction::MoveLeft;
        m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
        EXPECT_TRUE(inThere);
    }

    inThere = false;
    if(rightIndex) {
        stub.set_lamda(ADDR(Coordinate, moveRight), [&inThere, &newCoord]{
            inThere = true;
            return Coordinate(newCoord.d.x + 1, newCoord.d.y);
        });
        conut = 1;
        stub.reset(&Coordinate::position);
        stub.set_lamda(ADDR(Coordinate, position), [&colCount, &conut]{
            conut++;
            if (conut >= 3)
                return QPoint(colCount, colCount);
            return QPoint(0, 1);
        });
        tpCursorActon = QAbstractItemView::CursorAction::MoveRight;
        m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
        EXPECT_TRUE(inThere);
    }

    inThere = false;
    stub.set_lamda(ADDR(Coordinate, moveUp), [&inThere, &newCoord]{
        inThere = true;
        return Coordinate(newCoord.d.x, newCoord.d.y - 1);
    });
    if(upIndex) {
        tpCursorActon = QAbstractItemView::CursorAction::MoveUp;
        m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
        EXPECT_TRUE(inThere);
    }

    inThere = false;
    stub.set_lamda(ADDR(Coordinate, moveDown), [&inThere, &newCoord]{
        inThere = true;
        return Coordinate(newCoord.d.x, newCoord.d.y + 1);
    });
    if(downIndex) {
        tpCursorActon = QAbstractItemView::CursorAction::MoveDown;
        m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
        EXPECT_TRUE(inThere);
    }

    tpCursorActon = QAbstractItemView::CursorAction::MovePageUp;
    auto targetPageUp = m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    auto expectPageUp = m_canvasGridView->firstIndex() == targetPageUp;
    EXPECT_TRUE(expectPageUp);

    tpCursorActon = QAbstractItemView::CursorAction::MovePageDown;
    auto targetPageDown = m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    auto expectPageDown = m_canvasGridView->lastIndex() == targetPageDown;
    EXPECT_TRUE(expectPageDown);

    tpModifiers = Qt::ShiftModifier;
    inThere = false;
    tpCursorActon = QAbstractItemView::CursorAction::MovePageUp;
    stub.reset(&Coordinate::position);
    stub.set_lamda(ADDR(Coordinate, position), []{
        return QPoint(-1, -1);
    });
    auto targetPageUpShift = m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    auto expectPageUpShift = m_canvasGridView->firstIndex() == targetPageUpShift;
    EXPECT_TRUE(expectPageUpShift);
    EXPECT_TRUE(inThere);

    inThere = false;
    conut = 1;
    stub.reset(&Coordinate::moveDown);
    stub.set_lamda(ADDR(Coordinate, moveDown), [&inThere, &newCoord, &conut]{
        inThere = true;
        conut++;
        if (conut >= 3)
            return Coordinate(newCoord.d.x, newCoord.d.y + 50);
        return Coordinate(newCoord.d.x, newCoord.d.y + 1);
    });

    stub.reset(&Coordinate::position);
    stub.set_lamda(ADDR(Coordinate, position), [&row, &colCount, &conut]{
        conut++;
        if (conut >= 3)
            return QPoint(row, colCount);
        return QPoint(0, 1);
    });

    tpCursorActon = QAbstractItemView::CursorAction::MovePageDown;
    auto targetPageDownShift = m_canvasGridView->moveCursorGrid(tpCursorActon, tpModifiers);
    auto expectPageDownShift = m_canvasGridView->lastIndex() == targetPageDownShift;
    EXPECT_TRUE(expectPageDownShift);
    EXPECT_TRUE(inThere);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_dragMoveEvent)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    waitData(m_canvasGridView);

    //先设置选中
    QList<DUrl> lst;
    QString urlPath = "file://" + path + '/' + CanvasGridViewTest::tstFile;
    lst << DUrl(urlPath);
    m_canvasGridView->select(lst);
    waitData(m_canvasGridView);

    //模拟拖拽数据
    auto tgIndex = m_canvasGridView->model()->index(DUrl(urlPath));
    ASSERT_TRUE(tgIndex.isValid());
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
    delete tgData;
    tgData = nullptr;
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_dropEvent)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    waitData(m_canvasGridView);
    ASSERT_GT(m_canvasGridView->model()->rowCount(), 1);

    QString urlPath = "file://" + path + "/" + tstFile;
    DUrl urlFile(urlPath);
    DUrl urlDir("file://" + path + "/" + tstDir);

    //模拟拖拽数据
    auto tgIndex = m_canvasGridView->model()->index(urlFile);
    ASSERT_TRUE(tgIndex.isValid());

    QModelIndexList tpIndexes;
    tpIndexes.append(tgIndex);
    QMimeData *tgData = m_canvasGridView->model()->mimeData(tpIndexes);

    //回收站,计算机等
    {
        auto dropIndex = m_canvasGridView->model()->index(urlDir);
        ASSERT_TRUE(dropIndex.isValid());

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

    //内部拖动到无效区域
    if (m_canvasGridView->model()->rowCount() >= 2) {
        QModelIndex vaildIndex;
        for (int i = 0; i < m_canvasGridView->model()->rowCount(); ++i) {
            auto idx = m_canvasGridView->model()->index(i, 0);
            if (idx != tgIndex) {
                vaildIndex = idx;
                break;
            }
        }

        if (!vaildIndex.isValid())
            return;

        QDropEvent dropevent(QPoint(64, 200), Qt::MoveAction, tgData, Qt::MouseButton::LeftButton, Qt::NoModifier);
        StubExt stu;
        stu.set_lamda(&QDropEvent::source,[this](){return m_canvasGridView;});
        QPoint droppos = m_canvasGridView->gridRectAt(dropevent.pos()).center();
        stu.set_lamda(VADDR(CanvasGridView,indexAt),[=](CanvasGridView *,const QPoint &pos){
            if (pos == droppos)
                return vaildIndex;
            return QModelIndex();
        });
        stu.set_lamda(ADDR(DFileSelectionModel,selectedIndexes),[=](){return tpIndexes;});

        bool move = false;
        stu.set_lamda(ADDR(GridManager,sigSyncOperation),[&move](){move = false;});

        bool checkMimeData = false;
        stu.set_lamda(&DFileDragClient::checkMimeData,[&checkMimeData](){checkMimeData = true;return true;});

        m_canvasGridView->dropEvent(&dropevent);
        EXPECT_FALSE(move);
        EXPECT_FALSE(checkMimeData);
    }
    delete tgData;
    tgData = nullptr;
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
    if (localFile.isEmpty())
        return;

    auto tempIndex = m_canvasGridView->model()->index(DUrl(localFile));
    auto tempIndexCanvs = m_canvasGridView->firstIndex();
    EXPECT_EQ(tempIndex, tempIndexCanvs);
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

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setSelection_two){
    ASSERT_NE(m_canvasGridView, nullptr);

    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);
    if (desktopUrl.isEmpty())
        return;
    if(m_canvasGridView->setRootUrl(desktopUrl)){
        EXPECT_TRUE(desktopUrl == m_canvasGridView->model()->rootUrl());
    }

    //获取一个item
    auto itemUrls = GridManager::instance()->itemIds(m_canvasGridView->m_screenNum);
    auto tgIndex = m_canvasGridView->model()->index(DUrl(itemUrls.first()));
    if (!tgIndex.isValid())
        return;
    auto itemRect = m_canvasGridView->visualRect(tgIndex);
    m_canvasGridView->d->mousePressed = true;
    m_canvasGridView->d->showSelectRect = true;

    //模拟其左上角框选
    auto itemTopLeft = itemRect.topLeft();
    QRect topLeftRect(itemTopLeft, QPoint(itemTopLeft.x() + 5, itemTopLeft.y() + 5));
    auto topLeftBeforeSelects = m_canvasGridView->selectedUrls();

    m_canvasGridView->setSelection(topLeftRect, QItemSelectionModel::Select);
    auto topLeftAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValue = topLeftBeforeSelects == topLeftAfterSelects && 0 == topLeftBeforeSelects.size() && 0 == topLeftAfterSelects.size();
    EXPECT_TRUE(expectValue);


    QRect topleftRectTwo(itemTopLeft, QPoint(itemTopLeft.x() + 15, itemTopLeft.y() + 15));
    topLeftBeforeSelects = m_canvasGridView->selectedUrls();
    m_canvasGridView->setSelection(topleftRectTwo, QItemSelectionModel::Select);
    topLeftAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValueTwo = topLeftBeforeSelects != topLeftAfterSelects && 0 == topLeftBeforeSelects.size() && 1 == topLeftAfterSelects.size();
    EXPECT_TRUE(expectValueTwo);
    m_canvasGridView->selectionModel()->clearSelection();


    //模拟其右上角框选
    auto itemTopRight = itemRect.topRight();
    QRect topRightRect(QPoint(itemTopRight.x() - 5, itemTopRight.y()), QPoint(itemTopRight.x(), itemTopRight.y() + 5));
    auto topRightBeforeSelects = m_canvasGridView->selectedUrls();
    m_canvasGridView->setSelection(topRightRect, QItemSelectionModel::Select);
    auto topRightAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValueTr = topRightBeforeSelects == topRightAfterSelects && 0 == topRightBeforeSelects.size() && 0 == topRightAfterSelects.size();
    EXPECT_TRUE(expectValueTr);


    QRect topRightRectTwo(QPoint(itemTopRight.x() - 15, itemTopRight.y()), QPoint(itemTopRight.x(), itemTopRight.y() + 15));
    topRightBeforeSelects = m_canvasGridView->selectedUrls();
    m_canvasGridView->setSelection(topRightRectTwo, QItemSelectionModel::Select);
    topRightAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValueTrTwo = topRightBeforeSelects != topRightAfterSelects && 0 == topRightBeforeSelects.size() && 1 == topRightAfterSelects.size();
    EXPECT_TRUE(expectValueTrTwo);
    m_canvasGridView->selectionModel()->clearSelection();

    //模拟其右下角框选
    auto itemBottomRight = itemRect.bottomRight();
    QRect bottomRightRect(QPoint(itemBottomRight.x() - 5, itemBottomRight.y() - 5), itemBottomRight);
    auto bottomRightBeforeSelects = m_canvasGridView->selectedUrls();
    m_canvasGridView->d->mousePressed = true;
    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->setSelection(bottomRightRect, QItemSelectionModel::Select);
    auto bottomRightAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValueBr = bottomRightBeforeSelects == bottomRightAfterSelects && 0 == bottomRightBeforeSelects.size() && 0 == bottomRightAfterSelects.size();
    EXPECT_TRUE(expectValueBr);


    QRect bottomRightRectTwo(QPoint(itemBottomRight.x() - 15, itemBottomRight.y() - 15), itemBottomRight);
    bottomRightBeforeSelects = m_canvasGridView->selectedUrls();
    m_canvasGridView->d->mousePressed = true;
    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->setSelection(bottomRightRectTwo, QItemSelectionModel::Select);
    bottomRightAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValueBrTwo = bottomRightBeforeSelects != bottomRightAfterSelects && 0 == bottomRightBeforeSelects.size() && 1 == bottomRightAfterSelects.size();
    EXPECT_TRUE(expectValueBrTwo);
    m_canvasGridView->selectionModel()->clearSelection();

    //模拟其左下角框选
    auto itemBottomLeft = itemRect.bottomLeft();
    QRect bottomLeftRect(QPoint(itemBottomLeft.x(), itemBottomLeft.y() - 5), QPoint(itemBottomLeft.x() + 5, itemBottomLeft.y()));
    auto bottomLeftBeforeSelects = m_canvasGridView->selectedUrls();
    m_canvasGridView->d->mousePressed = true;
    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->setSelection(bottomLeftRect, QItemSelectionModel::Select);
    auto bottomLeftAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValueBl = bottomLeftBeforeSelects == bottomLeftAfterSelects && 0 == bottomLeftBeforeSelects.size() && 0 == bottomLeftAfterSelects.size();
    EXPECT_TRUE(expectValueBl);


    QRect bottomLeftRectTwo(QPoint(itemBottomLeft.x(), itemBottomLeft.y() - 15), QPoint(itemBottomLeft.x() + 15, itemBottomLeft.y()));
    bottomLeftBeforeSelects = m_canvasGridView->selectedUrls();
    m_canvasGridView->d->mousePressed = true;
    m_canvasGridView->d->showSelectRect = true;
    m_canvasGridView->setSelection(bottomLeftRectTwo, QItemSelectionModel::Select);
    bottomLeftAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValueBlTwo = bottomLeftBeforeSelects != bottomLeftAfterSelects && 0 == bottomLeftBeforeSelects.size() && 1 == bottomLeftAfterSelects.size();
    EXPECT_TRUE(expectValueBlTwo);
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedIndexCount){
    ASSERT_NE(m_canvasGridView, nullptr);

    TestHelper::runInLoop([](){}, 200);
    m_canvasGridView->selectAll();
    auto tempCnt1 = m_canvasGridView->selectionModel()->selectedIndexes().size();
    auto tempCnt2= GridManager::instance()->allItems().size();
    auto expectValue = tempCnt1 == tempCnt2;
    EXPECT_TRUE(expectValue);
}

TEST_F(CanvasGridViewTest, Test_CanvasGridViewTest_selectedUrls){
    ASSERT_NE(m_canvasGridView, nullptr);
    waitData(m_canvasGridView);
    ASSERT_GT(m_canvasGridView->model()->rowCount(), 1);

    m_canvasGridView->selectionModel()->clearSelection();
    auto count1 = m_canvasGridView->selectionModel()->selectedCount();
    EXPECT_EQ(count1, 0);
    m_canvasGridView->selectAll();
    auto count2 = m_canvasGridView->selectedUrls().size();
    if(GridManager::instance()->allItems().size() != 0)
        EXPECT_NE(count2, count1);
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
    ASSERT_FALSE(GridManager::instance()->autoMerge());

    stub_ext::StubExt st;
    bool ok = false;
    st.set_lamda(ADDR(GridManager,initArrage),[](){});
    st.set_lamda(ADDR(GridManager,delaySyncAllProfile),[&ok](){ok = true;});

    //立即保存
    m_canvasGridView->delayArrage(0);
    EXPECT_TRUE(ok);

    //延迟保存
    ok = false;
    m_canvasGridView->delayArrage(1);
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_TRUE(ok);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_delayAutoMerge)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    waitData(m_canvasGridView);
    EXPECT_GT(m_canvasGridView->model()->rowCount(), 0);

    stub_ext::StubExt st;
    bool ok = false;
    st.set_lamda(ADDR(GridManager,initArrage),[&ok](){ok = true;});

    //return
    GridManager::instance()->setAutoMerge(false);
    m_canvasGridView->delayAutoMerge(0);
    EXPECT_FALSE(ok);
    ok = false;

    GridManager::instance()->setAutoMerge(true);
    ASSERT_TRUE(GridManager::instance()->autoMerge());

    m_canvasGridView->delayAutoMerge(0);
    EXPECT_TRUE(ok);

    ok = false;
    m_canvasGridView->delayAutoMerge(1);
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_TRUE(ok);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_delayAutoMerge_refresh)
{
    ASSERT_NE(m_canvasGridView, nullptr);
    waitData(m_canvasGridView);

    m_canvasGridView->setAutoMerge(true);
    ASSERT_TRUE(GridManager::instance()->autoMerge());
    ASSERT_EQ(m_canvasGridView->m_refreshTimer, nullptr);

    stub_ext::StubExt st;
    bool ok = false;
    st.set_lamda(ADDR(DFileSystemModel, refresh), [&ok](){ok = true;});

    m_canvasGridView->m_refreshTimer = new QTimer();
    m_canvasGridView->delayModelRefresh(0);
    EXPECT_EQ(m_canvasGridView->m_refreshTimer, nullptr);
    EXPECT_TRUE(ok);

    ok = false;
    ASSERT_EQ(m_canvasGridView->m_refreshTimer, nullptr);
    m_canvasGridView->delayModelRefresh(1);
    EXPECT_NE(m_canvasGridView->m_refreshTimer, nullptr);
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_TRUE(ok);
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

    delete model;
    model = nullptr;
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
    DUrlList validSel;
    QModelIndexList validIndexes;
    m_canvasGridView->viewSelectedUrls(validSel, validIndexes);

    if (validIndexes.count() > 1) {
        auto execFoo = (Qt::DropAction(QDrag::*)(Qt::DropActions, Qt::DropAction))ADDR(QDrag, exec);
        stu.set_lamda(execFoo, [&judge](){
            judge = true;
            return Qt::DropAction::MoveAction;
        });
        stu.set_lamda(VADDR(QAbstractItemView, startDrag), [&judge](){judge = false; return;});
    } else {
        auto execFoo = (Qt::DropAction(QDrag::*)(Qt::DropActions, Qt::DropAction))ADDR(QDrag, exec);
        stu.set_lamda(execFoo, [&judge](){
            judge = false;
            return Qt::DropAction::MoveAction;
        });

        stu.set_lamda(VADDR(QAbstractItemView, startDrag), [&judge](){judge = true; return;});
    }

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
    waitData(m_canvasGridView);
    GridManager::instance()->setAutoMerge(false);
    ASSERT_FALSE(GridManager::instance()->autoMerge());

    auto initCustom = (void (GridManager::*)(QStringList &))&GridManager::initCustom;
    {
        stub_ext::StubExt st;
        bool ok = true;
        st.set_lamda(ADDR(GridManager,autoArrange), [](){return false;});
        st.set_lamda(initCustom, [](){});
        st.set_lamda(ADDR(CanvasGridView, delayArrage), [&ok](){ok = false;});
        m_canvasGridView->delayCustom(0);
        EXPECT_TRUE(ok);
    }

    stub_ext::StubExt st;
    bool ok = false;
    st.set_lamda(ADDR(GridManager,autoArrange), [](){return true;});
    st.set_lamda(initCustom, [&ok](){ok = true;});
    st.set_lamda(ADDR(CanvasGridView, delayArrage), [](){});

    m_canvasGridView->delayCustom(0);
    EXPECT_TRUE(ok);

    ok = false;
    m_canvasGridView->delayCustom(1);
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_TRUE(ok);
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
    stub_ext::StubExt stub;
    typedef QList<QRect> (*fptr)(DIconItemDelegate*,const QStyleOptionViewItem &, const QModelIndex &, bool);
    fptr myPaintGeomertys = (fptr)&DIconItemDelegate::paintGeomertys;
    stub.set_lamda(myPaintGeomertys, []()->QList<QRect>{return {};});

    //    stub.set_lamda(ADDR(QRect, marginsRemoved), [](const QMargins &)->QRect{return QRect(10, 10, 10, 10);});//why ?
    Stub stu;
    QRect(*temppp)(const QMargins &) = [](const QMargins &)->QRect{
        QRect  aaa(10, 10, 10, 10);
        return aaa;
    };
    stu.set(ADDR(QRect, marginsRemoved), temppp);

    QRect rect = m_canvasGridView->itemIconGeomerty(index);
    EXPECT_TRUE(QRect(10, 10, 10, 10) == rect);

    stub.reset(myPaintGeomertys);
    stub.set_lamda(myPaintGeomertys, []()->QList<QRect>{return {QRect(9, 9, 9, 9)};});
    QRect rect2 = m_canvasGridView->itemIconGeomerty(index);
    EXPECT_TRUE(QRect(9, 9, 9, 9) == rect2);
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
    static QWidget* widget = new QWidget;
    widget->setGeometry(0, 0, 1600, 1000);
    mlist << index;
    QString myf = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    myf = myf + '/' + "new.txt";
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
        return widget;
    };
    bool(*visible)() = [](){
        return true;
    };
    QRect(*myrect)() = [](){
        QRect rect(QPoint(0,0), QPoint(1000, 800));
        return rect;
    };
    Stub tub;
    typedef int (*fplist)(DIconItemDelegate*, QModelIndexList);
    fplist list = (fplist)(&DIconItemDelegate::hasWidgetIndexs);

    typedef int (*fpwidget)(CanvasGridView*, QWidget*);
    fpwidget mwidget = (fpwidget)(&CanvasGridView::indexWidget);
    tub.set(list, mylist);
    tub.set(mwidget, mywidget);
    tub.set(ADDR(QWidget, isVisible), visible);
    m_canvasGridView->indexAt(pos);

    tub.set(ADDR(DIconItemDelegate, editingIndex), myindex);
    QModelIndex ret = m_canvasGridView->indexAt(pos);
    EXPECT_TRUE(index == index);

    //delete the test widget
    if (widget) {
        delete widget;
        widget = nullptr;
    }
}
#ifndef __arm__
TEST_F(CanvasGridViewTest, test_dropEvent)
{
    QMimeData data;
    QDropEvent event(QPointF(), Qt::DropAction::MoveAction, &data, Qt::LeftButton, Qt::NoModifier);
    stub_ext::StubExt stub;
    stub.set_lamda(&DFileDragClient::checkMimeData,[](){return true;});
    stub.set_lamda(VADDR(DFileSystemModel,dropMimeData),[](){return false;});

    QModelIndexList indexlist;
    indexlist << m_canvasGridView->firstIndex();
    stub.set_lamda(ADDR(DFileSelectionModel, selectedIndexes), [indexlist](){return indexlist;});
    bool bValid = false;
    stub.set_lamda(ADDR(QModelIndex, isValid), [&bValid](){return bValid;});
    m_canvasGridView->dropEvent(&event);

    DUrl url = DesktopFileInfo::homeDesktopFileUrl();
    stub.set_lamda(VADDR(DAbstractFileInfo, fileUrl), [url](){return url;});
    bValid = true;
    m_canvasGridView->m_urlsForDragEvent << url;
    m_canvasGridView->dropEvent(&event);
    EXPECT_EQ(event.dropAction(), Qt::IgnoreAction);
}

TEST_F(CanvasGridViewTest, test_EnableUIDebug)
{
    m_canvasGridView->EnableUIDebug(true);
    EXPECT_TRUE(m_canvasGridView->d->_debug_log);
    EXPECT_TRUE(m_canvasGridView->d->_debug_show_grid);

    m_canvasGridView->EnableUIDebug(false);
    EXPECT_FALSE(m_canvasGridView->d->_debug_log);
    EXPECT_FALSE(m_canvasGridView->d->_debug_show_grid);
}
#endif

TEST_F(CanvasGridViewTest, test_isIndexHidden)
{
    EXPECT_FALSE(m_canvasGridView->isIndexHidden(QModelIndex()));
}

TEST_F(CanvasGridViewTest, test_viewSelectedUrls)
{
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();

    auto selects = m_canvasGridView->selectionModel()->selectedIndexes();
    QModelIndexList vIndexes;

    for (auto index : selects) {

        auto info =  m_canvasGridView->model()->fileInfo(index);
        if (info && !info->isVirtualEntry() && GridManager::instance()->contains(m_canvasGridView->screenNum(), info->fileUrl().toString())) {
            vIndexes << index;
        }
    }
    DUrlList validSel;
    QModelIndexList validIndexes;
    m_canvasGridView->viewSelectedUrls(validSel, validIndexes);
    EXPECT_TRUE(validIndexes.size() == validSel.size());
}

TEST_F(CanvasGridViewTest, test_renderToPixmap)
{
    qApp->processEvents();
    m_canvasGridView->selectAll();
    qApp->processEvents();

    auto selects = m_canvasGridView->selectionModel()->selectedIndexes();
    auto pixMap = m_canvasGridView->renderToPixmap(selects);

    bool judge = false;
    stub_ext::StubExt stub;
    auto drawTextFoo = (void(QPainter::*)(const QRect &, int, const QString &, QRect *))ADDR(QPainter, drawText);
    stub.set_lamda(drawTextFoo, [&judge]() {
        judge = true;
    });

    m_canvasGridView->renderToPixmap(selects);
    EXPECT_TRUE(judge);
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setSelection_three){
    ASSERT_NE(m_canvasGridView, nullptr);

    stub_ext::StubExt stub;
    bool shift = false;
    stub.set_lamda(&DFMGlobal::keyShiftIsPressed, [&shift]() {
        return shift;
    });

    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);
    if (desktopUrl.isEmpty())
        return;
    if(m_canvasGridView->setRootUrl(desktopUrl)){
        EXPECT_TRUE(desktopUrl == m_canvasGridView->model()->rootUrl());
    }

    //获取一个item
    auto itemUrls = GridManager::instance()->itemIds(m_canvasGridView->m_screenNum);
    auto tgIndex = m_canvasGridView->model()->index(DUrl(itemUrls.first()));
    if (!tgIndex.isValid())
        return;
    auto itemRect = m_canvasGridView->visualRect(tgIndex);
    m_canvasGridView->d->mousePressed = true;
    m_canvasGridView->d->showSelectRect = true;

    //模拟其左上角框选
    auto itemTopLeft = itemRect.topLeft();
    QRect topLeftRect(itemTopLeft, QPoint(itemTopLeft.x() + 5, itemTopLeft.y() + 5));
    auto topLeftBeforeSelects = m_canvasGridView->selectedUrls();

    m_canvasGridView->setSelection(topLeftRect, QItemSelectionModel::Select);
    auto topLeftAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValue = topLeftBeforeSelects == topLeftAfterSelects && 0 == topLeftBeforeSelects.size() && 0 == topLeftAfterSelects.size();
    EXPECT_TRUE(expectValue);

    QRect topleftRectTwo(itemTopLeft, QPoint(itemTopLeft.x() + 15, itemTopLeft.y() + 15));
    topLeftBeforeSelects = m_canvasGridView->selectedUrls();
    m_canvasGridView->setSelection(topleftRectTwo, QItemSelectionModel::Select);
    topLeftAfterSelects = m_canvasGridView->selectedUrls();
    bool expectValueTwo = topLeftBeforeSelects != topLeftAfterSelects && 0 == topLeftBeforeSelects.size() && 1 == topLeftAfterSelects.size();
    EXPECT_TRUE(expectValueTwo);
    m_canvasGridView->selectionModel()->clearSelection();
    auto lastSelectww = m_canvasGridView->selectedUrls();
    //再次框选
    QRect lastRect = itemRect;
    lastRect.setLeft(itemRect.topLeft().x() - 3);

    m_canvasGridView->setSelection(lastRect, QItemSelectionModel::Select);
    auto lastSelect = m_canvasGridView->selectedUrls();
    EXPECT_TRUE(1 == lastSelect.size());
    //m_canvasGridView->selectionModel()->clearSelection();

    shift = true;
    m_canvasGridView->setSelection(lastRect, QItemSelectionModel::Select);
    lastSelect = m_canvasGridView->selectedUrls();
    EXPECT_TRUE(1 == lastSelect.size());
    m_canvasGridView->selectionModel()->clearSelection();
}

TEST_F(CanvasGridViewTest, CanvasGridViewTest_setSelection_shiftAndArrowKeys)
{
    ASSERT_NE(m_canvasGridView, nullptr);

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);
    if (desktopUrl.isEmpty())
        return;
    if(m_canvasGridView->setRootUrl(desktopUrl)){
        EXPECT_TRUE(desktopUrl == m_canvasGridView->model()->rootUrl());
    }

    auto coordSize = GridManager::instance()->gridSize(m_canvasGridView->screenNum());
    auto fileCnt = GridManager::instance()->itemIds(m_canvasGridView->screenNum());
    int testFileNum = coordSize.height() * 3;
    int addFileNum = 0;
    if (fileCnt.size() < testFileNum) {
        addFileNum = testFileNum - fileCnt.size();
        for (int i = 1; i <= addFileNum; ++i) {
            QFile tfile(QString("%1/%2%3").arg(desktopPath).arg("forutxxxxxxxxxxxx").arg(QString::number(i)));
            tfile.open(QIODevice::ReadWrite | QIODevice::NewOnly);
            tfile.close();
        }
    }
    waitData(m_canvasGridView);

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DFMGlobal, keyShiftIsPressed), [](){ return true;});

    //设置为自动排列
    stu.set_lamda(ADDR(GridManager, autoArrange), [](){ return true;});
    m_canvasGridView->model()->clear();
    m_canvasGridView->model()->refresh();
    GridManager::instance()->initGridItemsInfos();
    for(auto tpCanvas : m_cvmgr->m_canvasMap.values()){
        if(tpCanvas)
            tpCanvas->update();
    }
    waitData(m_canvasGridView);
    //清空选中
    m_canvasGridView->selectionModel()->clearSelection();

    auto fileCnt2 = GridManager::instance()->itemIds(m_canvasGridView->screenNum());
    auto url10 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 1, 0);
    QModelIndex index10 = m_canvasGridView->model()->index(DUrl(url10));
    auto url20 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 2, 0);
    QModelIndex index20 = m_canvasGridView->model()->index(DUrl(url20));
    auto url21 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 2, 1);
    QModelIndex index21 = m_canvasGridView->model()->index(DUrl(url21));
    auto url22 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 2, 2);
    QModelIndex index22 = m_canvasGridView->model()->index(DUrl(url22));
    auto url12 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 1, 2);
    QModelIndex index12 = m_canvasGridView->model()->index(DUrl(url12));
    auto url02 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 0, 2);
    QModelIndex index02 = m_canvasGridView->model()->index(DUrl(url02));
    auto url01 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 0, 1);
    QModelIndex index01 = m_canvasGridView->model()->index(DUrl(url01));
    auto url00 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 0, 0);
    QModelIndex index00 = m_canvasGridView->model()->index(DUrl(url00));

    auto url11 = GridManager::instance()->itemId(m_canvasGridView->screenNum(), 1, 1);
    QModelIndex index11 = m_canvasGridView->model()->index(DUrl(url11));

    //选中布局(1,1)图标
    m_canvasGridView->select({DUrl(url11)});
    m_canvasGridView->d->m_oldCursorIndex = index11;
    m_canvasGridView->d->currentCursorIndex = index11;

    //上
    m_canvasGridView->d->currentCursorIndex = index10;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    auto temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(3 <= temp);

    //右
    m_canvasGridView->d->currentCursorIndex = index20;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(3 <= temp);

    //下
    m_canvasGridView->d->currentCursorIndex = index21;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(2 == temp);

    //下
    m_canvasGridView->d->currentCursorIndex = index22;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(5 <= temp);

    //左
    m_canvasGridView->d->currentCursorIndex = index12;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(3 <= temp);

    //左
    m_canvasGridView->d->currentCursorIndex = index02;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    temp = m_canvasGridView->selectedUrls().size();
    auto ttt = m_canvasGridView->selectedUrls();
    EXPECT_TRUE(2 <= temp);

    //上
    m_canvasGridView->d->currentCursorIndex = index01;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(2 == temp);

    //上
    m_canvasGridView->d->currentCursorIndex = index00;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    temp = m_canvasGridView->selectedUrls().size();
    EXPECT_TRUE(3 <= temp);

    //选中布局(1,1)图标
    m_canvasGridView->select({DUrl(url11)});
    m_canvasGridView->d->m_oldCursorIndex = index11;
    m_canvasGridView->d->currentCursorIndex = index11;
    stu.reset(&DFMGlobal::keyShiftIsPressed);
    stu.set_lamda(ADDR(DFMGlobal, keyShiftIsPressed), [](){ return false;});
    QTest::keyPress(m_canvasGridView, Qt::Key_Left , Qt::NoModifier);
    waitData(m_canvasGridView);
    //重新选中布局(0,1)图标
    stu.reset(&DFMGlobal::keyShiftIsPressed);
    stu.set_lamda(ADDR(DFMGlobal, keyShiftIsPressed), [](){ return true;});
    m_canvasGridView->d->currentCursorIndex = index00;
    m_canvasGridView->setSelection(QRect(), QItemSelectionModel::SelectCurrent);
    auto sUrls = m_canvasGridView->selectedUrls();
    EXPECT_TRUE(!sUrls.contains(DUrl(url11)));
    //移除测试文件
    QDir dir(path);
    auto temppp = dir.entryInfoList();
    QStringList removeList;
    for (auto tmp : temppp) {
        if (tmp.filePath().contains("forutxxxxxxxxxxxx"))
            removeList.append(tmp.filePath());
    }
    for (auto tpp : removeList) {
        dir.remove(tpp);
    }
}

TEST(CanvasGridViewTest_end, endTest)
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QDir dir(path);
    dir.remove(CanvasGridViewTest::tstFile);
    dir.rmpath(dir.filePath(CanvasGridViewTest::tstDir));
}
