#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QFileInfo>
#include <QThread>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QPixmap>
#define private public

#include "../dde-wallpaper-chooser/thumbnailmanager.h"


using namespace testing;


namespace  {
     class ThumbnailManagerTest : public Test{
     public:
         ThumbnailManagerTest():Test(){

         }


         virtual void SetUp() override{
             Tmanager = new ThumbnailManager(2.1);
         }

         virtual void TearDown() override{
             delete Tmanager;
         }

         ThumbnailManager* Tmanager;
     };
}

TEST_F(ThumbnailManagerTest,scale)
{
    qreal temp = Tmanager->scale();
    EXPECT_EQ(temp,Tmanager->m_scale);
}


TEST_F(ThumbnailManagerTest,stop)
{
    Tmanager->stop();
    EXPECT_TRUE(Tmanager->m_queuedRequests.empty());
}


TEST_F(ThumbnailManagerTest,clear)
{
   QFileInfo dir1(Tmanager->m_cacheDir);
   QThread::sleep(1);
   Tmanager->clear();
   QFileInfo dir2(Tmanager->m_cacheDir);
   QDateTime dt1 = dir1.lastRead();
   QDateTime dt2 = dir2.lastRead();
   int dint1 = dt1.toString("ss").toInt();
   int dint2 = dt2.toString("ss").toInt();
   EXPECT_EQ(dint1,dint2);
}

TEST_F(ThumbnailManagerTest,remove)
{
    Tmanager->remove("my_test");
    QString file = QDir(Tmanager->m_cacheDir).absoluteFilePath("my_test");
    ASSERT_FALSE(QFile::exists(file));
}

