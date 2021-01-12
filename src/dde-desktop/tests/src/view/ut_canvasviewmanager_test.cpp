#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#define private public
#include "screen/screenhelper.h"
#include <view/canvasviewmanager.h>
#include <view/backgroundmanager.h>
#include <presenter/gridmanager.h>
#include "../third-party/cpp-stub/stub.h"
#include "desktopitemdelegate.h"
#include "dfilesystemmodel.h"
#include "AbstractStringAppender.h"

TEST(CanvasViewManager_Constructor,Test_CanvasViewManager_Constructor)
{
    QScopedPointer<BackgroundManager> p_bkmgr(new BackgroundManager());
    ASSERT_NE(p_bkmgr, nullptr);
    QScopedPointer<CanvasViewManager> p_cvmgr(new CanvasViewManager(p_bkmgr.data()));
    ASSERT_NE(p_cvmgr, nullptr);
}

class CanvasViewManagerTest : public testing::Test
{
public:
    CanvasViewManagerTest():Test(){}
    virtual void SetUp() override{
    }
    virtual void TearDown()override{
    }
public:
    QScopedPointer<CanvasViewManager> m_cvmgr{new CanvasViewManager(new BackgroundManager())};
};


TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_canvas){
    EXPECT_TRUE(m_cvmgr->m_canvasMap == m_cvmgr->canvas())<< "canvas() not equal m_canvasMap";
}


TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onCanvasViewBuild){

    //-1,0、2、4会创建对应屏幕个数CanvasGridView
    m_cvmgr->onCanvasViewBuild(-1);
    EXPECT_EQ(ScreenMrg->screens().size(), m_cvmgr->m_canvasMap.size());

    m_cvmgr->onCanvasViewBuild(0);
    EXPECT_EQ(ScreenMrg->screens().size(), m_cvmgr->m_canvasMap.size());

    m_cvmgr->onCanvasViewBuild(2);
    EXPECT_EQ(ScreenMrg->screens().size(), m_cvmgr->m_canvasMap.size());

    m_cvmgr->onCanvasViewBuild(4);
    EXPECT_EQ(ScreenMrg->screens().size(), m_cvmgr->m_canvasMap.size());

    //1、3直创建一个CanvasGridView；
    m_cvmgr->onCanvasViewBuild(1);
    EXPECT_EQ(1, m_cvmgr->m_canvasMap.size());

    m_cvmgr->onCanvasViewBuild(3);
    EXPECT_EQ(1, m_cvmgr->m_canvasMap.size());
}


TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onBackgroundEnableChanged){

//    //backgroundEnable这种情况由于不好模拟条件暂时无法覆盖，若需要强行覆盖则需要改动源码
//    m_cvmgr_enable.data()->onBackgroundEnableChanged();

//    for (auto tempSp : m_cvmgr_enable.data()->m_canvasMap.values()) {
//        EXPECT_EQ(1, Qt::FramelessWindowHint&tempSp->windowFlags()) << Qt::FramelessWindowHint<<"the flags: " << tempSp->windowFlags();
//    }

    //backgroundAble
    m_cvmgr->onBackgroundEnableChanged();

    for (auto tempSp : m_cvmgr->m_canvasMap.values()) {
        EXPECT_FALSE(tempSp->testAttribute(Qt::WA_NativeWindow));
    }

}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onScreenGeometryChanged){

    for (auto tempKey : m_cvmgr->m_canvasMap.keys()) {
        m_cvmgr->m_canvasMap.find(tempKey).value()->setGeometry(QRect(10,10,10,10));
    }
    m_cvmgr->onScreenGeometryChanged();
    for (auto tempVal : m_cvmgr->m_canvasMap.values()) {
        EXPECT_FALSE(tempVal->geometry() == QRect(10,10,10,10));
    }
}

//select操作设置不成功，可能与事件循环有关，
//todo
//TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onSyncSelection){
//    DUrlList tempDlst;
//    for (auto tempC : m_cvmgr->m_canvasMap.values()) {
//        if(1 == tempC->m_screenNum){
//            //设置主屏地一个图标选中
//            tempDlst << DUrl(GridManager::instance()->firstItemId(1));
//            m_cvmgr->onSyncSelection(static_cast<CanvasGridView*>(tempC.data()),tempDlst);
//            break;
//        }
//    }
//    for (auto tempC : m_cvmgr->m_canvasMap.values()){
//        if(1 != tempC->m_screenNum){
//            auto tp1 = tempC->selectedUrls();
//            bool tempTg = tempDlst == tempC->selectedUrls();
//            EXPECT_TRUE(tempTg);
//            break;
//        }
//    }
//}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_init){

    //backgroundEnable这种情况由于不好模拟条件,所以暂时无法覆盖，若需要强行覆盖则需要改动源码
    m_cvmgr->init();
    auto temp = ScreenMrg->displayMode();
    if((AbstractScreenManager::Showonly == temp) || (AbstractScreenManager::Duplicate == temp)){
        EXPECT_EQ(1, m_cvmgr->m_canvasMap.size());
    }else {
        EXPECT_EQ(ScreenMrg->screens().size(), m_cvmgr->m_canvasMap.size());
    }

    //connect onCanvasViewBuild,屏幕增删，模式改变
    m_cvmgr->m_canvasMap.clear();
    emit m_cvmgr->m_background->sigBackgroundBuilded(1);
    EXPECT_EQ(1, m_cvmgr->m_canvasMap.size());

    //connect onScreenGeometryChanged,屏幕大小改变,可用区改变
    for (auto tempKey : m_cvmgr->m_canvasMap.keys()) {
        m_cvmgr->m_canvasMap.find(tempKey).value()->setGeometry(QRect(10,10,10,10));
    }
    emit ScreenHelper::screenManager()->sigScreenGeometryChanged();
    for (auto tempVal : m_cvmgr->m_canvasMap.values()) {
        EXPECT_FALSE(tempVal->geometry() == QRect(10,10,10,10));
    }

    for (auto tempKey : m_cvmgr->m_canvasMap.keys()) {
        m_cvmgr->m_canvasMap.find(tempKey).value()->setGeometry(QRect(20,20,20,20));
    }

    emit ScreenHelper::screenManager()->sigScreenAvailableGeometryChanged();
    for (auto tempVal : m_cvmgr->m_canvasMap.values()) {
        EXPECT_FALSE(tempVal->geometry() == QRect(20,20,20,20));
    }
    //connect onSyncOperation,grid改变


    //connect onSyncSelection,同步选中状态
    DUrlList tempDlst;
    for (auto tempC : m_cvmgr->m_canvasMap.values()) {
        if(1 == tempC->m_screenNum){
            //设置主屏地一个图标选中
            tempDlst << DUrl(GridManager::instance()->firstItemId(1));
            emit GridManager::instance()->sigSyncSelection(static_cast<CanvasGridView*>(tempC.data()),tempDlst);
            break;
        }
    }
    for (auto tempC : m_cvmgr->m_canvasMap.values()){
        if(1 != tempC->m_screenNum){
            auto tp1 = tempC->selectedUrls();
            EXPECT_TRUE(tempDlst == tempC->selectedUrls());
            break;
        }
    }
}

//TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_arrageEditDeal){

//    //backgroundEnable这种情况由于不好模拟条件,所以暂时无法覆盖，若需要强行覆盖则需要改动源码
//    m_cvmgr->arrageEditDeal(QString());
//}

TEST_F(CanvasViewManagerTest, test_arrageEditDeal)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    path = path + '/' + "test.txt";
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::NewOnly | QIODevice::ReadWrite);
        file.close();
    }
    bool (*myfind)() = [](){return true;};
    int (*myscreennum)() =[](){return 0;};
    Stub tub, tub1;
    tub.set(ADDR(GridManager, find), myfind);
    tub1.set(ADDR(CanvasGridView, screenNum), myscreennum);
    m_cvmgr->arrageEditDeal(path);
}

TEST_F(CanvasViewManagerTest, test_onSyncOperation)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    path = path + '/' + "test.txt";
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::NewOnly | QIODevice::ReadWrite);
        file.close();
    }

    m_cvmgr->onSyncOperation(GridManager::soHideEditing, 0);
    m_cvmgr->onSyncOperation(GridManager::soUpdate, 0);
    m_cvmgr->onSyncOperation(GridManager::soHidenSwitch, 0);
    m_cvmgr->onSyncOperation(GridManager::soIconSize, QVariant(2));
    for (CanvasViewPointer view : m_cvmgr->m_canvasMap.values()){
        EXPECT_EQ(2, view->itemDelegate()->iconSizeLevel());
    }
    m_cvmgr->onSyncOperation(GridManager::soGsettingUpdate, 0);
    m_cvmgr->onSyncOperation(GridManager::soAutoMerge, 0);
    m_cvmgr->onSyncOperation(GridManager::soRename, 0);
    m_cvmgr->onSyncOperation(GridManager::soSort, QPoint(0, 1));
    for (CanvasViewPointer view : m_cvmgr->m_canvasMap.values()){
        EXPECT_EQ(0, view->model()->sortRole());
        EXPECT_EQ(1, view->model()->sortOrder());
    }

    if (file.exists()) {
        QProcess::execute("rm" + path);
    }
}
