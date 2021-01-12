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
#include "../third-party/cpp-stub/stub.h"
#include "../stub-ext/stubext.h"
#include "appearance_interface.h"

using namespace testing;

namespace {
    class FrameTest : public Test {
    public:
        FrameTest() : Test()
        {

        }

        virtual void SetUp() override
        {
            m_frame = new Frame(qApp->primaryScreen()->name());
        }

        virtual void TearDown() override
        {
            delete m_frame;
        }

        Frame *m_frame = nullptr;
    };
}

TEST_F(FrameTest, test_frame)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    m_frame->show();

    m_frame->m_wallpaperList->setCurrentIndex(0);
    qApp->processEvents();

    //模拟翻页，看显示情况
     qApp->processEvents();
     if (m_frame->m_wallpaperList->nextButton) {
         QVariant firstvalue = m_frame->m_wallpaperList->scrollAnimation.startValue();
         this->m_frame->m_mouseArea->buttonPress(QPoint(0, 0), 5);
         QVariant secondvalue = m_frame->m_wallpaperList->scrollAnimation.startValue();
         if (firstvalue == secondvalue) {
                QVariant endvalue1 = m_frame->m_wallpaperList->scrollAnimation.endValue();
                QVariant endvalue = m_frame->m_wallpaperList->scrollAnimation.endValue();
                EXPECT_EQ(endvalue1, endvalue);
         }
     }

     qApp->processEvents();
     if (m_frame->m_wallpaperList->prevButton) {
        QVariant firstvalue = m_frame->m_wallpaperList->scrollAnimation.startValue();
        this->m_frame->m_mouseArea->buttonPress(QPoint(0, 0), 4);
        QVariant secondvalue = m_frame->m_wallpaperList->scrollAnimation.startValue();
        if (firstvalue == secondvalue) {
                QVariant endvalue1 = m_frame->m_wallpaperList->scrollAnimation.endValue();
                QVariant endvalue = m_frame->m_wallpaperList->scrollAnimation.endValue();
                EXPECT_EQ(endvalue1, endvalue);
        }
    }


    this->m_frame->m_mouseArea->buttonPress(QPoint(0, 0), 1);


    EXPECT_TRUE(m_frame->isTopLevel());
    m_frame->hide();
}

TEST_F(FrameTest, test_show)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    m_frame->show();
    bool status = false;

    if (!m_frame->isHidden()) {
        status = true;
    }

    EXPECT_NE(m_frame->m_backgroundManager, nullptr);
    EXPECT_TRUE(status);
    m_frame->hide();
}


TEST_F(FrameTest, test_hide)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    m_frame->show();
    m_frame->hide();

    bool status = false;
    if (m_frame->isHidden()) {
        status = true;
    }
    EXPECT_TRUE(status);
}

TEST_F(FrameTest, test_onrest)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    WallpaperItem* item = new WallpaperItem;
    m_frame->m_wallpaperList->m_items << item;
    m_frame->m_backgroundManager = new BackgroundManager(true);
    m_frame->onRest();
    ScreenPointer testscreen = ScreenMrg->screen(m_frame->m_screenName);
    BackgroundWidgetPointer pointer = m_frame->m_backgroundManager->backgroundWidget(testscreen);
    if (pointer) {
       QWidget* widget = m_frame->topLevelWidget();
       EXPECT_NE(widget, pointer);
    }

    EXPECT_TRUE(m_frame->isTopLevel());
    delete item;
    delete m_frame->m_backgroundManager;
}

TEST_F(FrameTest, test_refreshlist)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    WallpaperItem* item1 = new WallpaperItem;
    m_frame->m_wallpaperList->m_items << item1;
    m_frame->m_backgroundManager = new BackgroundManager(true);
    m_frame->m_mode = Frame::ScreenSaverMode;
    QString currentPath = QString(m_frame->m_backgroundManager->backgroundImages().value(m_frame->m_screenName));
    WallpaperItem* item = m_frame->m_wallpaperList->addWallpaper(currentPath);
    Button* btn = dynamic_cast<Button*>(item->m_buttonLayout->itemAt(0));
    WallpaperItem* itemsingnal = m_frame->m_wallpaperList->getCurrentItem();
    m_frame->m_mode = Frame::WallpaperMode;
    emit itemsingnal->tab();

    EXPECT_NE(m_frame->m_wallpaperList->count(), 0);
    delete item1;
    delete m_frame->m_backgroundManager;
}

TEST_F(FrameTest, test_onitemispressed)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    WallpaperItem* item = new WallpaperItem;
    m_frame->m_wallpaperList->m_items << item;
    m_frame->m_backgroundManager = new BackgroundManager(true);

    QString data = QString("/usr/share/backgrounds/default_background.jpg");
    item->setData(data);

    m_frame->onItemPressed(data);
    QMap<QString, QString> path = m_frame->m_backgroundManager->backgroundImages();
    QString backgroundpath = path[qApp->primaryScreen()->name()];

    EXPECT_EQ(data, backgroundpath);
    EXPECT_EQ(m_frame->m_desktopWallpaper, m_frame->m_lockWallpaper);

    delete item;
    delete m_frame->m_backgroundManager;
}

TEST_F(FrameTest, test_onitembuttonisclicked)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    m_frame->m_wallpaperList->addItem(new WallpaperItem);
    m_frame->m_wallpaperList->addItem(new WallpaperItem);
    m_frame->m_wallpaperList->setCurrentIndex(1);

    auto item = m_frame->m_wallpaperList->getCurrentItem();
    emit item->buttonClicked("desktop");
    EXPECT_TRUE(m_frame->m_lockWallpaper.isEmpty());

    emit item->buttonClicked("lock-screen");
    EXPECT_TRUE(m_frame->m_desktopWallpaper.isEmpty());
}

TEST_F(FrameTest, test_setmode)
{
   ASSERT_EQ(m_frame->m_desktopWallpaper, nullptr);
   Frame::Mode m = m_frame->m_mode;

   m_frame->m_mode = Frame::ScreenSaverMode;
   m_frame->setMode(Frame::WallpaperMode);

   EXPECT_EQ(m, m_frame->m_mode);
}

TEST_F(FrameTest, test_loading)
{
    ASSERT_NE(m_frame->m_itemwait, nullptr);
    WaitItem* temp = m_frame->m_itemwait;

    m_frame->loading();
    QSize size = m_frame->m_itemwait->size();

    EXPECT_EQ(temp, m_frame->m_itemwait);
    EXPECT_EQ(size, m_frame->m_wallpaperList->size());
}

TEST_F(FrameTest, test_keypressevent)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    WallpaperItem* temp = new WallpaperItem;
    m_frame->m_wallpaperList->m_items << temp;

    qApp->processEvents();
    //触发事件看显示结果
    QKeyEvent* eventesc = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QKeyEvent* eventright = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent* eventleft = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);

    if (m_frame->m_wallpaperList->m_index != m_frame->m_wallpaperList->count() - 1) {
        m_frame->m_wallpaperCarouselControl->setVisible(true);
        WallpaperItem* firstitem = m_frame->m_wallpaperList->getCurrentItem();
        m_frame->keyPressEvent(eventright);
        m_frame->keyPressEvent(eventright);
        EXPECT_FALSE(firstitem->hasFocus());
    }
    else {
        m_frame->m_wallpaperCarouselControl->setVisible(true);
        WallpaperItem* firstitem = m_frame->m_wallpaperList->getCurrentItem();
        m_frame->keyPressEvent(eventright);
        m_frame->keyPressEvent(eventright);
        EXPECT_FALSE(firstitem->hasFocus());
    }

    qApp->processEvents();
    if (m_frame->m_wallpaperList->m_index != 0) {
            m_frame->m_wallpaperCarouselControl->setVisible(true);
            WallpaperItem* firstitem = m_frame->m_wallpaperList->getCurrentItem();
            m_frame->keyPressEvent(eventleft);
            EXPECT_FALSE(firstitem->hasFocus());
    }
    else {
            m_frame->m_wallpaperCarouselControl->setVisible(true);
            WallpaperItem* firstitem = m_frame->m_wallpaperList->getCurrentItem();
            m_frame->keyPressEvent(eventleft);
            m_frame->keyPressEvent(eventleft);
            EXPECT_FALSE(firstitem->hasFocus());
    }
    qApp->processEvents();
    m_frame->show();
    m_frame->setMode(Frame::ScreenSaverMode);
    m_frame->keyPressEvent(eventesc);
    qApp->processEvents();
    EXPECT_TRUE(m_frame->isHidden());

    delete eventesc;
    delete eventright;
    delete eventleft;
    delete temp;
}

TEST_F(FrameTest, test_eventfilter)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    m_frame->show();

    qApp->processEvents();

    QKeyEvent* eventtab = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QKeyEvent* eventbacktab = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    QKeyEvent* eventkeyright = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);

    QCoreApplication::sendEvent(m_frame,static_cast<QEvent*>(eventtab));
    QCoreApplication::sendEvent(m_frame->m_lockScreenBox,static_cast<QEvent*>(eventtab));
    QCoreApplication::sendEvent(m_frame->m_waitControl->buttonList().at(0),static_cast<QEvent*>(eventtab));
    EXPECT_TRUE(m_frame->m_switchModeControl->buttonList().at(0)->hasFocus());

    qApp->processEvents();
    QCoreApplication::sendEvent(m_frame->m_wallpaperCarouselCheckBox,static_cast<QEvent*>(eventbacktab));
    EXPECT_TRUE(m_frame->m_wallpaperList->getCurrentItem()->findChildren<Button*>().at(0)->hasFocus());

    qApp->processEvents();
    m_frame->m_mode = Frame::WallpaperMode;
    QCoreApplication::sendEvent(m_frame->m_switchModeControl->buttonList().first(),static_cast<QEvent*>(eventbacktab));
    EXPECT_TRUE(m_frame->m_wallpaperCarouselCheckBox->hasFocus());

    qApp->processEvents();
    m_frame->m_mode = Frame::ScreenSaverMode;
    QCoreApplication::sendEvent(m_frame->m_switchModeControl->buttonList().first(),static_cast<QEvent*>(eventbacktab));
    EXPECT_TRUE(m_frame->m_waitControl->buttonList().first()->hasFocus());

    if (m_frame->m_wallpaperList->m_index != m_frame->m_wallpaperList->count() -1) {
            m_frame->m_wallpaperCarouselControl->setVisible(true);
            WallpaperItem* itemfirst = m_frame->m_wallpaperList->getCurrentItem();
            QCoreApplication::sendEvent(m_frame->m_wallpaperCarouselCheckBox,static_cast<QEvent*>(eventkeyright));
            EXPECT_FALSE(itemfirst->hasFocus());
    }
    else {
            m_frame->m_wallpaperCarouselControl->setVisible(true);
            WallpaperItem* itemfirst = m_frame->m_wallpaperList->getCurrentItem();
            QCoreApplication::sendEvent(m_frame->m_wallpaperCarouselCheckBox,static_cast<QEvent*>(eventkeyright));
            EXPECT_FALSE(itemfirst->hasFocus());
    }
    qApp->processEvents();

    delete eventtab;
    delete eventbacktab;
    delete eventkeyright;
    m_frame->hide();
}

TEST_F(FrameTest, test_initsize)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    m_frame->initSize();

    QRect rect = qApp->primaryScreen()->geometry();
    QSize listsize  = QSize(rect.width() - 20, 100);
    QSize framesize = QSize(rect.width() - 20, 175);

    EXPECT_EQ(listsize, m_frame->m_wallpaperList->size());
    EXPECT_EQ(framesize, m_frame->size());
}

TEST_F(FrameTest, test_handleNeedclosebutton)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    m_frame->m_backgroundManager = new BackgroundManager(true);
    QString currentPath = QString(m_frame->m_backgroundManager->backgroundImages().value(m_frame->m_screenName));
    WallpaperItem* item = m_frame->m_wallpaperList->addWallpaper(currentPath);
    QPoint pos = item->pos();

    m_frame->handleNeedCloseButton(currentPath, pos);
    EXPECT_TRUE(!m_frame->m_closeButton->isHidden());
}

TEST_F(FrameTest, test_reLayoutTools)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

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

TEST_F(FrameTest, test_desktopbackground)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    WallpaperItem* temp = new WallpaperItem;
    m_frame->m_wallpaperList->m_items << temp;
    WallpaperItem* item = m_frame->m_wallpaperList->getCurrentItem();
    m_frame->onItemPressed(item->data());
    QPair<QString, QString> pair = m_frame->desktopBackground();
    QPair<QString, QString> compare = QPair<QString, QString>(m_frame->m_screenName, m_frame->m_desktopWallpaper);

    EXPECT_EQ(pair, compare);
    delete temp;
}

TEST_F(FrameTest, test_setwallpaperslideshow)
{

    QString temp("30");
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(ComDeepinDaemonAppearanceInterface, SetWallpaperSlideShow), [&judge](){judge = true; return QDBusPendingReply<QString>();});

    m_frame->setWallpaperSlideShow(temp);
    EXPECT_TRUE(judge);

    delete m_frame->m_dbusAppearance;
    m_frame->m_dbusAppearance = nullptr;
    m_frame->setWallpaperSlideShow(temp);
    QString ret = m_frame->getWallpaperSlideShow();
    EXPECT_EQ(ret, QString());
}

TEST_F(FrameTest, test_setbackground)
{
    m_frame->m_wallpaperList->setCurrentIndex(0);
    m_frame->m_desktopWallpaper = QString("/usr/share/backgrounds/default_background.jpg");

    m_frame->setBackground();
    EXPECT_TRUE(m_frame->desktopBackground().second == QString("/usr/share/backgrounds/default_background.jpg"));

    delete m_frame->m_dbusAppearance;
    m_frame->m_dbusAppearance = nullptr;
    m_frame->setBackground();
}

TEST_F(FrameTest, test_getwallpaperslideshow)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    stub_ext::StubExt stu;
    bool isshow = false;
    stu.set_lamda(ADDR(ComDeepinDaemonAppearanceInterface, GetWallpaperSlideShow), [&isshow](){isshow = true; return QDBusPendingReply<QString>();});
    m_frame->m_wallpaperList->setCurrentIndex(1);
    QString str;
    qApp->processEvents();
    QString temp1 = m_frame->getWallpaperSlideShow();
    EXPECT_TRUE(isshow);

    delete m_frame->m_dbusAppearance;
    m_frame->m_dbusAppearance = nullptr;
    temp1 = m_frame->getWallpaperSlideShow();
    EXPECT_EQ(temp1, str);
}

TEST_F(FrameTest, test_wayland)
{
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DesktopInfo, waylandDectected), [&judge](){judge = true; return true;});
    Frame* temp1 = new Frame(qApp->primaryScreen()->name());
    delete temp1;

    stu.set_lamda(ADDR(QWidget, windowHandle), [](){return nullptr;});
    temp1 = new Frame(qApp->primaryScreen()->name());
    delete temp1;
    EXPECT_TRUE(judge);
}
