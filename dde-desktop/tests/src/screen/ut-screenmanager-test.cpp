#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <screen/screenmanager.h>
#include <screen/screenobject.h>
#include <screen/screenhelper.h>
#include <screen/screenmanagerwayland.h>
#include <screen/screenobjectwayland.h>
#include <QApplication>
#include <QList>
#include <qpa/qplatformscreen.h>

using namespace testing;
namespace  {
    class ScreenManagerTest : public Test
    {
    public:
        ScreenManagerTest():Test()
        {
            qunsetenv("XDG_SESSION_TYPE");
            qunsetenv("WAYLAND_DISPLAY");
        }
        QVector<ScreenPointer> screens;
        QList<QScreen *> reference;
        virtual void SetUp() override{
            screens = ScreenMrg->screens();
            reference = qApp->screens();
        }

        virtual void TearDown() override{
        }
    };
    class ScreenManagerTestWayland : public ScreenManagerTest
    {
    public:
        ScreenManagerTestWayland() : ScreenManagerTest()
        {
            qputenv("XDG_SESSION_TYPE","wayland");
            qputenv("WAYLAND_DISPLAY","wayland");
        }
        ~ScreenManagerTestWayland()
        {
            qunsetenv("XDG_SESSION_TYPE");
            qunsetenv("WAYLAND_DISPLAY");
        }

    };
}

TEST_F(ScreenManagerTest, screen_object)
{
    EXPECT_EQ(reference.size(),screens.size());

    for (ScreenPointer obj : screens){
        bool find = false;
        for (QScreen *sc : reference){
            if (sc->name() == obj->name()){
                find = true;
            }
        }
        EXPECT_EQ(true,find);
    }
}

TEST_F(ScreenManagerTestWayland, screen_object)
{   
    EXPECT_EQ(reference.size(),screens.size());

    for (ScreenPointer obj : screens){
        bool find = false;
        for (QScreen *sc : reference){
            if (sc->name() == obj->name()){
                find = true;
            }
        }
        EXPECT_EQ(true,find) << "invaild screen" << obj->name().toStdString().c_str();
    }
}

TEST(ScreenObject,geometry)
{
    ScreenManager sm;
    auto screenList = sm.logicScreens();
    ScreenManagerWayland smw;
    auto myScreens = smw.logicScreens();
    EXPECT_EQ(screenList.size(),myScreens.size());

    for (int i = 0; i < myScreens.size(); ++i){
        ScreenPointer obj = myScreens[i];
        ScreenPointer sc = screenList[i];
        EXPECT_EQ(sc->name(),obj->name());
        EXPECT_EQ(sc->availableGeometry(),obj->availableGeometry());
        EXPECT_EQ(sc->handleGeometry(),obj->handleGeometry());
    }
}

TEST(ScreenObject,primary_screen)
{
    ScreenManager sm;
    EXPECT_EQ(qApp->primaryScreen()->name(),sm.primaryScreen()->name());

    ScreenManagerWayland smw;
    EXPECT_EQ(qApp->primaryScreen()->name(),smw.primaryScreen()->name());
}

TEST(ScreenObject,display_mode)
{
    ScreenManager sm;
    ScreenManagerWayland smw;
    EXPECT_EQ(sm.displayMode(),smw.displayMode());
}

TEST(ScreenObject,device_pixel_ratio)
{
    ScreenManager sm;
    ScreenManagerWayland smw;
    EXPECT_EQ(sm.devicePixelRatio(),smw.devicePixelRatio());
}

namespace  {
class ScreenOperation : public Test , public ScreenManager
{
public:
    ScreenOperation() :
            Test()
          ,ScreenManager()
    {}
};
}

TEST_F(ScreenOperation, screen_remove)
{
    int count = screens().size();
    onScreenRemoved(nullptr);
    EXPECT_EQ(count, screens().size());

    for (QScreen *sc : qApp->screens()){
        onScreenRemoved(sc);
        bool find = false;
        for (ScreenPointer obj : screens()){
            if (sc->name() == obj->name()){
                find = true;
                break;
            }
        }
        EXPECT_EQ(false,find);
    }

    EXPECT_EQ(true,screens().isEmpty());
}

TEST_F(ScreenOperation, screen_added)
{
    for (QScreen *sc : qApp->screens()){
        onScreenRemoved(sc);
    }
    EXPECT_EQ(true,screens().isEmpty());

    for (QScreen *sc : qApp->screens()){
        onScreenAdded(sc);
        bool find = false;
        for (ScreenPointer obj : screens()){
            if (sc->name() == obj->name()){
                find = true;
                break;
            }
        }

        EXPECT_EQ(true,find);
    }

    int count = screens().size();
    onScreenAdded(nullptr);
    EXPECT_EQ(count,screens().size());


    for (QScreen *sc : qApp->screens()){
        onScreenAdded(sc);
    }
    EXPECT_EQ(count,screens().size());
}

TEST_F(ScreenOperation, screen_reset)
{
    for (QScreen *sc : qApp->screens()){
        onScreenRemoved(sc);
    }
    EXPECT_EQ(true,screens().isEmpty());

    reset();

    EXPECT_EQ(qApp->screens().size(),screens().size());
}

namespace  {
class ScreenSignal : public Test , public ScreenManager
{
public:
    ScreenSignal() :
            Test()
          ,ScreenManager()
    {}
    bool sreenDisplayChanged = false;
    bool sreenGeometryChanged = false;
    bool sreenChanged = false;
    bool sreenAvailableGeometryChanged = false;
    void init(QEventLoop *loop)
    {
        connect(this,&AbstractScreenManager::sigDisplayModeChanged,this,[=](){
            sreenDisplayChanged = true;
            loop->exit();
        });

        connect(this,&AbstractScreenManager::sigScreenChanged,this,[=](){
            sreenChanged = true;
            loop->exit();
        });

        connect(this,&AbstractScreenManager::sigScreenGeometryChanged,this,[=](){
            sreenGeometryChanged = true;
            loop->exit();
        });

        connect(this,&AbstractScreenManager::sigScreenAvailableGeometryChanged,this,[=](){
            sreenAvailableGeometryChanged = true;
            loop->exit();
        });
    }
};
}

TEST_F(ScreenSignal, screen_display_changed)
{
    QEventLoop loop;
    init(&loop);
    appendEvent(Mode);
    appendEvent(Screen);
    appendEvent(Geometry);
    appendEvent(AvailableGeometry);
    loop.exec();
    EXPECT_EQ(true,sreenDisplayChanged);
    EXPECT_EQ(false,sreenChanged);
    EXPECT_EQ(false,sreenGeometryChanged);
    EXPECT_EQ(false,sreenAvailableGeometryChanged);
}

TEST_F(ScreenSignal, screen_changed)
{
    QEventLoop loop;
    init(&loop);
    appendEvent(Screen);
    appendEvent(Geometry);
    appendEvent(AvailableGeometry);
    loop.exec();
    EXPECT_EQ(false,sreenDisplayChanged);
    EXPECT_EQ(true,sreenChanged);
    EXPECT_EQ(false,sreenGeometryChanged);
    EXPECT_EQ(false,sreenAvailableGeometryChanged);
}

TEST_F(ScreenSignal, screen_geometry_changed)
{
    QEventLoop loop;
    init(&loop);

    appendEvent(Geometry);
    appendEvent(AvailableGeometry);
    loop.exec();
    EXPECT_EQ(false,sreenDisplayChanged);
    EXPECT_EQ(false,sreenChanged);
    EXPECT_EQ(true,sreenGeometryChanged);
    EXPECT_EQ(false,sreenAvailableGeometryChanged);
}

TEST_F(ScreenSignal, sreen_available_geometry_changed)
{
    QEventLoop loop;
    init(&loop);
    appendEvent(AvailableGeometry);
    loop.exec();
    EXPECT_EQ(false,sreenDisplayChanged);
    EXPECT_EQ(false,sreenChanged);
    EXPECT_EQ(false,sreenGeometryChanged);
    EXPECT_EQ(true,sreenAvailableGeometryChanged);
}
