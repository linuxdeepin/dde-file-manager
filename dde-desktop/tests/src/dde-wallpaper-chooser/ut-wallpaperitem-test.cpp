#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QScreen>
#include <QEventLoop>
#include <QVBoxLayout>
#include <QPushButton>
#include <QObject>
#include <QFuture>
#include <QPixmap>
#include <QQueue>

#define private public
#define protected public

#include "../dde-wallpaper-chooser/wallpaperitem.h"
#include "../stub-ext/stubext.h"

class WrapperWidget;
using namespace testing;

namespace  {
       class WallpaperItemTest : public Test {
       public:

           void SetUp() override {
               if (m_item == nullptr) {
                    m_item = new WallpaperItem;
               }

           }

           void TearDown() override {
               delete m_item;
           }

       private:

           WallpaperItem* m_item = nullptr;
       };
}

TEST_F(WallpaperItemTest, test_addbutton)
{
    int first = m_item->m_buttonLayout->count();
    m_item->addButton("1", "test");
    int second = m_item->m_buttonLayout->count();
    EXPECT_NE(first, second);
}

TEST_F(WallpaperItemTest, test_keyevent)
{
    QKeyEvent* eventup = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent* eventdown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent* eventdefault = new QKeyEvent(QEvent::KeyPress, Qt::Key_0, Qt::NoModifier);

    m_item->addButton("1", "test01");
    m_item->addButton("2", "test02");

    stub_ext::StubExt stu;
    bool judge = false;

    stu.set_lamda(ADDR(QWidget, hasFocus), [](){return true;});
    m_item->keyPressEvent(eventup);
    EXPECT_TRUE(m_item->m_buttonLayout->itemAt(0)->widget()->hasFocus());

    m_item->keyPressEvent(eventdown);
    EXPECT_TRUE(m_item->m_buttonLayout->itemAt(m_item->m_buttonLayout->count() - 1)->widget()->hasFocus());

    m_item->focusLastButton();
    EXPECT_TRUE(m_item->m_buttonLayout->itemAt(m_item->m_buttonLayout->count() - 1)->widget()->hasFocus());

    stu.set_lamda(ADDR(QKeyEvent, ignore), [&judge](){judge = true;});
    m_item->keyPressEvent(eventdefault);
    EXPECT_TRUE(judge == true);
}

TEST_F(WallpaperItemTest, test_setdata)
{
    m_item->setData(QString("test"));
    EXPECT_EQ(m_item->m_data, QString("test"));
}

TEST_F(WallpaperItemTest, test_getdeleteable)
{
    m_item->setDeletable(false);
    EXPECT_FALSE(m_item->getDeletable());
}

TEST_F(WallpaperItemTest, test_setusethumbnailmanager)
{
   m_item->setUseThumbnailManager(false);
   EXPECT_FALSE(m_item->m_useThumbnailManager);
}

TEST_F(WallpaperItemTest, test_enterEvent)
{
    QEvent event(QEvent::KeyPress);
    bool judge = false;
    QObject::connect(m_item, &WallpaperItem::hoverIn, m_item, [&judge](){judge = true;});
    m_item->enterEvent(&event);
    EXPECT_TRUE(judge);
}

TEST_F(WallpaperItemTest, test_leaveEvent)
{
    QEvent event(QEvent::KeyPress);
    bool judge = false;
    QObject::connect(m_item, &WallpaperItem::hoverOut, m_item, [&judge](){judge = true;});
    m_item->leaveEvent(&event);
    qApp->processEvents();
    EXPECT_TRUE(judge);
}

TEST_F(WallpaperItemTest, test_onFindAborted)
{
     QQueue<QString> que;
     que.push_back("string");
     stub_ext::StubExt stu;
     bool judge = false;
     stu.set_lamda(ADDR(QList<QString>, contains), [&judge](){judge = true; return true;});
     m_item->onFindAborted(que);
     EXPECT_TRUE(judge);
}

TEST_F(WallpaperItemTest, test_mousePressEvent)
{
     QMouseEvent event(QEvent::KeyPress, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
     bool judge = false;
     QObject::connect(m_item, &WallpaperItem::pressed, m_item, [&judge](){judge = true;});
     m_item->mousePressEvent(&event);
     qApp->processEvents();
     EXPECT_TRUE(judge);
}

TEST_F(WallpaperItemTest, test_thumbnailFinished)
{
     stub_ext::StubExt stu;
     stu.set_lamda(ADDR(QFuture<QPixmap>, result), [](){return QPixmap();});
     m_item->thumbnailFinished();
     qApp->processEvents();
     EXPECT_FALSE(m_item->m_wrapper == nullptr);
}

TEST_F(WallpaperItemTest, test_setpath)
{
    m_item->setPath(QString("test"));
    EXPECT_TRUE(m_item->m_path == QString("test"));

    m_item->contentImageGeometry();
    m_item->setUseThumbnailManager(false);
    m_item->useThumbnailManager();
    EXPECT_EQ(m_item->m_useThumbnailManager, false);
}
