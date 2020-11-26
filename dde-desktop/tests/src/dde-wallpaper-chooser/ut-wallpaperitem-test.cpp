#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QScreen>
#include <QEventLoop>
#include <QVBoxLayout>
#include <QPushButton>

#define private public
#define protected public

#include "../dde-wallpaper-chooser/wallpaperitem.h"
#include "../dde-wallpaper-chooser/frame.h"
#include "../dde-wallpaper-chooser/wallpaperlist.h"
using namespace testing;

namespace  {
       class WallpaperItemTest : public Test {
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

TEST_F(WallpaperItemTest, test_addbutton)
{
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
         loop.exit();
    });
    loop.exec();

    WallpaperItem* item = dynamic_cast<WallpaperItem*>(m_frame->m_wallpaperList->getCurrentItem());
    QPushButton* btn = item->addButton("desktop", "comming_test");
    QString string = QString("comming_test");

    EXPECT_EQ(string, btn->text());
    m_frame->hide();
}

TEST_F(WallpaperItemTest, test_keyevent)
{
    m_frame->show();
    QKeyEvent* eventup = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent* eventdown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    m_frame->refreshList();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    m_frame->m_wallpaperList->setCurrentIndex(1);
    WallpaperItem* temp = m_frame->m_wallpaperList->getCurrentItem();
    temp->keyPressEvent(eventup);

    EXPECT_TRUE(temp->m_buttonLayout->itemAt(0)->widget()->hasFocus());
    temp->keyPressEvent(eventdown);

    EXPECT_TRUE(temp->m_buttonLayout->itemAt(temp->m_buttonLayout->count() - 1)->widget()->hasFocus());
    m_frame->hide();

    delete eventup;
    delete eventdown;
}

TEST_F(WallpaperItemTest, test_focuslastbutton)
{
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    if (m_frame->m_wallpaperList->m_index < m_frame->m_wallpaperList->count() - 1) {
        m_frame->m_wallpaperList->m_index ++;
    }
    m_frame->m_wallpaperList->setCurrentIndex(m_frame->m_wallpaperList->m_index);
    m_frame->m_wallpaperList->setCurrentIndex(1);
    WallpaperItem* temp = m_frame->m_wallpaperList->getCurrentItem();
    temp->focusLastButton();

    EXPECT_TRUE(temp->m_buttonLayout->itemAt(temp->m_buttonLayout->count()-1)->widget()->hasFocus());
    m_frame->hide();
}

TEST_F(WallpaperItemTest, test_setdata)
{
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    if (m_frame->m_wallpaperList->m_index < m_frame->m_wallpaperList->count() - 1) {
        m_frame->m_wallpaperList->m_index ++;
    }
    m_frame->m_wallpaperList->setCurrentIndex(m_frame->m_wallpaperList->m_index);
    WallpaperItem* item = dynamic_cast<WallpaperItem*>(m_frame->m_wallpaperList->getCurrentItem());
    item->setData("ceshi");

    EXPECT_EQ(item->data(), QString("ceshi"));
    m_frame->hide();
}

TEST_F(WallpaperItemTest, test_getdeleteable)
{
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    if (m_frame->m_wallpaperList->m_index < m_frame->m_wallpaperList->count() - 1) {
        m_frame->m_wallpaperList->m_index ++;
    }
    m_frame->m_wallpaperList->setCurrentIndex(m_frame->m_wallpaperList->m_index);
    WallpaperItem* item = dynamic_cast<WallpaperItem*>(m_frame->m_wallpaperList->getCurrentItem());
    bool able = item->getDeletable();

    EXPECT_EQ(able, item->m_deletable);
}

TEST_F(WallpaperItemTest, test_setusethumbnailmanager)
{
    m_frame->show();

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [&loop](){
        loop.exit();
    });
    loop.exec();

    if (m_frame->m_wallpaperList->m_index < m_frame->m_wallpaperList->count() - 1) {
        m_frame->m_wallpaperList->m_index ++;
    }
    m_frame->m_wallpaperList->setCurrentIndex(m_frame->m_wallpaperList->m_index);
    WallpaperItem* item = dynamic_cast<WallpaperItem*>(m_frame->m_wallpaperList->getCurrentItem());
    item->setUseThumbnailManager(false);

    EXPECT_FALSE(item->m_useThumbnailManager);
}

