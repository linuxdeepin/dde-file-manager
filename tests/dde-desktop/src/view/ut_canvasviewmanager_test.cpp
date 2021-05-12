#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#define private public
#include "screen/screenhelper.h"
#include <view/canvasviewmanager.h>
#include <view/backgroundmanager.h>
#include <presenter/gridmanager.h>
#include "stub.h"
#include "stubext.h"
#include "desktopitemdelegate.h"
#include "dfilesystemmodel.h"
#include "AbstractStringAppender.h"
#include <QWidget>
#include <QVector>
#include "screen/abstractscreen.h"
#include "screen/screenmanager.h"
namespace  {
class CanvasViewManagerTest : public testing::Test
{
public:
    CanvasViewManagerTest():Test(){
    }
    virtual void SetUp() override{
        if (!m_st && !m_bgm && !m_cvmgr) {
            m_st = new stub_ext::StubExt;
            m_st->set_lamda(ADDR(CanvasGridView, setCurrentUrl), [](){return false;});
            m_st->set_lamda(ADDR(GridManager, initGridItemsInfos), [](){;});

            m_bgm = new BackgroundManager;
            m_cvmgr = new CanvasViewManager(m_bgm);
        }
    }
    virtual void TearDown()override{
    }
public:
    static stub_ext::StubExt *m_st;
    static BackgroundManager *m_bgm;
    static CanvasViewManager *m_cvmgr;
};

stub_ext::StubExt *CanvasViewManagerTest::m_st = nullptr;
BackgroundManager *CanvasViewManagerTest::m_bgm = nullptr;
CanvasViewManager *CanvasViewManagerTest::m_cvmgr = nullptr;
}

TEST_F(CanvasViewManagerTest,Test_CanvasViewManager_Constructor)
{
    ASSERT_NE(m_cvmgr, nullptr);
}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_canvas){
    EXPECT_TRUE(m_cvmgr->m_canvasMap == m_cvmgr->canvas())<< "canvas() not equal m_canvasMap";
}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onCanvasViewBuild)
{

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

    m_cvmgr->m_background->m_backgroundMap.clear();
    m_cvmgr->m_background->m_backgroundEnable = true;
    m_cvmgr->onCanvasViewBuild(0);
    EXPECT_TRUE(m_cvmgr->m_canvasMap.isEmpty());
}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onBackgroundEnableChanged)
{

    stub_ext::StubExt st;
    bool isCallInfos = false;
    st.set_lamda(ADDR(GridManager, initGridItemsInfos), [&isCallInfos](){isCallInfos = true;});

    m_cvmgr->m_background->m_backgroundEnable = true;
    m_cvmgr->onBackgroundEnableChanged();
    for (auto tempSp : m_cvmgr->m_canvasMap.keys()) {
        BackgroundWidgetPointer bw = m_cvmgr->m_background->backgroundWidget(tempSp);
        if (!bw) {
            EXPECT_TRUE(m_cvmgr->m_canvasMap.isEmpty());
            EXPECT_FALSE(isCallInfos);
            return;
        }
        CanvasViewPointer mView = m_cvmgr->m_canvasMap.value(tempSp);
        ASSERT_NE(mView, nullptr);
        EXPECT_FALSE(mView->testAttribute(Qt::WA_NativeWindow));
    }
    EXPECT_TRUE(isCallInfos);

    m_cvmgr->m_background->m_backgroundEnable = false;
    m_cvmgr->onBackgroundEnableChanged();
    for (auto tempSp : m_cvmgr->m_canvasMap.keys()) {
        CanvasViewPointer mView = m_cvmgr->m_canvasMap.value(tempSp);
        ASSERT_NE(mView, nullptr);
        EXPECT_TRUE(mView->windowFlags() & (Qt::FramelessWindowHint));
    }
    EXPECT_TRUE(isCallInfos);

    st.reset(ADDR(GridManager, initGridItemsInfos));
}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onScreenGeometryChanged)
{
#ifdef UNUSED_SMARTDOCK

#else
    for (auto tempKey : m_cvmgr->m_canvasMap.keys()) {
        m_cvmgr->m_canvasMap.find(tempKey).value()->setGeometry(QRect(10,10,10,10));
    }

    void(*geometry)(void*) = [](void* obj){return;};
    stub_ext::StubExt stu;
    typedef void (*fptr)(CanvasGridView*, int);
    fptr A_foo = (fptr)((void(CanvasGridView::*)(const QRect&))&CanvasGridView::setGeometry);
    stu.set(A_foo, geometry);
    m_cvmgr->onCanvasViewBuild(2);
    if (m_cvmgr->m_canvasMap.size()) {
        m_cvmgr->onScreenGeometryChanged();
        for (auto tempVal : m_cvmgr->m_canvasMap.values()) {
            EXPECT_FALSE(tempVal->geometry() == QRect(10,10,10,10));
        }
    }
#endif
}

TEST_F(CanvasViewManagerTest, test_onSyncOperation)
{
    int so;
    QVariant var;

    stub_ext::StubExt st;
    bool isAutoMerge = false;
    bool isCallInfos = false;

    st.set_lamda(ADDR(CanvasGridView, setAutoMerge), [&isAutoMerge](CanvasGridView *obj, bool enable){isAutoMerge = enable;});
    st.set_lamda(ADDR(GridManager, initGridItemsInfos), [&isCallInfos](){isCallInfos = true;});

    so = GridManager::soAutoMerge;
    var = true;
    m_cvmgr->onSyncOperation(so, var);
    if (!m_cvmgr->m_canvasMap.isEmpty())
        EXPECT_EQ(isAutoMerge, var);
    EXPECT_FALSE(isCallInfos);

    var = false;
    m_cvmgr->onSyncOperation(so, var);
    EXPECT_TRUE(isCallInfos);

    st.reset(ADDR(CanvasGridView, setAutoMerge));
    st.reset(ADDR(GridManager, initGridItemsInfos));

    so = GridManager::soRename;
    var = QString("/test/test.txt");
    QString path;
    st.set_lamda(ADDR(CanvasViewManager, arrageEditDeal), [&path](CanvasViewManager *obj, QString file){path = file;});
    m_cvmgr->onSyncOperation(so, var);
    EXPECT_EQ(path, var.toString());
    st.reset(ADDR(CanvasViewManager, arrageEditDeal));

    so = GridManager::soIconSize;
    var = 1;
    int level = 0;
    st.set_lamda(ADDR(CanvasGridView, syncIconLevel), [&level](CanvasGridView *obj, int l){level = l;});
    m_cvmgr->onSyncOperation(so, var);
    if (!m_cvmgr->m_canvasMap.isEmpty())
        EXPECT_EQ(level, var.toInt());
    else
        EXPECT_EQ(level, 0);
    st.reset(ADDR(CanvasGridView, syncIconLevel));

    so = GridManager::soSort;
    var = QPoint(0, 0);
    bool isCallUpdate = false;
    st.set_lamda((void(QWidget::*)())ADDR(QWidget, update), [&isCallUpdate](){isCallUpdate = true;});
    m_cvmgr->onSyncOperation(so, var);
    if (!m_cvmgr->m_canvasMap.values().isEmpty())
        EXPECT_TRUE(isCallUpdate);
    else
        EXPECT_FALSE(isCallUpdate);

    so = GridManager::soHideEditing;
    isCallUpdate = false;
    m_cvmgr->onSyncOperation(so, var);
    if (!m_cvmgr->m_canvasMap.values().isEmpty())
        EXPECT_TRUE(isCallUpdate);
    else
        EXPECT_FALSE(isCallUpdate);

    so = GridManager::soUpdate;
    isCallUpdate = false;
    m_cvmgr->onSyncOperation(so, var);
    if (!m_cvmgr->m_canvasMap.values().isEmpty())
        EXPECT_TRUE(isCallUpdate);
    else
        EXPECT_FALSE(isCallUpdate);

    st.reset((void(QWidget::*)())ADDR(QWidget, update));

    so = GridManager::soAutoMergeUpdate;
    QMap<QString, DUrl> utMap;
    utMap.insert(QString("screen"), DUrl("/test/test/txt"));
    var = QVariant::fromValue(utMap);
    bool isExpandedState = false;
    st.set_lamda(ADDR(CanvasGridView, updateEntryExpandedState), [&isExpandedState](){isExpandedState = true;});
    m_cvmgr->onSyncOperation(so, var);
    if (!m_cvmgr->m_canvasMap.isEmpty())
        EXPECT_TRUE(isExpandedState);
    else
        EXPECT_FALSE(isExpandedState);
    st.reset(ADDR(CanvasGridView, updateEntryExpandedState));

    so = GridManager::soHidenSwitch;
    var = 0;
    bool isHiddenItems = false;
    st.set_lamda(ADDR(CanvasGridView, updateHiddenItems), [&isHiddenItems](){isHiddenItems = true;});
    m_cvmgr->onSyncOperation(so, var);
    if (!m_cvmgr->m_canvasMap.isEmpty())
        EXPECT_TRUE(isHiddenItems);
    else
        EXPECT_FALSE(isHiddenItems);
    st.reset(ADDR(CanvasGridView, updateHiddenItems));

    so = GridManager::soGsettingUpdate;
    bool isModelRefresh = false;
    st.set_lamda(ADDR(CanvasGridView, delayModelRefresh), [&isModelRefresh](){isModelRefresh = true;});
    m_cvmgr->onSyncOperation(so, var);
    if (!m_cvmgr->m_canvasMap.isEmpty())
        EXPECT_TRUE(isModelRefresh);
    else
        EXPECT_FALSE(isModelRefresh);
    st.reset(ADDR(CanvasGridView, delayModelRefresh));
}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_onSyncSelection)
{

    stub_ext::StubExt st;
    bool isCallSelect = false;
    st.set_lamda(ADDR(CanvasGridView, select), [&isCallSelect](){isCallSelect = true;});
    st.set_lamda((void(QWidget::*)())ADDR(QWidget, update), [](){});

    m_cvmgr->onSyncSelection(nullptr, DUrlList());
    if (!m_cvmgr->m_canvasMap.isEmpty())
        EXPECT_TRUE(isCallSelect);
    else
        EXPECT_FALSE(isCallSelect);

    st.reset(ADDR(CanvasGridView, select));
    st.reset((void(QWidget::*)())ADDR(QWidget, update));
}

TEST_F(CanvasViewManagerTest, Test_CanvasViewManager_Slot_init)
{
    stub_ext::StubExt st;
    bool isCallBuild = false;
    st.set_lamda(ADDR(CanvasViewManager, onCanvasViewBuild), [&isCallBuild](){isCallBuild = true;});
    m_cvmgr->init();
    EXPECT_TRUE(isCallBuild);
    st.reset(ADDR(CanvasViewManager, onCanvasViewBuild));
}

TEST_F(CanvasViewManagerTest, Test_extend_CanvasViewManager_Slot_onCanvasViewBuild)
{
    stub_ext::StubExt gstu;
    //GridManager单例构造，对当前所调用的全部打桩，防止不确定的崩溃
    gstu.set_lamda(ADDR(CanvasGridView, initUI), [](){return;});
    gstu.set_lamda(ADDR(CanvasGridView, initConnection), [](){return;});
    gstu.set_lamda(ADDR(GridManager, restCoord), [](){return;});
    gstu.set_lamda(ADDR(GridManager, addCoord), [](){return;});
    gstu.set_lamda(ADDR(GridManager, setDisplayMode), [](){return;});

    {
        stub_ext::StubExt stu;
        bool isnullptr = false;
        stu.set_lamda(VADDR(ScreenManager, primaryScreen), [&isnullptr](){isnullptr = true; return nullptr;});
        stu.set_lamda(ADDR(CanvasViewManager, onBackgroundEnableChanged), [](){});
        m_cvmgr->onCanvasViewBuild(3);
        ASSERT_TRUE(isnullptr);
    }

    {
        stub_ext::StubExt stu;
        bool islogic = false;
        stu.set_lamda(VADDR(ScreenManager, screens), [](){return QVector<ScreenPointer>();});
        stu.set_lamda(ADDR(CanvasViewManager, onBackgroundEnableChanged), [](){});
        m_cvmgr->onCanvasViewBuild(2);
        ASSERT_FALSE(m_cvmgr->m_canvasMap.isEmpty());
        stu.set_lamda(VADDR(ScreenManager, logicScreens), [&islogic](){islogic = true; return QVector<ScreenPointer>();});
        m_cvmgr->onCanvasViewBuild(2);
        ASSERT_TRUE(islogic);
    }

}

TEST_F(CanvasViewManagerTest, test_endAndFree)
{
    if (m_bgm) {
        delete m_bgm;
        m_bgm = nullptr;
    }
    if (m_cvmgr) {
        delete m_cvmgr;
        m_cvmgr = nullptr;
    }
    if (m_st) {
        delete m_st;
        m_st = nullptr;
    }

    EXPECT_EQ(m_bgm, nullptr);
    EXPECT_EQ(m_cvmgr, nullptr);
}
