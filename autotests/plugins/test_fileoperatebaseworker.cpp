// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatebaseworker.cpp
 * @brief Unit tests for FileOperateBaseWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileOperateBaseWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperateBaseWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperateBaseWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperateBaseWorkerTest, checkAndCopyFile)
{
    // Test method: bool checkAndCopyFile((const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip))
    auto result = obj->checkAndCopyFile(DFileInfoPointer(), DFileInfoPointer(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, checkFileSize)
{
    // Test method: bool checkFileSize((qint64 size, const QUrl &fromUrl,
                                          const QUrl &toUrl, bool *skip))
    QUrl _arg1{};
    QUrl _arg2{};
    auto result = obj->checkFileSize(0, _arg1, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, emitCurrentTaskNotify)
{
    // Test method: void emitCurrentTaskNotify((const QUrl &from, const QUrl &to))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->emitCurrentTaskNotify(_arg0, _arg1));
}

TEST_F(FileOperateBaseWorkerTest, emitErrorNotify)
{
    // Test method: void emitErrorNotify((const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                                            const bool isTo, const quint64 id, const QString &errorMsg,
                                            const bool allUsErrorMsg))
    QUrl _arg0{};
    QUrl _arg1{};
    AbstractJobHandler::JobErrorType _arg2{};
    QString _arg5{};
    EXPECT_NO_FATAL_FAILURE(obj->emitErrorNotify(_arg0, _arg1, _arg2, false, 0, _arg5, false));
}

TEST_F(FileOperateBaseWorkerTest, getTidWriteSize)
{
    // Test getter: qint64 getTidWriteSize()
    auto result = obj->getTidWriteSize();
    EXPECT_EQ(result, 0);

}

TEST_F(FileOperateBaseWorkerTest, getWriteDataSize)
{
    // Test getter: qint64 getWriteDataSize()
    auto result = obj->getWriteDataSize();
    EXPECT_EQ(result, 0);

}

TEST_F(FileOperateBaseWorkerTest, initCopyWay)
{
    // Test method: void initCopyWay(())
    EXPECT_NO_FATAL_FAILURE(obj->initCopyWay());
}

TEST_F(FileOperateBaseWorkerTest, initThreadCopy)
{
    // Test method: void initThreadCopy(())
    EXPECT_NO_FATAL_FAILURE(obj->initThreadCopy());
}

TEST_F(FileOperateBaseWorkerTest, removeTrashInfo)
{
    // Test method: void removeTrashInfo((const QUrl &trashInfoUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeTrashInfo(_arg0));
}

TEST_F(FileOperateBaseWorkerTest, shouldUseBlockWriteType)
{
    // Test bool getter: shouldUseBlockWriteType()
    bool result = obj->shouldUseBlockWriteType();
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, trashInfo)
{
    // Test method: QUrl trashInfo((const DFileInfoPointer &fromInfo))
    DFileInfoPointer _arg0{};
    auto result = obj->trashInfo(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileOperateBaseWorkerTest, FileOperateBaseWorker)
{
    // Test constructor: FileOperateBaseWorker((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileOperateBaseWorkerTest, doHandleErrorAndWait)
{
    // Test method: AbstractJobHandler::SupportAction doHandleErrorAndWait((const QUrl &urlFrom, const QUrl &urlTo,
                                                                              const AbstractJobHandler::JobErrorType &error, const bool isTo,
                                                                              const QString &errorMsg, const bool errorMsgAll))
    QUrl _arg0{};
    QUrl _arg1{};
    AbstractJobHandler::JobErrorType _arg2{};
    QString _arg4{};
    auto result = obj->doHandleErrorAndWait(_arg0, _arg1, _arg2, false, _arg4, false);
    EXPECT_GE(static_cast<int>(result), 0);

}
