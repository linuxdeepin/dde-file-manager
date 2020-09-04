#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define private public
#include "screen/screenhelper.h"
#include <view/canvasviewmanager.h>
#include <view/backgroundmanager.h>
#include <presenter/gridmanager.h>

TEST(CanvasViewManager_Constructor,Test_CanvasViewManager_Constructor)
{
    QScopedPointer<BackgroundManager> p_bkmgr(new BackgroundManager ());
    QScopedPointer<CanvasViewManager> p_cvmgr(new CanvasViewManager (p_bkmgr.data()));

    EXPECT_NE(p_bkmgr,nullptr);
}

class CanvasViewManagerTest : public testing::Test
{
public:
    CanvasViewManagerTest():Test(){}
    virtual void SetUp() override{
        p_bkmgr = new BackgroundManager ();
        p_cvmgr = new CanvasViewManager (p_bkmgr);
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
};


TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_canvas){
    EXPECT_TRUE(p_cvmgr->m_canvasMap == p_cvmgr->canvas())<< "canvas() not equal m_canvasMap";
}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onCanvasViewBuild){

    //-1,0、2、4会创建对应屏幕个数CanvasGridView
    p_cvmgr->onCanvasViewBuild(-1);
    EXPECT_EQ(ScreenMrg->screens().size(), p_cvmgr->m_canvasMap.size());

    p_cvmgr->onCanvasViewBuild(0);
    EXPECT_EQ(ScreenMrg->screens().size(), p_cvmgr->m_canvasMap.size());

    p_cvmgr->onCanvasViewBuild(2);
    EXPECT_EQ(ScreenMrg->screens().size(), p_cvmgr->m_canvasMap.size());

    p_cvmgr->onCanvasViewBuild(4);
    EXPECT_EQ(ScreenMrg->screens().size(), p_cvmgr->m_canvasMap.size());

    //1、3直创建一个CanvasGridView；
    p_cvmgr->onCanvasViewBuild(1);
    EXPECT_EQ(1, p_cvmgr->m_canvasMap.size());

    p_cvmgr->onCanvasViewBuild(3);
    EXPECT_EQ(1, p_cvmgr->m_canvasMap.size());
}


TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onBackgroundEnableChanged){

//    //backgroundEnable这种情况由于不好模拟条件暂时无法覆盖，若需要强行覆盖则需要改动源码
//    p_cvmgr_enable.data()->onBackgroundEnableChanged();

//    for (auto tempSp : p_cvmgr_enable.data()->m_canvasMap.values()) {
//        EXPECT_EQ(1, Qt::FramelessWindowHint&tempSp->windowFlags()) << Qt::FramelessWindowHint<<"the flags: " << tempSp->windowFlags();
//    }

    //backgroundAble
    p_cvmgr->onBackgroundEnableChanged();

    for (auto tempSp : p_cvmgr->m_canvasMap.values()) {
        EXPECT_FALSE(tempSp->testAttribute(Qt::WA_NativeWindow));
    }

}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onScreenGeometryChanged){

    for (auto tempKey : p_cvmgr->m_canvasMap.keys()) {
        p_cvmgr->m_canvasMap.find(tempKey).value()->setGeometry(QRect(10,10,10,10));
    }
    p_cvmgr->onScreenGeometryChanged();
    for (auto tempVal : p_cvmgr->m_canvasMap.values()) {
        EXPECT_FALSE(tempVal->geometry() == QRect(10,10,10,10));
    }
}

//TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onSyncOperation){

//    //backgroundEnable这种情况由于不好模拟条件,所以暂时无法覆盖，若需要强行覆盖则需要改动源码
//    p_cvmgr->onSyncOperation(GridManager::soAutoMerge,true);
//}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onSyncSelection){
    DUrlList tempDlst;
    for (auto tempC : p_cvmgr->m_canvasMap.values()) {
        if(1 == tempC->m_screenNum){
            //设置主屏地一个图标选中
            tempDlst << DUrl(GridManager::instance()->firstItemId(1));
            p_cvmgr->onSyncSelection(static_cast<CanvasGridView*>(tempC.data()),tempDlst);
            break;
        }
    }
    for (auto tempC : p_cvmgr->m_canvasMap.values()){
        if(1 != tempC->m_screenNum){
            auto tp1 = tempC->selectedUrls();
            bool tempTg = tempDlst == tempC->selectedUrls();
            EXPECT_TRUE(tempTg);
            break;
        }
    }
}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_init){

    //backgroundEnable这种情况由于不好模拟条件,所以暂时无法覆盖，若需要强行覆盖则需要改动源码
    p_cvmgr->init();
    auto temp = ScreenMrg->displayMode();
    if((AbstractScreenManager::Showonly == temp) || (AbstractScreenManager::Duplicate == temp)){
        EXPECT_EQ(1, p_cvmgr->m_canvasMap.size());
    }else {
        EXPECT_EQ(ScreenMrg->screens().size(), p_cvmgr->m_canvasMap.size());
    }

    //connect onCanvasViewBuild,屏幕增删，模式改变
    p_cvmgr->m_canvasMap.clear();
    emit p_bkmgr->sigBackgroundBuilded(1);
    EXPECT_EQ(1, p_cvmgr->m_canvasMap.size());

    //connect onScreenGeometryChanged,屏幕大小改变,可用区改变
    for (auto tempKey : p_cvmgr->m_canvasMap.keys()) {
        p_cvmgr->m_canvasMap.find(tempKey).value()->setGeometry(QRect(10,10,10,10));
    }
    emit ScreenHelper::screenManager()->sigScreenGeometryChanged();
    for (auto tempVal : p_cvmgr->m_canvasMap.values()) {
        EXPECT_FALSE(tempVal->geometry() == QRect(10,10,10,10));
    }

    for (auto tempKey : p_cvmgr->m_canvasMap.keys()) {
        p_cvmgr->m_canvasMap.find(tempKey).value()->setGeometry(QRect(20,20,20,20));
    }

    emit ScreenHelper::screenManager()->sigScreenAvailableGeometryChanged();
    for (auto tempVal : p_cvmgr->m_canvasMap.values()) {
        EXPECT_FALSE(tempVal->geometry() == QRect(20,20,20,20));
    }
    //connect onSyncOperation,grid改变


    //connect onSyncSelection,同步选中状态
    DUrlList tempDlst;
    for (auto tempC : p_cvmgr->m_canvasMap.values()) {
        if(1 == tempC->m_screenNum){
            //设置主屏地一个图标选中
            tempDlst << DUrl(GridManager::instance()->firstItemId(1));
            emit GridManager::instance()->sigSyncSelection(static_cast<CanvasGridView*>(tempC.data()),tempDlst);
            break;
        }
    }
    for (auto tempC : p_cvmgr->m_canvasMap.values()){
        if(1 != tempC->m_screenNum){
            auto tp1 = tempC->selectedUrls();
            EXPECT_TRUE(tempDlst == tempC->selectedUrls());
            break;
        }
    }
}

//TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_arrageEditDeal){

//    //backgroundEnable这种情况由于不好模拟条件,所以暂时无法覆盖，若需要强行覆盖则需要改动源码
//    p_cvmgr->arrageEditDeal(QString());
//}
