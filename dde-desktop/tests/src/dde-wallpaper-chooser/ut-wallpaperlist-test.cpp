#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QScreen>
#include <QEventLoop>
#include <QVBoxLayout>
#include <QAbstractAnimation>
#define private public
#define protected public

#include "../dde-wallpaper-chooser/wallpaperitem.h"
#include "../dde-wallpaper-chooser/frame.h"
#include "../dde-wallpaper-chooser/wallpaperlist.h"

#include "../stub-ext/stubext.h"

using namespace testing;

namespace  {
       class WallpaperlistTest : public Test {
       public:

           void SetUp() override {
               for(int i = 0; i < 3; ++i) {
                   m_list->addItem(&item[i]);
               }
           }

           void TearDown() override {
           }

       private:
           WallpaperList list;
           WallpaperList* m_list = &list;
           WallpaperItem item[3];
       };
}

TEST_F(WallpaperlistTest, test_count)
{
    int ret = m_list->count();
    EXPECT_EQ(3, ret);
}
TEST_F(WallpaperlistTest, test_keypressevent)
{
    QKeyEvent* eventright = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent* eventleft = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent* eventdown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    Stub stu;
    static bool index = false;
    bool (*myindex)() = [](){ index = true; return true;};
    stu.set(ADDR(WallpaperList, setCurrentIndex), myindex);
    m_list->keyPressEvent(eventright);
    EXPECT_TRUE(index);

    m_list->keyPressEvent(eventleft);
    EXPECT_TRUE(index);

    static bool ignore = false;
    void (*myignore)() = [](){ ignore = true; };
    stu.set(ADDR(QKeyEvent, ignore), myignore);
    m_list->keyPressEvent(eventdown);
    EXPECT_TRUE(ignore);

    static bool judge = false;
    void (*myjudge)() = [](){ judge = true; };
    int (*mystate)() = [](){ return 2;};
    stu.set(ADDR(QKeyEvent, isAutoRepeat), myindex);
    stu.set(ADDR(QKeyEvent, accept), myjudge);
    stu.set(ADDR(QAbstractAnimation, state), mystate);
    m_list->keyPressEvent(eventleft);
    EXPECT_TRUE(judge);

    delete eventright;
    delete eventleft;
    delete eventdown;
}

TEST_F(WallpaperlistTest, test_removewallpaper)
{
   int count = m_list->count();
   Stub stu;
   static QString temp("test");
   QString (*mystring)() = [](){return temp;};
   void (*mydelete)() = [](){return;};
   stu.set(ADDR(WallpaperItem, getPath), mystring);
   stu.set(ADDR(QObject, deleteLater), mydelete);
   m_list->removeWallpaper(QString("test"));
   EXPECT_NE(count, m_list->count());

   m_list->prevItem = m_list->m_items[0];
   m_list->removeWallpaper(QString("test"));
   EXPECT_NE(count, m_list->count());

   m_list->nextItem = m_list->m_items[0];
   m_list->removeWallpaper(QString("test"));
   EXPECT_NE(count, m_list->count());
}

TEST_F(WallpaperlistTest, test_wallpaperitempressed)
{
   Stub stu;
   static bool judge = false;
   void (*myjudge)() = [](){judge = true;};
   stu.set(ADDR(WallpaperList, setCurrentIndex), myjudge);
   m_list->wallpaperItemPressed();
   qApp->processEvents();
   EXPECT_TRUE(judge);

   QWheelEvent event(QPointF(), 1, Qt::LeftButton, Qt::NoModifier);
   m_list->wheelEvent(&event);//函数体无代码,直接调用
   m_list->wallpaperItemHoverOut();//函数体无代码
   static bool visible = false;
   bool (*myvisible)() = [](){visible = true; return true;};
   stu.set(ADDR(QWidget, isVisible), myvisible);

   m_list->wallpaperItemHoverIn();
   EXPECT_TRUE(visible);
}

