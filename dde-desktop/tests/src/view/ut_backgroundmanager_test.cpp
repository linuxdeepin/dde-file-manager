#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include <DWindowManagerHelper>
#include <QScopedPointer>
#include <QGuiApplication>
#include <QScreen>

#define private public
#define protected public
#include "view/backgroundmanager.h"
#include "view/backgroundwidget.h"
#include "view/canvasviewmanager.h"
#include "screen/screenhelper.h"

using namespace testing;
namespace  {
    class BackgroundManagerTest : public Test
    {
    public:
        BackgroundManagerTest():Test()
        {
            windowManagerHelper = DWindowManagerHelper::instance();
            screens = ScreenMrg->screens();
            primaryScreen = ScreenMrg->primaryScreen();
        }

        virtual void SetUp() override {
            p_preview_true = new BackgroundManager(true, nullptr);
            p_preview_false = new BackgroundManager(false, nullptr);
        }

        virtual void TearDown() override {
            delete p_preview_true;
            delete p_preview_false;
        }

        BackgroundManager * p_preview_true;
        BackgroundManager * p_preview_false;
        DWindowManagerHelper* windowManagerHelper = nullptr;
        QVector<ScreenPointer> screens;
        ScreenPointer primaryScreen;
    };
}

TEST_F(BackgroundManagerTest, BackgroundManager)
{
    EXPECT_EQ(p_preview_true->m_preview, true);
    EXPECT_EQ(p_preview_false->m_preview, false);
    ASSERT_NE(primaryScreen, nullptr);
    ASSERT_NE(screens.size(), 0);
}

TEST_F(BackgroundManagerTest, isEnabled)
{
    bool result = windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM || !windowManagerHelper->hasComposite();
    EXPECT_EQ(p_preview_true->isEnabled(), result);
    EXPECT_EQ(p_preview_false->isEnabled(), result);
}

TEST_F(BackgroundManagerTest, visible)
{
    p_preview_true->setVisible(true);
    p_preview_false->setVisible(false);

    EXPECT_EQ(p_preview_true->isVisible(), true);
    EXPECT_EQ(p_preview_false->isVisible(), false);

    for(BackgroundWidgetPointer w: p_preview_true->m_backgroundMap.values()) {
        EXPECT_EQ(w->isVisible(), true);
    }
    for(BackgroundWidgetPointer w: p_preview_false->m_backgroundMap.values()) {
        EXPECT_EQ(w->isVisible(), false);
    }
}

TEST_F(BackgroundManagerTest, backgroundWidget)
{
    BackgroundWidgetPointer bwp = p_preview_true->backgroundWidget(primaryScreen);
    ASSERT_NE(bwp, nullptr);
    EXPECT_EQ(bwp->property("isPreview").toBool(), true);
    EXPECT_EQ(bwp->property("myScreen").toString(), primaryScreen->name());
    EXPECT_EQ(bwp->geometry(), primaryScreen->geometry());
}

TEST_F(BackgroundManagerTest, allbackgroundWidgets)
{
    QMap<ScreenPointer,BackgroundWidgetPointer> bwps = p_preview_true->allbackgroundWidgets();
    EXPECT_EQ(bwps.size(), p_preview_true->m_backgroundMap.size());
    for(BackgroundWidgetPointer w : bwps) {
        EXPECT_NE(w, nullptr);

        bool find = false;
        ScreenPointer s = bwps.key(w);
        for (ScreenPointer obj : screens){
            if (s->name() == obj->name()){
                EXPECT_EQ(w->property("isPreview"), true);
                EXPECT_EQ(w->property("myScreen").toString(), obj->name());
                EXPECT_EQ(w->geometry(), obj->geometry());
                find = true;
                break;
            }
        }
        EXPECT_EQ(find,true);
    }
}

TEST_F(BackgroundManagerTest, setBackgroundImage)
{
    QString path("imagePathTest");
    p_preview_true->setBackgroundImage(primaryScreen->name(), path);

    QMap<QString,QString> images = p_preview_true->backgroundImages();

    EXPECT_FALSE(images.isEmpty());
    if(!images.isEmpty()) {
        EXPECT_TRUE(images.keys().contains(primaryScreen->name()));
        EXPECT_EQ(images.value(primaryScreen->name()), path);
    }

    p_preview_true->setBackgroundImage(QString("testName"), path);
    images = p_preview_true->backgroundImages();
    EXPECT_FALSE(images.keys().contains(QString("testName")));
}

TEST_F(BackgroundManagerTest, onBackgroundBuild)
{
    QObject::connect(p_preview_true, &BackgroundManager::sigBackgroundBuilded, [=](int mode){
        EXPECT_EQ(mode, ScreenMrg->displayMode());
    });
    p_preview_true->onBackgroundBuild();
    EXPECT_EQ(p_preview_true->m_backgroundMap.size(), screens.size());
    ASSERT_EQ(p_preview_true->m_backgroundMap.keys().contains(primaryScreen), true);
    EXPECT_EQ(p_preview_true->m_backgroundMap.value(primaryScreen)->geometry(), primaryScreen->geometry());
}

TEST_F(BackgroundManagerTest, onSkipBackgroundBuild)
{
    QObject::connect(p_preview_true, &BackgroundManager::sigBackgroundBuilded, [=](int mode){
        EXPECT_EQ(mode, ScreenMrg->displayMode());
    });
    auto oldMap = p_preview_true->m_backgroundMap;
    p_preview_true->onSkipBackgroundBuild();
    auto newMap = p_preview_true->m_backgroundMap;
    EXPECT_EQ(oldMap, newMap);
}

TEST_F(BackgroundManagerTest, onResetBackgroundImage)
{
    QString path("imagePathTest");
    p_preview_true->m_backgroundImagePath[primaryScreen->name()] = path;

    auto oldImages = p_preview_true->m_backgroundImagePath;
    p_preview_true->onResetBackgroundImage();
    auto newImages = p_preview_true->m_backgroundImagePath;
    EXPECT_EQ(oldImages, newImages);

    p_preview_false->m_backgroundImagePath[QString("testName")] = path;
    oldImages = p_preview_false->m_backgroundImagePath;
    p_preview_false->onResetBackgroundImage();
    newImages = p_preview_false->m_backgroundImagePath;
    EXPECT_NE(oldImages, newImages);
}

TEST_F(BackgroundManagerTest, onRestBackgroundManager)
{
    bool unknow = p_preview_true->m_preview || p_preview_true->isEnabled();
    p_preview_true->onRestBackgroundManager();
    if(unknow) {
        EXPECT_NE(p_preview_true->wmInter, nullptr);
        EXPECT_NE(p_preview_true->gsettings, nullptr);
    } else {
        EXPECT_EQ(p_preview_true->wmInter, nullptr);
        EXPECT_EQ(p_preview_true->gsettings, nullptr);
    }
}

TEST_F(BackgroundManagerTest, onScreenGeometryChanged)
{
    p_preview_true->onScreenGeometryChanged();
    for (ScreenPointer sp : p_preview_true->m_backgroundMap.keys()) {
        BackgroundWidgetPointer bw = p_preview_true->m_backgroundMap.value(sp);
        ASSERT_NE(bw, nullptr);
        EXPECT_EQ(bw->geometry(), sp->geometry());
    }
}

TEST_F(BackgroundManagerTest, init)
{
    bool unknow = p_preview_true->m_preview || p_preview_true->isEnabled();
    p_preview_true->init();
    if(unknow) {
        EXPECT_NE(p_preview_true->wmInter, nullptr);
        EXPECT_NE(p_preview_true->gsettings, nullptr);
    } else {
        EXPECT_EQ(p_preview_true->wmInter, nullptr);
        EXPECT_EQ(p_preview_true->gsettings, nullptr);
    }
}

TEST_F(BackgroundManagerTest, pullImageSettings)
{
    p_preview_true->pullImageSettings();
    EXPECT_EQ(p_preview_true->m_backgroundImagePath.count(), screens.count());
}

TEST_F(BackgroundManagerTest, getBackgroundFromWm)
{
    EXPECT_FALSE(p_preview_true->getBackgroundFromWm(primaryScreen->name()).isEmpty());
    EXPECT_FALSE(p_preview_false->getBackgroundFromWm(primaryScreen->name()).isEmpty());
}

TEST_F(BackgroundManagerTest, getDefaultBackground)
{
    EXPECT_FALSE(p_preview_true->getDefaultBackground().isEmpty());
    EXPECT_FALSE(p_preview_false->getDefaultBackground().isEmpty());
}

TEST_F(BackgroundManagerTest, createBackgroundWidget)
{
    BackgroundWidgetPointer bwp = p_preview_true->createBackgroundWidget(primaryScreen);
    EXPECT_EQ(bwp->property("isPreview").toBool(), true);
    EXPECT_EQ(bwp->property("myScreen").toString(), primaryScreen->name());
    EXPECT_EQ(bwp->geometry(), primaryScreen->geometry());
}

TEST_F(BackgroundManagerTest, sigBackgroundManager)
{
    QObject::connect(p_preview_true, &BackgroundManager::sigBackgroundBuilded, [=](int mode) {
        EXPECT_EQ(mode, 0);
    });
    emit p_preview_true->sigBackgroundBuilded(0);

    QObject::connect(p_preview_false, &BackgroundManager::sigBackgroundBuilded, [=](int mode) {
        EXPECT_EQ(mode, -1);
    });
    emit p_preview_false->sigBackgroundBuilded(-1);

}
