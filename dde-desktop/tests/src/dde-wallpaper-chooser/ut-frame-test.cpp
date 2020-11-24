#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QScreen>
#include <DButtonBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <DIconButton>
#include <QLabel>

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
    QEventLoop loop1;
    QTimer::singleShot(1000, &loop1, [&loop1](){
        loop1.exit();
    });
    loop1.exec();

    //模拟翻页，看显示情况
    qApp->processEvents();
    QEventLoop loop;
    QTimer::singleShot(500, &loop, [this] {
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
    });
    QTimer::singleShot(1000, &loop, [this] {
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
    });
    QTimer::singleShot(1500, &loop, [&loop, this] {
        this->m_frame->m_mouseArea->buttonPress(QPoint(0, 0), 1);
        loop.exit();
    });
    loop.exec();

    EXPECT_TRUE(m_frame->isTopLevel());
    m_frame->hide();
}

TEST_F(FrameTest, test_show)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    m_frame->show();
    bool status = false;

    QEventLoop loop;
    if (!m_frame->isHidden()) {
        status = true;
    }
    QTimer::singleShot(1000, &loop, [&loop](){
       loop.exit();
    });
    loop.exec();

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
    QEventLoop loop;
    if (m_frame->isHidden()) {
        status = true;
    }
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    EXPECT_TRUE(status);
}

TEST_F(FrameTest, test_onrest)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    m_frame->show();
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();
    m_frame->onRest();
    ScreenPointer testscreen = ScreenMrg->screen(m_frame->m_screenName);
    BackgroundWidgetPointer pointer = m_frame->m_backgroundManager->backgroundWidget(testscreen);
    if (pointer) {
       QWidget* widget = m_frame->topLevelWidget();
       EXPECT_NE(widget, pointer);
    }

    EXPECT_TRUE(m_frame->isTopLevel());
    m_frame->hide();
}

TEST_F(FrameTest, test_refreshlist)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){loop.exit();});
    loop.exec();

    m_frame->m_mode = Frame::ScreenSaverMode;
    QString currentPath = QString(m_frame->m_backgroundManager->backgroundImages().value(m_frame->m_screenName));
    WallpaperItem* item = m_frame->m_wallpaperList->addWallpaper(currentPath);
    Button* btn = dynamic_cast<Button*>(item->m_buttonLayout->itemAt(0));
    WallpaperItem* itemsingnal = m_frame->m_wallpaperList->getCurrentItem();
    m_frame->m_mode = Frame::WallpaperMode;
    emit itemsingnal->tab();

    EXPECT_NE(m_frame->m_wallpaperList->count(), 0);
    m_frame->hide();
}

TEST_F(FrameTest, test_onitemispressed)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(100, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();
    QString data = QString("/usr/share/backgrounds/default_background.jpg");
    m_frame->onItemPressed(data);
    QMap<QString, QString> path = m_frame->m_backgroundManager->backgroundImages();
    QString backgroundpath = path[qApp->primaryScreen()->name()];

    EXPECT_EQ(data, backgroundpath);
    EXPECT_EQ(m_frame->m_desktopWallpaper, m_frame->m_lockWallpaper);
    m_frame->hide();
}

TEST_F(FrameTest, test_onitembuttonisclicked)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    m_frame->show();

    m_frame->m_wallpaperList->addItem(new WallpaperItem);
    m_frame->m_wallpaperList->addItem(new WallpaperItem);
    m_frame->m_wallpaperList->setCurrentIndex(1);

    auto item = m_frame->m_wallpaperList->getCurrentItem();
    emit item->buttonClicked("desktop");
    EXPECT_TRUE(m_frame->m_lockWallpaper.isEmpty());

    emit item->buttonClicked("lock-screen");
    EXPECT_TRUE(m_frame->m_desktopWallpaper.isEmpty());
    m_frame->hide();
}

TEST_F(FrameTest, test_setmode)
{
   ASSERT_EQ(m_frame->m_desktopWallpaper, nullptr);
   Frame::Mode m = m_frame->m_mode;

   m_frame->m_mode = Frame::ScreenSaverMode;
   m_frame->setMode(Frame::WallpaperMode);

   EXPECT_EQ(m, m_frame->m_mode);
   QTimer timer;
   timer.start(300);
   QEventLoop loop;
   QObject::connect(&timer, &QTimer::timeout, [&]{
       m_frame->hide();
       timer.stop();
       loop.exit();
   });
   loop.exec();

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
    m_frame->show();

    //触发事件看显示结果
    QKeyEvent* eventesc = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QKeyEvent* eventright = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent* eventleft = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);

    QTimer timer,timer1;
    timer.start(2500);
    QEventLoop loop;
    QTimer::singleShot(500, &loop, [&](){
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
    });
    QTimer::singleShot(1000, &loop, [&](){
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
    });
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
        loop.exit();
    });
    loop.exec();
    m_frame->show();
    m_frame->setMode(Frame::ScreenSaverMode);
    timer1.start(2000);
    QEventLoop loop1;
    QTimer::singleShot(1000, &loop, [&](){
        m_frame->keyPressEvent(eventesc);
        EXPECT_TRUE(m_frame->isHidden());
    });
    QObject::connect(&timer1, &QTimer::timeout, [&]{
        timer1.stop();
        loop1.exit();
    });
    loop1.exec();

    delete eventesc;
    delete eventright;
    delete eventleft;
    m_frame->hide();
}

TEST_F(FrameTest, test_eventfilter)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    m_frame->show();

    QEventLoop loop1;
    QTimer::singleShot(1000, &loop1, [&loop1](){
        loop1.exit();
    });
    loop1.exec();

    QKeyEvent* eventtab = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QKeyEvent* eventbacktab = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    QKeyEvent* eventkeyright = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);

    QEventLoop loop;
    QTimer::singleShot(500, m_frame, [this, eventtab] {
         QCoreApplication::sendEvent(m_frame,static_cast<QEvent*>(eventtab));
         QCoreApplication::sendEvent(m_frame->m_lockScreenBox,static_cast<QEvent*>(eventtab));
         QCoreApplication::sendEvent(m_frame->m_waitControl->buttonList().at(0),static_cast<QEvent*>(eventtab));
         EXPECT_TRUE(m_frame->m_switchModeControl->buttonList().at(0)->hasFocus());
    });
    QTimer::singleShot(1000, m_frame, [this, eventbacktab] {
        QCoreApplication::sendEvent(m_frame->m_wallpaperCarouselCheckBox,static_cast<QEvent*>(eventbacktab));
        EXPECT_TRUE(m_frame->m_wallpaperList->getCurrentItem()->findChildren<Button*>().at(0)->hasFocus());
    });
    QTimer::singleShot(1500, m_frame, [this, eventbacktab] {
        m_frame->m_mode = Frame::WallpaperMode;
        QCoreApplication::sendEvent(m_frame->m_switchModeControl->buttonList().first(),static_cast<QEvent*>(eventbacktab));
        EXPECT_TRUE(m_frame->m_wallpaperCarouselCheckBox->hasFocus());
    });
    QTimer::singleShot(2000, m_frame, [this, eventbacktab] {
        m_frame->m_mode = Frame::ScreenSaverMode;
        QCoreApplication::sendEvent(m_frame->m_switchModeControl->buttonList().first(),static_cast<QEvent*>(eventbacktab));
        EXPECT_TRUE(m_frame->m_waitControl->buttonList().first()->hasFocus());
    });
    QTimer::singleShot(2200, m_frame, [this, eventkeyright] {
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
    });
    QTimer::singleShot(4000, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();

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
    m_frame->show();

    QString currentPath = QString(m_frame->m_backgroundManager->backgroundImages().value(m_frame->m_screenName));
    WallpaperItem* item = m_frame->m_wallpaperList->addWallpaper(currentPath);
    QPoint pos = item->pos();

    m_frame->handleNeedCloseButton(currentPath, pos);
    EXPECT_TRUE(!m_frame->m_closeButton->isHidden());
    m_frame->hide();
}

TEST_F(FrameTest, test_reLayoutTools)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    m_frame->show();

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
    m_frame->hide();
}

TEST_F(FrameTest, test_desktopbackground)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(100, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    WallpaperItem* item = m_frame->m_wallpaperList->getCurrentItem();
    m_frame->onItemPressed(item->data());
    QPair<QString, QString> pair = m_frame->desktopBackground();
    QPair<QString, QString> compare = QPair<QString, QString>(m_frame->m_screenName, m_frame->m_desktopWallpaper);

    EXPECT_EQ(pair, compare);
    m_frame->hide();
}

TEST_F(FrameTest, test_setwallpaperslideshow)
{
    m_frame->show();
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();

    QString temp("30");
    if (m_frame->m_dbusAppearance != nullptr) {
        m_frame->setWallpaperSlideShow(temp);
    }
    m_frame->m_dbusAppearance = nullptr;
    if (m_frame->m_dbusAppearance == nullptr) {
        m_frame->setWallpaperSlideShow(temp);
        QString ret = m_frame->getWallpaperSlideShow();
        EXPECT_EQ(ret, QString());
    }
}

TEST_F(FrameTest, test_setbackground)
{
    m_frame->show();
    QEventLoop loop;
    QTimer::singleShot(100, &loop, [&loop]{
        loop.exit();
    });
    loop.exec();

    m_frame->m_wallpaperList->setCurrentIndex(0);
    m_frame->m_desktopWallpaper = m_frame->m_wallpaperList->getCurrentItem()->getPath();

    m_frame->setBackground();
    WallpaperItem* item1 = m_frame->m_wallpaperList->getCurrentItem();
    WallpaperItem* item2 = dynamic_cast<WallpaperItem*>(m_frame->m_wallpaperList->item(0));

    EXPECT_EQ(item1->getPath(), item2->getPath());
    m_frame->hide();
}

TEST_F(FrameTest, test_getwallpaperslideshow)
{
    ASSERT_EQ(m_frame->m_dbusDeepinWM, nullptr);
    ASSERT_EQ(m_frame->m_backgroundManager, nullptr);

    m_frame->m_wallpaperList->setCurrentIndex(1);
    QString str;
    m_frame->show();
    qApp->processEvents();

    QString temp1 = m_frame->getWallpaperSlideShow();

    if (m_frame->m_dbusAppearance != nullptr) {
        EXPECT_NE(temp1, str);
    }
    else {
        EXPECT_EQ(temp1, str);
    }
    m_frame->hide();
}
