/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
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
#include <QTest>
#include <QScreen>
#include <DButtonBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <DIconButton>
#include <QLabel>
#include <QDBusAbstractInterface>
#include <DWindowManagerHelper>

#define private public
#define protected public

#include "../dde-wallpaper-chooser/button.h"
#include "../dde-wallpaper-chooser/checkbox.h"
#include "../dde-wallpaper-chooser/waititem.h"
#include "../dde-wallpaper-chooser/wallpaperlist.h"
#include "../dde-wallpaper-chooser/wallpaperitem.h"
#include "../dde-wallpaper-chooser/frame.h"
#include "../dde-wallpaper-chooser/dbus/deepin_wm.h"
#include "screen/screenhelper.h"
#include "desktopinfo.h"
#include "stub.h"
#include "stubext.h"
#include "appearance_interface.h"
#include "screensaver_interface.h"
#include "../dde-wallpaper-chooser/dbus/deepin_wm.h"
#include "../dde-wallpaper-chooser/thumbnailmanager.h"

using namespace testing;
using namespace stub_ext;

namespace {
    class FrameTest : public Test {
    public:
        FrameTest() : Test()
        {

        }

        static Frame *m_frame;
        virtual void SetUp() override;
    };

    Frame *FrameTest::m_frame = nullptr;

    void FrameTest::SetUp()
    {
        if (!m_frame)
            m_frame = new Frame(qApp->primaryScreen()->name());
    }
}

TEST(Frame, test_construct_frame)
{
    ASSERT_NE(qApp->primaryScreen(), nullptr);

    StubExt stub;
    stub.set_lamda(ADDR(DesktopInfo,waylandDectected),[](){return false;});
    const QString scName = qApp->primaryScreen()->name();

    Frame frame(scName);
    EXPECT_NE(frame.m_sessionManagerInter, nullptr);
    EXPECT_EQ(frame.m_mode, Frame::WallpaperMode);
    EXPECT_NE(frame.m_wallpaperList, nullptr);
    EXPECT_NE(frame.m_closeButton, nullptr);
    EXPECT_NE(frame.m_dbusWmInter, nullptr);
    EXPECT_NE(frame.m_dbusAppearance, nullptr);
    EXPECT_NE(frame.m_mouseArea, nullptr);
    EXPECT_EQ(frame.m_screenName, scName);
    EXPECT_NE(frame.m_wallpaperCarouselCheckBox, nullptr);
    EXPECT_NE(frame.m_wallpaperCarouselControl, nullptr);

    EXPECT_NE(frame.m_waitControl, nullptr);
    EXPECT_NE(frame.m_lockScreenBox, nullptr);
    EXPECT_NE(frame.m_dbusScreenSaver, nullptr);
    EXPECT_NE(frame.m_waitControl, nullptr);
    EXPECT_NE(frame.m_switchModeControl, nullptr);

    EXPECT_NE(frame.m_itemwait, nullptr);
}

TEST(Frame, test_construct_frame_wayland)
{
    StubExt stub;
    stub.set_lamda(ADDR(DesktopInfo,waylandDectected),[](){return true;});
    Frame frame(qApp->primaryScreen()->name());
    EXPECT_NE(frame.windowHandle(), nullptr);
    EXPECT_EQ(frame.windowHandle()->property("_d_dwayland_window-type").toString(),
              QString("wallpaper-set"));
}

//! 为提升ut性能，复用Frame对象，下面的用例子执行必须保证开始和结束的状态，否则会导致用例失败
//! 以下用例以流水线进行，修改或调整，新增，删除下列用例可能导致失败，注意修复
//! 流水用例开始
TEST_F(FrameTest, test_setwallpaperslideshow)
{
    ASSERT_NE(m_frame->m_dbusAppearance, nullptr);
    QString temp("30");
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(ComDeepinDaemonAppearanceInterface, SetWallpaperSlideShow),
                  [&judge](){judge = true; return QDBusPendingReply<QString>();});

    m_frame->setWallpaperSlideShow(temp);
    EXPECT_TRUE(judge);

    auto tmp = m_frame->m_dbusAppearance;
    m_frame->m_dbusAppearance = nullptr;
    judge = false;

    m_frame->setWallpaperSlideShow(temp);
    EXPECT_FALSE(judge);

    m_frame->m_dbusAppearance = tmp;
}

TEST_F(FrameTest, test_applyToDesktop)
{
    ASSERT_NE(m_frame->m_dbusAppearance, nullptr);
    auto ctmp = m_frame->m_cureentWallpaper;

    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(ComDeepinDaemonAppearanceInterface, SetMonitorBackground),
                  [&judge](){judge = true; return QDBusPendingReply<QString>();});

    m_frame->m_cureentWallpaper.clear();
    m_frame->applyToDesktop();
    EXPECT_FALSE(judge);

    judge = false;
    m_frame->m_cureentWallpaper = "test";
    m_frame->applyToDesktop();
    EXPECT_TRUE(judge);

    judge = false;
    auto tmp = m_frame->m_dbusAppearance;
    m_frame->m_dbusAppearance = nullptr;

    m_frame->applyToDesktop();
    EXPECT_FALSE(judge);

    m_frame->m_dbusAppearance = tmp;
    m_frame->m_cureentWallpaper = ctmp;
}

TEST_F(FrameTest, test_applyToGreeter)
{
    ASSERT_NE(m_frame->m_dbusAppearance, nullptr);
    auto ctmp = m_frame->m_cureentWallpaper;

    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(ComDeepinDaemonAppearanceInterface, Set),
                  [&judge](){judge = true; return QDBusPendingReply<>();});

    m_frame->m_cureentWallpaper.clear();
    m_frame->applyToGreeter();
    EXPECT_FALSE(judge);

    judge = false;
    m_frame->m_cureentWallpaper = "test";
    m_frame->applyToGreeter();
    EXPECT_TRUE(judge);

    judge = false;
    auto tmp = m_frame->m_dbusAppearance;
    m_frame->m_dbusAppearance = nullptr;

    m_frame->applyToGreeter();
    EXPECT_FALSE(judge);

    m_frame->m_dbusAppearance = tmp;
    m_frame->m_cureentWallpaper = ctmp;
}

TEST_F(FrameTest, test_getwallpaperslideshow)
{
    ASSERT_NE(m_frame->m_dbusAppearance, nullptr);

    stub_ext::StubExt stu;
    bool called = false;
    stu.set_lamda(ADDR(ComDeepinDaemonAppearanceInterface, GetWallpaperSlideShow), [&called](){called = true; return QDBusPendingReply<QString>();});

    m_frame->getWallpaperSlideShow();
    EXPECT_TRUE(called);

    auto tmp = m_frame->m_dbusAppearance;
    m_frame->m_dbusAppearance = nullptr;
    called = false;

    m_frame->getWallpaperSlideShow();
    EXPECT_FALSE(called);

    m_frame->m_dbusAppearance = tmp;
}

TEST_F(FrameTest, test_initsize)
{
    m_frame->initSize();

    QRect rect = qApp->primaryScreen()->geometry();
    QSize listsize  = QSize(rect.width() - 20, 100);
    QSize framesize = QSize(rect.width() - 20, 175);

    EXPECT_EQ(listsize, m_frame->m_wallpaperList->size());
    EXPECT_EQ(framesize, m_frame->size());
}

TEST_F(FrameTest, test_reLayoutTools)
{
    m_frame->m_mode = Frame::ScreenSaverMode;
    int count = m_frame->layout()->count();
    m_frame->reLayoutTools();

    EXPECT_TRUE(!m_frame->m_waitControlLabel->isHidden());
    EXPECT_TRUE(!m_frame->m_waitControl->isHidden());
    EXPECT_TRUE(!m_frame->m_lockScreenBox->isHidden());

    m_frame->m_mode = Frame::WallpaperMode;
    m_frame->reLayoutTools();

    EXPECT_TRUE(m_frame->m_waitControlLabel->isHidden());
    EXPECT_TRUE(m_frame->m_waitControl->isHidden());
    EXPECT_TRUE(m_frame->m_lockScreenBox->isHidden());
    EXPECT_EQ(m_frame->layout()->count(), count);
}

TEST_F(FrameTest, test_frame_show)
{
    ASSERT_NE(m_frame, nullptr);
    m_frame->show();
    bool ok = false;

    StubExt stub;
    stub.set_lamda(ADDR(Frame, refreshList),[&ok](){ok = true;});
    QTest::qWaitFor([&ok]{return ok;});
    EXPECT_NE(m_frame->m_backgroundManager, nullptr);
    EXPECT_EQ(m_frame->isVisible(), true);
}

TEST_F(FrameTest, test_frame_hide)
{
    ASSERT_NE(m_frame, nullptr);
    ASSERT_TRUE(m_frame->isVisible());

    auto tmp = m_frame->m_cureentWallpaper;
    m_frame->m_cureentWallpaper = "test";
    bool desktop = false;
    bool greeter = false;
    bool setSs = false;
    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(Frame, applyToDesktop),
                   [&desktop](){desktop = true;});
    stub.set_lamda(ADDR(Frame, applyToGreeter),
                   [&greeter](){greeter = true;});
    stub.set_lamda(ADDR(ComDeepinScreenSaverInterface, setCurrentScreenSaver),
                   [&setSs](){setSs = true;});

    m_frame->hide();

    EXPECT_FALSE(desktop);
    EXPECT_FALSE(greeter);
    EXPECT_FALSE(setSs);

    m_frame->m_cureentWallpaper = tmp;
    EXPECT_FALSE(m_frame->isVisible());
    EXPECT_EQ(m_frame->m_backgroundManager, nullptr);
    EXPECT_EQ(m_frame->m_dbusDeepinWM, nullptr);
}

TEST_F(FrameTest, test_setmode_screensaver)
{
    ASSERT_NE(m_frame->m_mode, Frame::ScreenSaverMode);
    ASSERT_NE(m_frame->m_dbusScreenSaver, nullptr);

    StubExt stub;
    bool ok = false;
    stub.set_lamda(ADDR(Frame, refreshList),[&ok](){ok = true;});

    m_frame->setMode(Frame::ScreenSaverMode);

    EXPECT_EQ(m_frame->m_mode, Frame::ScreenSaverMode);
    EXPECT_TRUE(ok);

    bool show = false;
    stub.set_lamda(VADDR(Frame, showEvent),[&show](Frame *f,QShowEvent *event){
        show = true;
        f->DBlurEffectWidget::showEvent(event);
    });
    m_frame->show();
    EXPECT_TRUE(show);
}

TEST_F(FrameTest, test_refreshlist_screensaver)
{
    ASSERT_NE(m_frame->m_itemwait, nullptr);
    ASSERT_FALSE(m_frame->m_loadTimer.isActive());
    ASSERT_EQ(m_frame->m_mode, Frame::ScreenSaverMode);

    StubExt stub;
    stub.set_lamda(ADDR(WallpaperItem,pressed),[](){});

    m_frame->refreshList();

    EXPECT_FALSE(m_frame->m_loadTimer.isActive());
    EXPECT_EQ(m_frame->m_itemwait, nullptr);
    EXPECT_TRUE(m_frame->m_wallpaperList->count() > 0);

    //test select default item
    int currentIndex = -1;
    stub.set_lamda(ADDR(WallpaperList, setCurrentIndex), [&currentIndex](WallpaperList *obj, int index){
        Q_UNUSED(obj)
        currentIndex = index;
    });
    QString currentScreenSaver = m_frame->m_dbusScreenSaver->currentScreenSaver();
    m_frame->m_dbusScreenSaver->setCurrentScreenSaver(QString("noneexistitem"));
    QString tstScreenSaver = m_frame->m_dbusScreenSaver->currentScreenSaver();
    ASSERT_NE(currentScreenSaver, tstScreenSaver);

    m_frame->refreshList();
    EXPECT_EQ(currentIndex, 0);

    m_frame->m_dbusScreenSaver->setCurrentScreenSaver(currentScreenSaver);
}

TEST_F(FrameTest, test_setmode_wallpaper)
{
    ASSERT_NE(m_frame->m_mode, Frame::WallpaperMode);
    ASSERT_NE(m_frame->m_dbusScreenSaver, nullptr);

    StubExt stub;
    bool ok = false;
    stub.set_lamda(ADDR(Frame, refreshList),[&ok](){ok = true;});

    m_frame->setMode(Frame::WallpaperMode);

    EXPECT_EQ(m_frame->m_mode, Frame::WallpaperMode);
    EXPECT_TRUE(ok);
}

TEST_F(FrameTest, test_refreshlist_wallpaper)
{
    ASSERT_EQ(m_frame->m_mode, Frame::WallpaperMode);
    ASSERT_TRUE(m_frame->isVisible());
    ASSERT_EQ(m_frame->m_itemwait, nullptr);

    m_frame->refreshList();
    ASSERT_NE(m_frame->m_itemwait, nullptr);

    QTest::qWaitFor([m_frame]{return (m_frame->m_itemwait == nullptr)
                || (m_frame->m_loadTimer.isActive());}
                , 3000);

    if (m_frame->m_itemwait == nullptr) {
        EXPECT_FALSE(m_frame->m_loadTimer.isActive());
        EXPECT_GE(m_frame->m_wallpaperList->count(), 0);
    } else {
        EXPECT_TRUE(m_frame->m_loadTimer.isActive());
        m_frame->m_loadTimer.stop();
    }
}

TEST_F(FrameTest, test_handleNeedclosebutton)
{
    ASSERT_NE(m_frame->m_backgroundManager, nullptr);
    if (m_frame->m_wallpaperList->count() < 1)
        return;

    ASSERT_EQ(m_frame->m_mode, Frame::WallpaperMode);

    WallpaperItem* item = m_frame->m_wallpaperList->m_items.first();
    QPoint pos = item->pos();
    auto currentPath = QString("test");
    m_frame->handleNeedCloseButton(currentPath, pos);
    EXPECT_TRUE(m_frame->m_closeButton->isVisible());

    m_frame->m_closeButton->click();
    EXPECT_FALSE(m_frame->m_needDeleteList.isEmpty());
    EXPECT_FALSE(m_frame->m_closeButton->isVisible());
}

TEST_F(FrameTest, test_onitemispressed)
{
    ASSERT_NE(m_frame->m_backgroundManager, nullptr);
    if (m_frame->m_wallpaperList->count() < 1)
        return;
    ASSERT_EQ(m_frame->m_mode, Frame::WallpaperMode);

    StubExt stub;
    stub.set_lamda(ADDR(BackgroundManager, onResetBackgroundImage),
                   [](){});
    int is_set = false;
    stub.set_lamda(ADDR(BackgroundManager, setBackgroundImage),
                   [&is_set](){is_set = true;});
    QString data = m_frame->m_wallpaperList->m_items.at(0)->getPath();
    m_frame->onItemPressed(data);
    EXPECT_TRUE(is_set);
    EXPECT_EQ(m_frame->m_cureentWallpaper, data);

    m_frame->m_mode = Frame::ScreenSaverMode;
    bool preview = false;
    stub.set_lamda(ADDR(ComDeepinScreenSaverInterface, Preview),
                   [&preview](){ preview = true;return QDBusPendingReply<>();});

    stub.set_lamda(ADDR(BackgroundManager, setVisible),
                   [](){return;});
    m_frame->onItemPressed("test");
    EXPECT_TRUE(preview);

    m_frame->m_mode = Frame::WallpaperMode;
}

//donot need showing
TEST_F(FrameTest, test_desktopbackground)
{
    QPair<QString, QString> pair = m_frame->desktopBackground();
    QPair<QString, QString> compare = QPair<QString, QString>(m_frame->m_screenName, m_frame->m_cureentWallpaper);

    EXPECT_EQ(pair, compare);
}

TEST_F(FrameTest, test_screenGeometryChanged)
{
    bool sizeChanged = false;
    StubExt stub;
    ASSERT_NE(m_frame->m_wallpaperList, nullptr);

    stub.set_lamda(ADDR(Frame,initSize),
                   [&sizeChanged](){sizeChanged = true;});
    stub.set_lamda(ADDR(Frame,activateWindow),
                   [](){});
    stub.set_lamda(ADDR(WallpaperList,updateItemThumb),
                   [](){});

    ScreenMrg->sigScreenGeometryChanged();
    EXPECT_TRUE(sizeChanged);
}

TEST_F(FrameTest, test_area_button_press)
{
    ASSERT_NE(m_frame->m_wallpaperList, nullptr);
    ASSERT_NE(m_frame->m_mouseArea, nullptr);

    int ret = -1;
    StubExt stub;
    stub.set_lamda(ADDR(WallpaperList, prevPage),
                   [&ret](){ret = 4;});
    stub.set_lamda(ADDR(WallpaperList, nextPage),
                   [&ret](){ret = 5;});

    m_frame->m_mouseArea->buttonPress(QPoint(0,0),4);
    EXPECT_EQ(ret, 4);

    ret = -1;
    m_frame->m_mouseArea->buttonPress(QPoint(0,0), 5);
    EXPECT_EQ(ret, 5);

    stub.set_lamda(ADDR(Frame, hide),
                   [](){});
    ret = -1;
    m_frame->m_mouseArea->buttonPress(QPoint(0,0), 1);
    EXPECT_EQ(ret, -1);
}

TEST_F(FrameTest, test_loading)
{
    if (m_frame->m_itemwait) {
        delete m_frame->m_itemwait;
        m_frame->m_itemwait = nullptr;
    }

    ASSERT_EQ(m_frame->m_itemwait, nullptr);
    m_frame->loading();

    QSize size = m_frame->m_itemwait->size();
    EXPECT_NE(m_frame->m_itemwait, nullptr);
    EXPECT_EQ(size, m_frame->m_wallpaperList->size());
}

TEST_F(FrameTest, test_onitembuttonisclicked)
{
    const QString name = "test";
    m_frame->m_cureentWallpaper = name;
    StubExt stub;
    bool callHide = false;
    stub.set_lamda(ADDR(Frame, hide),
                   [&callHide](){callHide = true;});

    WallpaperItem *item = nullptr;
    stub.set_lamda(ADDR(Frame, sender),
                   [&item]()->QObject *{return item;});

    m_frame->onItemButtonClicked("");
    EXPECT_FALSE(callHide);

    WallpaperItem wl;
    item = &wl;

    bool desktop = false;
    bool greeter = false;
    bool setSs = false;
    stub.set_lamda(ADDR(Frame, applyToDesktop),
                   [&desktop](){desktop = true;});
    stub.set_lamda(ADDR(Frame, applyToGreeter),
                   [&greeter](){greeter = true;});
    stub.set_lamda(ADDR(ComDeepinScreenSaverInterface, setCurrentScreenSaver),
                   [&setSs](){setSs = true;});
    callHide = false;
    m_frame->onItemButtonClicked("desktop");
    EXPECT_EQ(m_frame->m_cureentWallpaper, name);
    EXPECT_TRUE(callHide);
    EXPECT_TRUE(desktop);
    EXPECT_FALSE(greeter);
    EXPECT_FALSE(setSs);

    callHide = false;
    desktop = false;
    greeter = false;
    setSs = false;
    m_frame->onItemButtonClicked("lock-screen");
    EXPECT_EQ(m_frame->m_cureentWallpaper, name);
    EXPECT_TRUE(callHide);
    EXPECT_TRUE(greeter);
    EXPECT_FALSE(desktop);
    EXPECT_FALSE(setSs);

    callHide = false;
    desktop = false;
    greeter = false;
    setSs = false;
    m_frame->onItemButtonClicked("desktop-lockscreen");
    EXPECT_EQ(m_frame->m_cureentWallpaper, name);
    EXPECT_TRUE(callHide);
    EXPECT_TRUE(desktop);
    EXPECT_TRUE(greeter);
    EXPECT_FALSE(setSs);

    callHide = false;
    desktop = false;
    greeter = false;
    setSs = false;
    m_frame->onItemButtonClicked("screensaver");
    EXPECT_TRUE(setSs);
    EXPECT_TRUE(callHide);
    EXPECT_FALSE(greeter);
    EXPECT_FALSE(desktop);
}

TEST_F(FrameTest, test_keypressevent)
{
    ASSERT_EQ(m_frame->m_mode, Frame::WallpaperMode);
    StubExt stub;
    bool hidden = false;
    stub.set_lamda(ADDR(Frame, hide),
                   [&hidden](){hidden = true;});
    QTest::keyPress(m_frame, Qt::Key_Escape, Qt::NoModifier);

    QTest::qWaitFor([&hidden](){return hidden == true;}, 100);
    EXPECT_TRUE(hidden);

    hidden = false;
    QTest::keyPress(m_frame, Qt::Key_Right, Qt::NoModifier);
    QTest::keyPress(m_frame, Qt::Key_Right, Qt::NoModifier);


    QTest::keyPress(m_frame, Qt::Key_Left, Qt::NoModifier);
    QTest::keyPress(m_frame, Qt::Key_Left, Qt::NoModifier);

    QTest::keyPress(m_frame, Qt::Key_Space, Qt::NoModifier);

    hidden = false;
    m_frame->m_mode = Frame::ScreenSaverMode;
    QTest::keyPress(m_frame, Qt::Key_Escape, Qt::NoModifier);

    QTest::qWaitFor([&hidden](){return hidden == true;}, 100);
    EXPECT_TRUE(hidden);

    m_frame->m_mode = Frame::WallpaperMode;
}

TEST_F(FrameTest, test_eventfilter)
{
    QKeyEvent key1(QEvent::KeyPress,Qt::Key_Tab,Qt::NoModifier);
    m_frame->eventFilter(m_frame->m_wallpaperCarouselCheckBox, &key1);
    m_frame->eventFilter(m_frame->m_switchModeControl, &key1);

    QKeyEvent key2(QEvent::KeyPress,Qt::Key_Backtab,Qt::NoModifier);
    m_frame->eventFilter(m_frame->m_wallpaperCarouselCheckBox, &key2);
    m_frame->eventFilter(m_frame->m_switchModeControl, &key2);

    QKeyEvent key3(QEvent::KeyPress,Qt::Key_Up,Qt::NoModifier);
    m_frame->eventFilter(m_frame->m_wallpaperCarouselCheckBox, &key3);

    QKeyEvent key4(QEvent::KeyPress,Qt::Key_Down,Qt::NoModifier);
    m_frame->eventFilter(m_frame->m_wallpaperCarouselCheckBox, &key4);

    QKeyEvent key5(QEvent::KeyPress,Qt::Key_Right,Qt::NoModifier);
    m_frame->eventFilter(m_frame->m_wallpaperCarouselCheckBox, &key5);
    m_frame->eventFilter(m_frame, &key5);
}

TEST_F(FrameTest, test_endAndFree)
{
    if (m_frame) {
        delete m_frame;
        m_frame = nullptr;
    }

    EXPECT_EQ(m_frame, nullptr);
}
//! 流水用例结束


TEST(Frame, test_onrest)
{
    Frame frame(qApp->primaryScreen()->name());
    ASSERT_EQ(frame.m_backgroundManager, nullptr);

    StubExt stub;
    bool closed = false;
    stub.set_lamda(ADDR(Frame,close),[&closed](){closed = true;return false;});
    frame.onRest();
    EXPECT_TRUE(closed);

    closed = false;
    frame.m_backgroundManager = new BackgroundManager(true);
    frame.onRest();
    EXPECT_FALSE(closed);
    delete frame.m_backgroundManager;
    frame.m_backgroundManager = nullptr;
}



TEST(Frame, test_show)
{
    void(*region)(void*) = [](void* obj){};
    stub_ext::StubExt stu;
    stub_ext::StubExt st;
    stu.set_lamda(ADDR(DBlurEffectWidget, show), [](){return;});
    stu.set_lamda(ADDR(DWindowManagerHelper, windowManagerName), [](){return DWindowManagerHelper::DeepinWM;});
    Frame frame(qApp->primaryScreen()->name());
    ASSERT_EQ(frame.m_backgroundManager, nullptr);

    stu.set_lamda(ADDR(BackgroundManager, init), [](){return;});
    stu.set_lamda(ADDR(DeepinWM, RequestHideWindows), [](){return QDBusPendingReply<>();});
    typedef void (*fptr)(DRegionMonitor*, int);
    fptr A_foo = (fptr)((void(DRegionMonitor::*)())&DRegionMonitor::registerRegion);
    st.set(A_foo, region);
    frame.m_backgroundManager = new BackgroundManager;
    frame.show();
    ASSERT_TRUE(frame.m_dbusDeepinWM != nullptr);

    stu.reset(ADDR(DWindowManagerHelper, windowManagerName));
    frame.show();
    ASSERT_TRUE(frame.m_dbusDeepinWM == nullptr);

    stu.set_lamda(ADDR(DWindowManagerHelper, windowManagerName), [](){return DWindowManagerHelper::DeepinWM;});
    frame.show();
    ASSERT_TRUE(frame.m_backgroundManager == nullptr);
}

TEST(Frame, test_hideEvent)
{
    stub_ext::StubExt stu;
    Frame frame(qApp->primaryScreen()->name());
    QHideEvent event;
    stu.set_lamda(ADDR(ThumbnailManager, instance), [](){return nullptr;});
    stu.set_lamda(ADDR(ThumbnailManager, stop), [](){return;});

    bool ishide = false;
    QObject::connect(&frame, &Frame::done, &frame,[&ishide](){ ishide = true;});
    frame.hideEvent(&event);
    QTest::qWaitFor([&ishide](){return ishide == true;}, 100);
    ASSERT_TRUE(ishide);
}

TEST(Frame, test_keyevent)
{
    stub_ext::StubExt stu;
    Frame frame(qApp->primaryScreen()->name());
    frame.m_mode = Frame::ScreenSaverMode;

    bool ishide = false;
    stu.set_lamda(ADDR(Frame, hide), [&ishide](){ishide = true; return;});
    QTest::keyClick(&frame, Qt::Key_Left);
    QTest::keyClick(&frame, Qt::Key_0);
    QTest::keyClick(&frame, Qt::Key_Escape);
    QTest::qWaitFor([&ishide](){return ishide == true;}, 100);
    EXPECT_TRUE(ishide);
}

TEST(Frame, test_setmode)
{
    stub_ext::StubExt stu;
    Frame frame(qApp->primaryScreen()->name());
    QAbstractButton* btn = new QPushButton;

    stu.set_lamda(ADDR(DButtonBox, buttonList), [btn](){return QList<QAbstractButton*>() << btn;});
    frame.setMode(btn, true);
    delete btn;
    btn = nullptr;
}
