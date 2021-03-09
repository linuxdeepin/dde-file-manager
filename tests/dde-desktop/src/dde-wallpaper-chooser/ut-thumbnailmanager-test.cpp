/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xinglinkun<xinglinkun@uniontech.com>
 *
 * Maintainer: xinglinkun<xinglinkun@uniontech.com>
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
    future1.waitForFinished();
    m_manager->processNextReq();
    QFuture<QPixmap> future2 = m_manager->m_futureWatcher.future();
    EXPECT_NE(future1, future2);
    future2.waitForFinished();
}

TEST_F(ThumbnailManagerTest, find_bysize_singnal)
{
    bool bjudge = false;
    QString test("test");
    {
        m_manager->m_queuedRequests.clear();
        stub_ext::StubExt stu;
        stu.set_lamda(ADDR(QPixmap, isNull), [](){return false;});
        stu.set_lamda(ADDR(ThumbnailManager, thumbnailFounded), [&bjudge](){bjudge = true;});
        m_manager->find(test);
        EXPECT_TRUE(bjudge);
        ASSERT_TRUE(m_manager->m_queuedRequests.isEmpty());
    }

    {
        m_manager->m_queuedRequests.clear();
        stub_ext::StubExt stu;
        bjudge = false;
        stu.set_lamda(ADDR(QPixmap, isNull), [](){return true;});
        stu.set_lamda(ADDR(ThumbnailManager, processNextReq), [&bjudge](){bjudge = true;});
        m_manager->find(test);
        ASSERT_EQ(m_manager->m_queuedRequests.size(), 1);
        EXPECT_EQ(m_manager->m_queuedRequests.first(), test);
        EXPECT_TRUE(bjudge);
    }
}


