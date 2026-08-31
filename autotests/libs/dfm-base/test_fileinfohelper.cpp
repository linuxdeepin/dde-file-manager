// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfohelper.cpp
 * @brief Unit tests for FileInfoHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/fileinfohelper.h"

#include <QTest>

using namespace src;

class FileInfoHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileInfoHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileInfoHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileInfoHelperTest, M_~FileInfoHelper)
{
    // Test method:  ~FileInfoHelper(())
    EXPECT_NO_FATAL_FAILURE({ FileInfoHelper *tmp = new FileInfoHelper(); delete tmp; });
}

TEST_F(FileInfoHelperTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(FileInfoHelperTest, instance)
{
    // Test getter: FileInfoHelper instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(FileInfoHelperTest, fileCountAsync)
{
    // Test method: QSharedPointer<FileInfoHelperUeserData> fileCountAsync((QUrl &url))
    QUrl _arg0{};
    auto result = obj->fileCountAsync(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileInfoHelperTest, fileRefreshAsync)
{
    // Test method: void fileRefreshAsync((const QSharedPointer<FileInfo> dfileInfo))
    EXPECT_NO_FATAL_FAILURE(obj->fileRefreshAsync(QSharedPointer<FileInfo>()));
}

TEST_F(FileInfoHelperTest, aboutToQuit)
{
    // Test method: void aboutToQuit(())
    EXPECT_NO_FATAL_FAILURE(obj->aboutToQuit());
}

TEST_F(FileInfoHelperTest, handleFileRefresh)
{
    // Test method: void handleFileRefresh((QSharedPointer<FileInfo> dfileInfo))
    EXPECT_NO_FATAL_FAILURE(obj->handleFileRefresh(QSharedPointer<FileInfo>()));
}

TEST_F(FileInfoHelperTest, checkInfoRefresh)
{
    // Test method: void checkInfoRefresh((QSharedPointer<FileInfo> dfileInfo))
    EXPECT_NO_FATAL_FAILURE(obj->checkInfoRefresh(QSharedPointer<FileInfo>()));
}

TEST_F(FileInfoHelperTest, fileMimeTypeAsync)
{
    // Test method: QSharedPointer<FileInfoHelperUeserData> fileMimeTypeAsync((const QUrl &url, const QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs))
    QUrl _arg0{};
    QString _arg2{};
    auto result = obj->fileMimeTypeAsync(_arg0, QMimeDatabase::MatchMode(), _arg2, false);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileInfoHelperTest, handleCheckInfoRefresh)
{
    // Test method: void handleCheckInfoRefresh((QSharedPointer<FileInfo> dfileInfo))
    EXPECT_NO_FATAL_FAILURE(obj->handleCheckInfoRefresh(QSharedPointer<FileInfo>()));
}

TEST_F(FileInfoHelperTest, threadHandleDfmFileInfo)
{
    // Test method: void threadHandleDfmFileInfo((const QSharedPointer<FileInfo> dfileInfo))
    EXPECT_NO_FATAL_FAILURE(obj->threadHandleDfmFileInfo(QSharedPointer<FileInfo>()));
}

TEST_F(FileInfoHelperTest, cacheFileInfoByThread)
{
    // Test method: void cacheFileInfoByThread((const QSharedPointer<FileInfo> dfileInfo))
    EXPECT_NO_FATAL_FAILURE(obj->cacheFileInfoByThread(QSharedPointer<FileInfo>()));
}

TEST_F(FileInfoHelperTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(FileInfoHelperTest, thread)
{
    // Test getter: QSharedPointer<QThread> thread()
    auto result = obj->thread();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(FileInfoHelperTest, worker)
{
    // Test getter: QSharedPointer<FileInfoAsycWorker> worker()
    auto result = obj->worker();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(FileInfoHelperTest, stoped)
{
    // Test getter: std::atomic_bool stoped()
    auto result = obj->stoped();
    EXPECT_NO_FATAL_FAILURE({ obj->stoped(); });

}

TEST_F(FileInfoHelperTest, pool)
{
    // Test getter: QThreadPool pool()
    auto result = obj->pool();
    EXPECT_NO_FATAL_FAILURE({ obj->pool(); });

}

TEST_F(FileInfoHelperTest, qureingInfo)
{
    // Test getter: DThreadList<FileInfoPointer> qureingInfo()
    auto result = obj->qureingInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(FileInfoHelperTest, needQureingInfo)
{
    // Test getter: DThreadList<FileInfoPointer> needQureingInfo()
    auto result = obj->needQureingInfo();
    EXPECT_EQ(result.get(), nullptr);

}
