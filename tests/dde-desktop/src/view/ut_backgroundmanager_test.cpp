#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "stubext.h"

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
#include "screen/abstractscreenmanager.h"
#include "screen/screenmanager.h"
#include "screen/screenobject.h"
#include "util/dde/desktopinfo.h"
#include "util/util.h"

#include <private/qdbusmessage_p.h>

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
            stub = new stub_ext::StubExt;
            stub->set_lamda(&DesktopUtil::set_desktop_window,[](){});
            stub->set_lamda(ADDR(BackgroundWidget,show),[](){});
            m_manager = new BackgroundManager(false, nullptr);
        }

        virtual void TearDown() override {
            delete stub;
            delete m_manager;
        }
        stub_ext::StubExt *stub = nullptr;
        BackgroundManager * m_manager;
        DWindowManagerHelper* windowManagerHelper = nullptr;
        QVector<ScreenPointer> screens;
        ScreenPointer primaryScreen;
    };
}

TEST_F(BackgroundManagerTest, BackgroundManager)
{
    EXPECT_EQ(m_manager->m_preview, false);
    ASSERT_NE(primaryScreen, nullptr);
    ASSERT_NE(screens.size(), 0);
}

TEST_F(BackgroundManagerTest, isEnabled)
{
    bool result = true;
    EXPECT_EQ(m_manager->isEnabled(), result);
}

TEST_F(BackgroundManagerTest, visible)
{
    m_manager->setVisible(true);

    EXPECT_EQ(m_manager->isVisible(), true);

    for(BackgroundWidgetPointer w: m_manager->m_backgroundMap.values()) {
        EXPECT_EQ(w->isVisible(), true);
    }
}

TEST_F(BackgroundManagerTest, backgroundWidget)
{
    BackgroundWidgetPointer bwp = m_manager->backgroundWidget(primaryScreen);
    ASSERT_NE(bwp, nullptr);
    EXPECT_EQ(bwp->property("isPreview").toBool(), false);
    EXPECT_EQ(bwp->property("myScreen").toString(), primaryScreen->name());
    EXPECT_EQ(bwp->geometry(), primaryScreen->geometry());
}

TEST_F(BackgroundManagerTest, allbackgroundWidgets)
{
    QMap<ScreenPointer,BackgroundWidgetPointer> bwps = m_manager->allbackgroundWidgets();
    EXPECT_EQ(bwps.size(), m_manager->m_backgroundMap.size());
    for(BackgroundWidgetPointer w : bwps) {
        EXPECT_NE(w, nullptr);

        bool find = false;
        ScreenPointer s = bwps.key(w);
        for (ScreenPointer obj : screens){
            if (s->name() == obj->name()){
                EXPECT_EQ(w->property("isPreview"), false);
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
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(BackgroundManager, onResetBackgroundImage), [](){return;});
    QString path;
    QString screenname = "HDMI";
    m_manager->setBackgroundImage(screenname, path);

    path = "imagePathTest";
    m_manager->setBackgroundImage(screenname, path);

    QMap<QString,QString> images = m_manager->backgroundImages();

    EXPECT_FALSE(images.isEmpty());
    if(!images.isEmpty()) {
        EXPECT_TRUE(images.keys().contains(screenname));
        EXPECT_EQ(images.value(screenname), path);
    }
}

TEST_F(BackgroundManagerTest, onBackgroundBuild)
{
    int oriMode = ScreenMrg->m_lastMode;

    // 测试单屏创建对象
    m_manager->m_backgroundMap.clear();
    ScreenMrg->m_lastMode = AbstractScreenManager::Showonly;
    m_manager->onBackgroundBuild();
    ASSERT_EQ(m_manager->m_backgroundMap.keys().contains(primaryScreen), true);
    EXPECT_EQ(m_manager->m_backgroundMap.value(primaryScreen)->geometry(), primaryScreen->geometry());

    // 测试单屏复用对象
    bool oriVisible = m_manager->m_visible;
    m_manager->m_visible = false;
    m_manager->m_backgroundMap.value(primaryScreen)->setGeometry(100, 100, 100, 100);
    m_manager->onBackgroundBuild();
    EXPECT_EQ(m_manager->m_backgroundMap.value(primaryScreen)->geometry(), primaryScreen->geometry());
    m_manager->m_visible = oriVisible;

    ScreenManager *screenManager = static_cast<ScreenManager*>(ScreenMrg);
    QMap<QScreen *, ScreenPointer>  oriScreens;

    if (screenManager) {
        oriScreens = screenManager->m_screens;
        if (screenManager->m_screens.count() == 1) {
        ScreenPointer sp = screenManager->m_screens.first();
        screenManager->m_screens.insert(nullptr, sp);
        }
    }
    if (screenManager && screenManager->m_screens.count() != 1) {
        ScreenMrg->m_lastMode = AbstractScreenManager::Custom;
        // 多屏创建对象
        m_manager->m_backgroundMap.clear();
        ScreenPointer delPointer(new ScreenObject(qApp->primaryScreen()));
        BackgroundWidgetPointer delBackgroundWidgetPointer(m_manager->createBackgroundWidget(delPointer));
        m_manager->m_backgroundMap.insert(delPointer, delBackgroundWidgetPointer);

        m_manager->onBackgroundBuild();
        EXPECT_EQ(m_manager->m_backgroundMap.size(), screens.size());
        ASSERT_EQ(m_manager->m_backgroundMap.keys().contains(primaryScreen), true);
        EXPECT_EQ(m_manager->m_backgroundMap.value(primaryScreen)->geometry(), primaryScreen->geometry());

        // 多屏复用对象
        bool visibles = m_manager->m_visible;
        m_manager->m_visible = false;
        m_manager->m_backgroundMap.value(primaryScreen)->setGeometry(100, 100, 100, 100);
        m_manager->onBackgroundBuild();
        EXPECT_EQ(m_manager->m_backgroundMap.value(primaryScreen)->geometry(), primaryScreen->geometry());
        m_manager->m_visible = visibles;
    }

    //测试单屏代码中主屏获取错误
    if (screenManager) {
        screenManager->m_screens.clear();
        ScreenMrg->m_lastMode = AbstractScreenManager::Showonly;
        m_manager->onBackgroundBuild();
        EXPECT_TRUE(m_manager->m_backgroundMap.isEmpty());

        screenManager->m_screens = oriScreens;
    }
    ScreenMrg->m_lastMode = oriMode;
}

TEST_F(BackgroundManagerTest, onSkipBackgroundBuild)
{
    QObject::connect(m_manager, &BackgroundManager::sigBackgroundBuilded, [=](int mode){
        EXPECT_EQ(mode, ScreenMrg->displayMode());
    });
    auto oldMap = m_manager->m_backgroundMap;
    m_manager->onSkipBackgroundBuild();
    auto newMap = m_manager->m_backgroundMap;
    EXPECT_EQ(oldMap, newMap);
}

TEST_F(BackgroundManagerTest, onWmDbusStarted)
{
    QString name("com.deepin.wm");
    m_manager->onWmDbusStarted(name, QString(), QString());
}

TEST_F(BackgroundManagerTest, onResetBackgroundImage)
{
    QString path("imagePathTest");
    m_manager->m_backgroundImagePath[primaryScreen->name()] = path;

    auto oldImages = m_manager->m_backgroundImagePath;
    m_manager->onResetBackgroundImage();
    auto newImages = m_manager->m_backgroundImagePath;
    EXPECT_EQ(oldImages, newImages);
}

TEST_F(BackgroundManagerTest, onRestBackgroundManager)
{
    bool unknow = m_manager->m_preview || m_manager->isEnabled();
    m_manager->onRestBackgroundManager();
    if(unknow) {
        EXPECT_NE(m_manager->wmInter, nullptr);
        EXPECT_NE(m_manager->gsettings, nullptr);
    } else {
        EXPECT_EQ(m_manager->wmInter, nullptr);
        EXPECT_EQ(m_manager->gsettings, nullptr);
    }

    bool ori = m_manager->m_backgroundEnable;
    m_manager->m_backgroundEnable = false;
    unknow = m_manager->m_preview || m_manager->isEnabled();
    m_manager->onRestBackgroundManager();
    if(unknow) {
        EXPECT_NE(m_manager->wmInter, nullptr);
        EXPECT_NE(m_manager->gsettings, nullptr);
    } else {
        EXPECT_EQ(m_manager->wmInter, nullptr);
        EXPECT_EQ(m_manager->gsettings, nullptr);
    }
    m_manager->m_backgroundEnable = ori;
}

TEST_F(BackgroundManagerTest, onScreenGeometryChanged)
{
    m_manager->onScreenGeometryChanged();
    for (ScreenPointer sp : m_manager->m_backgroundMap.keys()) {
        BackgroundWidgetPointer bw = m_manager->m_backgroundMap.value(sp);
        ASSERT_NE(bw, nullptr);
        EXPECT_EQ(bw->geometry(), sp->geometry());
        QRect tmpRect(sp->geometry().x()+10, sp->geometry().y()+10, sp->geometry().width(), sp->geometry().height());
        bw->setGeometry(tmpRect);
    }
    m_manager->onScreenGeometryChanged();
}

TEST_F(BackgroundManagerTest, init)
{
    bool unknow = m_manager->m_preview || m_manager->isEnabled();
    m_manager->init();
    if(unknow) {
        EXPECT_NE(m_manager->wmInter, nullptr);
        EXPECT_NE(m_manager->gsettings, nullptr);
    } else {
        EXPECT_EQ(m_manager->wmInter, nullptr);
        EXPECT_EQ(m_manager->gsettings, nullptr);
    }
}

TEST_F(BackgroundManagerTest, pullImageSettings)
{
    m_manager->pullImageSettings();
    EXPECT_EQ(m_manager->m_backgroundImagePath.count(), screens.count());
}

TEST_F(BackgroundManagerTest, getBackgroundFromWm)
{
    ASSERT_NE(m_manager->wmInter, nullptr);
    stub_ext::StubExt stu;
    QString defaultpath("file:///usr/share/backgrounds/default_background.jpg");
    QDBusPendingReply<QString> reply;
    {
        //构造dbus返回
        QDBusMessage message;
        message = message.createReply(QList<QVariant> {defaultpath});
        message.d_ptr->signature = "s";
        reply = QDBusPendingCall::fromCompletedCall(message);
    }

    QString ret = m_manager->getBackgroundFromWm("");
    EXPECT_TRUE(ret.isEmpty());

    //kwin正常
    stu.set_lamda(ADDR(WMInter, GetCurrentWorkspaceBackgroundForMonitor), [reply]() {return reply;});
    ret = m_manager->getBackgroundFromWm("test_screen");
    EXPECT_EQ(ret.toStdString(), defaultpath.toStdString());

    //kwin空，配置文件空
    stu.reset(ADDR(WMInter, GetCurrentWorkspaceBackgroundForMonitor));
    stu.set_lamda(ADDR(WMInter, GetCurrentWorkspaceBackgroundForMonitor), [](){ return QDBusPendingReply<QString>();});
    stu.set_lamda(ADDR(BackgroundManager, getBackgroundFromWmConfig), [](){return QString();});
    ret = m_manager->getBackgroundFromWm("test_screen");
    EXPECT_FALSE(ret.isEmpty());

    //kwin空，配置文件有
    stu.reset(ADDR(WMInter, GetCurrentWorkspaceBackgroundForMonitor));
    stu.set_lamda(ADDR(WMInter, GetCurrentWorkspaceBackgroundForMonitor), [](){ return QDBusPendingReply<QString>();});
    stu.reset(ADDR(BackgroundManager, getBackgroundFromWmConfig));
    stu.set_lamda(ADDR(BackgroundManager, getBackgroundFromWmConfig), [defaultpath](){return defaultpath;});
    ret = m_manager->getBackgroundFromWm("test_screen");
    EXPECT_EQ(ret.toStdString(), defaultpath.toStdString());
}

TEST_F(BackgroundManagerTest, getBackgroundFromWmConfig)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(QString, mid), [](){return "";});
    EXPECT_TRUE(m_manager->getBackgroundFromWmConfig("test_screen").trimmed().isEmpty());
}

TEST_F(BackgroundManagerTest, getDefaultBackground)
{
    EXPECT_FALSE(m_manager->getDefaultBackground().isEmpty());
}

TEST_F(BackgroundManagerTest, createBackgroundWidget)
{
    BackgroundWidgetPointer bwp = m_manager->createBackgroundWidget(primaryScreen);
    EXPECT_EQ(bwp->property("isPreview").toBool(), false);
    EXPECT_EQ(bwp->property("myScreen").toString(), primaryScreen->name());
    EXPECT_EQ(bwp->geometry(), primaryScreen->geometry());
}

TEST_F(BackgroundManagerTest, sigBackgroundManager)
{
    QObject::connect(m_manager, &BackgroundManager::sigBackgroundBuilded, [=](int mode) {
        EXPECT_EQ(mode, 0);
    });
    emit m_manager->sigBackgroundBuilded(0);
}
