#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

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
    m_manager->m_queuedRequests << "jjj";
    QFuture<QPixmap> future1 = m_manager->m_futureWatcher.future();
    m_manager->processNextReq();
    QFuture<QPixmap> future2 = m_manager->m_futureWatcher.future();
    EXPECT_NE(future1, future2);
}

TEST_F(ThumbnailManagerTest, find_bysize_singnal)
{
    int size = m_manager->m_queuedRequests.size();
    bool bjudge = false;
    QTimer timer;
    QString test("test");
    m_manager->find("test");

    timer.start(2000);
    QEventLoop loop;
    qApp->processEvents();
    QObject::connect(m_manager, &ThumbnailManager::thumbnailFounded, &loop, [&]{
        EXPECT_NE(size, m_manager->m_queuedRequests.size());
        bjudge = true;
        loop.exit();
    });
    QObject::connect(&timer, &QTimer::timeout, [&]{
        timer.stop();
        loop.exit();
    });
    qApp->processEvents();
    loop.exec();
    QString file = QDir(m_manager->m_cacheDir).absoluteFilePath("test");
    const QPixmap pixmap(file);
    if (!pixmap.isNull()) {
        EXPECT_TRUE(bjudge);
    }
}


