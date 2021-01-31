#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTest>
#include <QFileInfo>
#include <QThread>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QEventLoop>
#include <QObject>
#include <QTimer>
#include <QtConcurrent>

#define private public

#include "../dde-wallpaper-chooser/thumbnailmanager.h"
#include "stubext.h"


using namespace testing;

namespace  {
     class ThumbnailManagerTest : public Test {
     public:

         ThumbnailManagerTest() : Test()
         {

         }

         virtual void SetUp() override
         {
             m_manager = new ThumbnailManager(2.1);
         }

         virtual void TearDown() override
         {
             delete m_manager;
         }

         ThumbnailManager *m_manager = nullptr;
     };
}

TEST_F(ThumbnailManagerTest, scale)
{
    qreal result = m_manager->scale();
    EXPECT_EQ(result, m_manager->m_scale);
}


TEST_F(ThumbnailManagerTest, stop)
{
    m_manager->stop();
    EXPECT_TRUE(m_manager->m_queuedRequests.empty());
}


TEST_F(ThumbnailManagerTest, clear_filepath)
{
  m_manager->clear();
  bool bexist = QFile::exists(m_manager->m_cacheDir);
  EXPECT_TRUE(bexist);
}


TEST_F(ThumbnailManagerTest, remove_byfilepath)
{
   m_manager->remove("test");
   QString filepath = QDir(m_manager->m_cacheDir).absoluteFilePath("test");
   EXPECT_FALSE(QFile::exists(filepath));
}

TEST_F(ThumbnailManagerTest, processnextreq_byfuture)
{
    ASSERT_TRUE(m_manager->m_futureWatcher.isFinished());
    m_manager->m_queuedRequests << "test";
    QFuture<QPixmap> future1 = m_manager->m_futureWatcher.future();
    m_manager->processNextReq();
    QFuture<QPixmap> future2 = m_manager->m_futureWatcher.future();
    EXPECT_NE(future1, future2);
}

TEST_F(ThumbnailManagerTest, find_bysize_singnal)
{
    bool bjudge = false;
    QTimer timer;
    QString test("test");
    int qsize = m_manager->m_queuedRequests.size();
    QObject::connect(m_manager, &ThumbnailManager::thumbnailFounded, m_manager, [&]{
        bjudge = true;
    });
    m_manager->find("test");
    EXPECT_NE(m_manager->m_queuedRequests.size(), qsize);

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(QPixmap, isNull), [](){return true;});
    stu.set_lamda(ADDR(ThumbnailManager, processNextReq), [](){return;});
    m_manager->find("test");
    QTest::qSleep(10);
    EXPECT_TRUE(bjudge);
}


