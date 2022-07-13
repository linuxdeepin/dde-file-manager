/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QApplication>
#include <QList>
#include <qpa/qplatformscreen.h>
#include "stub.h"
#include "stubext.h"
#include "dbus/dbusdisplay.h"
#include "dbus/dbusdock.h"
#include "dbus/dbusmonitor.h"
#define private public
#include <screen/screenmanager.h>
#include "screen/abstractscreenmanager_p.h"
#include <screen/screenobject.h>
#include <screen/screenhelper.h>
#include <screen/screenmanagerwayland.h>
#include <screen/screenobjectwayland.h>

using namespace testing;
namespace  {
    class ScreenManagerTest : public Test
    {
    public:
        ScreenManagerTest():Test()
        {

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

        }
        ~ScreenManagerTestWayland()
        {
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
        if (!find) EXPECT_TRUE(1) << "invaild screen" << obj->name().toStdString().c_str();
        else EXPECT_EQ(true,find);
    }
}

TEST(ScreenObject, geometry)
{
    ScreenManager sm;
    auto screenList = sm.logicScreens();
    ScreenManagerWayland smw;
    auto myScreens = smw.logicScreens();
    ASSERT_EQ(screenList.size(),myScreens.size());

    for (int i = 0; i < myScreens.size(); ++i){
        ScreenPointer obj = myScreens[i];
        ScreenPointer sc = screenList[i];
        if (!sc->name().isEmpty()) {
            EXPECT_EQ(sc->name(),obj->name());
            EXPECT_EQ(sc->availableGeometry(),obj->availableGeometry());
            EXPECT_EQ(sc->handleGeometry(),obj->handleGeometry());
        }
    }
    }

TEST(ScreenObject, primary_screen)
{
    ScreenManager sm;
    auto screen = sm.primaryScreen();
    ASSERT_NE(screen.data(), nullptr);
    if (qApp->primaryScreen() && !qApp->primaryScreen()->name().trimmed().isEmpty())
        EXPECT_EQ(qApp->primaryScreen()->name(), screen->name());

    ScreenManagerWayland smw;
    screen = smw.primaryScreen();
    ASSERT_NE(screen.data(), nullptr);
    if (qApp->primaryScreen() && !qApp->primaryScreen()->name().trimmed().isEmpty())
        EXPECT_EQ(qApp->primaryScreen()->name(), screen->name());
}

TEST(ScreenObject, display_mode)
{
    ScreenManager sm;
    auto getDisplayMode = []() -> AbstractScreenManager::DisplayMode{
        auto allScreen = qApp->screens();
        if (allScreen.isEmpty())
            return AbstractScreenManager::Custom;

        if (allScreen.size() == 1) {
            return AbstractScreenManager::Showonly;
        } else {
            //存在两个屏幕坐标不一样则视为扩展，只有所有屏幕坐标相等发生重叠时才视为复制
            QScreen *screen = allScreen.at(0);
            for (int i = 1; i < allScreen.size(); ++i) {
                QScreen *screen2 = allScreen.at(i);
                if (screen->geometry().topLeft() != screen2->geometry().topLeft()) {
                    return AbstractScreenManager::Extend;
                }
            }

            //所有屏幕的都重叠，则视为复制
            return AbstractScreenManager::Duplicate;
        }
    };

    EXPECT_EQ(getDisplayMode(), sm.displayMode());
}

TEST(ScreenObject, display_mode_wayland)
{
    ScreenManagerWayland smw;
    QDBusInterface inf("com.deepin.daemon.Display", "/com/deepin/daemon/Display", "com.deepin.daemon.Display");
    QList<QVariant> var = inf.call(QStringLiteral("GetRealDisplayMode")).arguments();
    auto value = qvariant_cast<uchar>(var.at(0));
    AbstractScreenManager::DisplayMode mode = static_cast<AbstractScreenManager::DisplayMode>(value);
    EXPECT_EQ(mode, smw.displayMode());
}

TEST(ScreenObject, device_pixel_ratio)
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

TEST_F(ScreenOperation, primaryScreen)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);
    EXPECT_EQ(dynamic_cast<ScreenObject *>(primary.data())->screen(),qApp->primaryScreen());
}

TEST_F(ScreenOperation, availableGeometry_hide)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 1;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);
    EXPECT_EQ(primary->availableGeometry(),primary->geometry());
}

TEST_F(ScreenOperation, availableGeometry_invaild)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 6;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObject*))(&ScreenObject::geometry);
    st.set(objgeo,geo_foo);

    QRect geo = geo_foo();
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperation, availableGeometry_top)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{0,0,1920,50};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObject*))(&ScreenObject::geometry);
    st.set(objgeo,geo_foo);

    auto hldgeo = (QRect (*)(ScreenObject*))(&ScreenObject::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 1.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    QRect geo = geo_foo();
    QRect dock = dockrect_foo().operator QRect();
    geo.setY(dock.bottom());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperation, availableGeometry_top_2)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{0,0,1920,50};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObject*))(&ScreenObject::geometry);
    st.set(objgeo,geo_foo);

    auto hldgeo = (QRect (*)(ScreenObject*))(&ScreenObject::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 2.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    qreal ratio = ratio_foo();
    QRect geo = geo_foo();

    QRect dock = dockrect_foo().operator QRect();
    dock = QRect(dock.x(),dock.y(), dock.width() / ratio,dock.height() / ratio);

    geo.setY(dock.bottom());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperation, availableGeometry_right)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 1;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{1900,0,20,1080};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObject*))(&ScreenObject::geometry);
    st.set(objgeo,geo_foo);
    auto hldgeo = (QRect (*)(ScreenObject*))(&ScreenObject::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 1.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    QRect geo = geo_foo();
    QRect dock = dockrect_foo().operator QRect();

    geo.setWidth(dock.left() - geo.left());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperation, availableGeometry_bottom)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 2;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{0,1000,1920,80};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObject*))(&ScreenObject::geometry);
    st.set(objgeo,geo_foo);
    auto hldgeo = (QRect (*)(ScreenObject*))(&ScreenObject::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 1.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    QRect geo = geo_foo();
    QRect dock = dockrect_foo().operator QRect();

    geo.setHeight(dock.top() - geo.top());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperation, availableGeometry_left)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 3;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{0,0,50,1080};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObject*))(&ScreenObject::geometry);
    st.set(objgeo,geo_foo);
    auto hldgeo = (QRect (*)(ScreenObject*))(&ScreenObject::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 1.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    QRect geo = geo_foo();
    QRect dock = dockrect_foo().operator QRect();

    geo.setX(dock.right());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperation, onScreenGeometryChanged)
{
    EXPECT_TRUE(d->m_events.isEmpty());
    this->onScreenGeometryChanged({});
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::Geometry);
}

TEST_F(ScreenOperation, onDockChanged)
{
    EXPECT_TRUE(d->m_events.isEmpty());
    this->onDockChanged();
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::AvailableGeometry);
}

TEST_F(ScreenOperation, onScreenAvailableGeometryChanged)
{
    EXPECT_TRUE(d->m_events.isEmpty());
    this->onScreenAvailableGeometryChanged({});
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::AvailableGeometry);
}

TEST_F(ScreenOperation, lastChangedMode)
{
    EXPECT_EQ(m_lastMode, this->lastChangedMode());
}

TEST_F(ScreenOperation, screen)
{
    auto qscreen = qApp->primaryScreen();
    auto screen = this->screen(qscreen->name());
    ASSERT_NE(screen.data(), nullptr);

    EXPECT_EQ(screen->name(), qscreen->name());
}

TEST_F(ScreenOperation, screen_invaild)
{
    auto screen = this->screen("dsadsa/ds/a/dsa/");
    EXPECT_EQ(screen.data(), nullptr);
}

namespace  {
class ScreenOperationWayland : public Test , public ScreenManagerWayland
{
public:
    ScreenOperationWayland() :
            Test()
          ,ScreenManagerWayland()
    {}
};
}
/**********************************************/
TEST_F(ScreenOperationWayland, primaryScreen)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    DBusMonitor m(dynamic_cast<ScreenObjectWayland *>(primary.data())->path());
    EXPECT_EQ(primary->name(), m.name());
    EXPECT_EQ(primary->handleGeometry(), m.rect());
}

TEST_F(ScreenOperationWayland, availableGeometry_hide)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 1;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);
    EXPECT_EQ(primary->availableGeometry(),primary->geometry());
}

TEST_F(ScreenOperationWayland, availableGeometry_invaild)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 6;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::geometry);
    st.set(objgeo,geo_foo);

    QRect geo = geo_foo();
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperationWayland, availableGeometry_top)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{0,0,1920,50};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::geometry);
    st.set(objgeo,geo_foo);
    auto hldgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 1.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    QRect geo = geo_foo();
    QRect dock = dockrect_foo().operator QRect();
    geo.setY(dock.bottom());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperationWayland, availableGeometry_top_2)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{0,0,1920,50};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::geometry);
    st.set(objgeo,geo_foo);
    auto hldgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 2.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    qreal ratio = ratio_foo();
    QRect geo = geo_foo();

    QRect dock = dockrect_foo().operator QRect();
    dock = QRect(dock.x(),dock.y(), dock.width() / ratio,dock.height() / ratio);

    geo.setY(dock.bottom());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperationWayland, availableGeometry_right)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 1;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{1900,0,20,1080};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::geometry);
    st.set(objgeo,geo_foo);
    auto hldgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 1.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    QRect geo = geo_foo();
    QRect dock = dockrect_foo().operator QRect();

    geo.setWidth(dock.left() - geo.left());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperationWayland, availableGeometry_bottom)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 2;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{0,1000,1920,80};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::geometry);
    st.set(objgeo,geo_foo);
    auto hldgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 1.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    QRect geo = geo_foo();
    QRect dock = dockrect_foo().operator QRect();

    geo.setHeight(dock.top() - geo.top());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperationWayland, availableGeometry_left)
{
    auto primary = primaryScreen();
    ASSERT_NE(primary.data(), nullptr);

    Stub st;
    auto hide_foo = (int(*)())([](){return 0;});
    st.set(ADDR(DBusDock,hideMode),hide_foo);

    auto pos_foo = (int(*)())([](){return 3;});
    st.set(ADDR(DBusDock,position),pos_foo);

    auto dockrect_foo = (DockRect(*)())([](){return DockRect{0,0,50,1080};});
    st.set(ADDR(DBusDock,frontendWindowRect),dockrect_foo);

    auto geo_foo = (QRect(*)())([](){return QRect(0,0,1920,1080);});
    auto objgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::geometry);
    st.set(objgeo,geo_foo);
    auto hldgeo = (QRect (*)(ScreenObjectWayland*))(&ScreenObjectWayland::handleGeometry);
    st.set(hldgeo,geo_foo);

    auto ratio_foo = (qreal(*)())([](){return 1.0;});
    st.set(ADDR(QScreen,devicePixelRatio),ratio_foo);

    QRect geo = geo_foo();
    QRect dock = dockrect_foo().operator QRect();

    geo.setX(dock.right());
    EXPECT_EQ(primary->availableGeometry(),geo);
}

TEST_F(ScreenOperationWayland, onMonitorChanged)
{
    m_screens.clear();
    onMonitorChanged();
    EXPECT_EQ(m_display->monitors().size(), screens().size());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::Screen);
}

TEST_F(ScreenOperationWayland, screen_reset)
{
    m_screens.clear();
    reset();
    EXPECT_EQ(m_display->monitors().size(), screens().size());
}

TEST_F(ScreenOperationWayland, onScreenGeometryChanged)
{
    EXPECT_TRUE(d->m_events.isEmpty());
    this->onScreenGeometryChanged({});
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::Geometry);
}

TEST_F(ScreenOperationWayland, onDockChanged)
{
    EXPECT_TRUE(d->m_events.isEmpty());
    this->onDockChanged();
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::AvailableGeometry);
}

TEST_F(ScreenOperationWayland, lastChangedMode)
{
    EXPECT_EQ(m_lastMode, this->lastChangedMode());
}

TEST_F(ScreenOperationWayland, screen)
{
    auto qscreen = m_display->primary();
    ASSERT_FALSE(qscreen.isEmpty());

    auto screen = this->screen(qscreen);
    ASSERT_NE(screen.data(), nullptr);

    EXPECT_EQ(screen->name(), qscreen);
    EXPECT_EQ(this->primaryScreen()->name(), qscreen);
}

TEST_F(ScreenOperationWayland, screen_invaild)
{
    auto screen = this->screen("dsadsa/ds/a/dsa/");
    EXPECT_EQ(screen.data(), nullptr);
}

/**********************************************/

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

TEST_F(ScreenSignal, process_displayModeChanged)
{
    m_lastMode = -1;
    EXPECT_TRUE(d->m_events.isEmpty());
    this->onScreenGeometryChanged({});
    this->processEvent();
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::Mode);
}

TEST_F(ScreenSignal, emit_PrimaryChanged)
{
    ASSERT_TRUE(d->m_events.isEmpty());
    m_lastMode = displayMode();

    emit m_display->PrimaryChanged();
    qApp->processEvents();
    if (qApp->screens().count() == 1) {
        ASSERT_TRUE(!d->m_events.isEmpty());
        EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::Screen);
    }
}

namespace  {
class ScreenSignalWayland : public Test , public ScreenManagerWayland
{
public:
    ScreenSignalWayland() :
            Test()
          ,ScreenManagerWayland()
    {}
};
}

TEST_F(ScreenSignalWayland, emit_PrimaryChanged)
{
    EXPECT_TRUE(d->m_events.isEmpty());
    emit m_display->PrimaryChanged();
    qApp->processEvents();
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::Screen);
}

TEST_F(ScreenSignalWayland, emit_DisplayModeChanged)
{
    EXPECT_TRUE(d->m_events.isEmpty());
    m_lastMode = m_display->GetRealDisplayMode();
    emit m_display->DisplayModeChanged();
    qApp->processEvents();
    ASSERT_TRUE(d->m_events.isEmpty());

    m_lastMode = -1;
    emit m_display->PrimaryRectChanged();
    qApp->processEvents();
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::Mode);
}

TEST_F(ScreenSignalWayland, emit_PrimaryRectChanged)
{
    EXPECT_TRUE(d->m_events.isEmpty());
    m_lastMode = m_display->GetRealDisplayMode();
    emit m_display->PrimaryRectChanged();
    qApp->processEvents();
    ASSERT_TRUE(d->m_events.isEmpty());

    m_lastMode = -1;
    emit m_display->PrimaryRectChanged();
    qApp->processEvents();
    ASSERT_FALSE(d->m_events.isEmpty());
    EXPECT_EQ(d->m_events.begin().key(),AbstractScreenManager::Mode);
}
