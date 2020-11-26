#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QScreen>
#include <QEventLoop>
#include <QVBoxLayout>
#define private public
#define protected public

#include "../dde-wallpaper-chooser/wallpaperitem.h"
#include "../dde-wallpaper-chooser/frame.h"
#include "../dde-wallpaper-chooser/wallpaperlist.h"
using namespace testing;

namespace  {
       class WallpaperlistTest : public Test {
       public:

           void SetUp() override {
               m_frame = new Frame(qApp->primaryScreen()->name());
           }

           void TearDown() override {
               delete m_frame;
           }

       private:

           Frame* m_frame = nullptr;
       };
}

TEST_F(WallpaperlistTest, test_count)
{
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    qint32 count = m_frame->m_wallpaperList->count();

    EXPECT_TRUE(count != 0);
}

TEST_F(WallpaperlistTest, test_keypressevent)
{
    m_frame->show();

    QKeyEvent* eventright = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent* eventleft = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent* eventdown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QWheelEvent* wheelevent = new QWheelEvent(QPointF(), 1, Qt::LeftButton, Qt::NoModifier);

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    m_frame->m_wallpaperList->setCurrentIndex(1);
    qint32 first  = m_frame->m_wallpaperList->m_index;
    m_frame->m_wallpaperList->keyPressEvent(eventright);
    m_frame->m_wallpaperList->keyPressEvent(eventleft);
    m_frame->m_wallpaperList->keyPressEvent(eventdown);
    m_frame->m_wallpaperList->wheelEvent(wheelevent);
    qint32 second = m_frame->m_wallpaperList->m_index;

    EXPECT_EQ(first, second);
    m_frame->hide();
    delete eventright;
    delete eventleft;
    delete eventdown;
    delete wheelevent;
}

TEST_F(WallpaperlistTest, test_removewallpaper)
{
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    WallpaperItem* temp = dynamic_cast<WallpaperItem*>(m_frame->m_wallpaperList->getCurrentItem());
    const QString path = temp->getPath();
    qint32 fcount = m_frame->m_wallpaperList->m_items.size();
    m_frame->m_wallpaperList->removeWallpaper(path);
    qint32 scount = m_frame->m_wallpaperList->m_items.size();

    EXPECT_NE(fcount, scount);
    m_frame->hide();
}

TEST_F(WallpaperlistTest, test_wallpaperitempressed)
{
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    qint32 index = m_frame->m_wallpaperList->m_index == m_frame->m_wallpaperList->count() - 1 ? m_frame->m_wallpaperList->count() - 2 : m_frame->m_wallpaperList->m_index;
    WallpaperItem* temp = dynamic_cast<WallpaperItem*>(m_frame->m_wallpaperList->item(index));
    QTest::mousePress(temp, Qt::LeftButton, Qt::NoModifier);
    WallpaperItem* item = m_frame->m_wallpaperList->getCurrentItem();

    EXPECT_EQ(temp, item);
}
