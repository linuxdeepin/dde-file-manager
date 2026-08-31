// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatebaseworker_1.cpp
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

TEST_F(FileOperateBaseWorkerTest, applyAllPendingReplacements)
{
    // Test bool getter: applyAllPendingReplacements()
    bool result = obj->applyAllPendingReplacements();
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, checkDiskSpaceAvailable)
{
    // Test method: bool checkDiskSpaceAvailable((const QUrl &fromUrl,
                                                    const QUrl &toUrl,
                                                    bool *skip))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->checkDiskSpaceAvailable(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, checkLinkAndSameUrl)
{
    // Test method: QVariant checkLinkAndSameUrl((const DFileInfoPointer &fromInfo,
                                                    const DFileInfoPointer &newTargetInfo,
                                                    const bool isCountSize))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    auto result = obj->checkLinkAndSameUrl(_arg0, _arg1, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileOperateBaseWorkerTest, checkTotalDiskSpaceAvailable)
{
    // Test method: bool checkTotalDiskSpaceAvailable((const QUrl &fromUrl, const QUrl &toUrl, bool *skip))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->checkTotalDiskSpaceAvailable(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, createNewTargetUrl)
{
    // Test method: QUrl createNewTargetUrl((const DFileInfoPointer &toInfo, const QString &fileName))
    DFileInfoPointer _arg0{};
    QString _arg1{};
    auto result = obj->createNewTargetUrl(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileOperateBaseWorkerTest, doActionMerge)
{
    // Test method: QVariant doActionMerge((const DFileInfoPointer &fromInfo, const DFileInfoPointer &newTargetInfo, const bool isCountSize))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    auto result = obj->doActionMerge(_arg0, _arg1, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileOperateBaseWorkerTest, doActionReplace)
{
    // Test method: QVariant doActionReplace((const DFileInfoPointer &fromInfo,
                                                const DFileInfoPointer &newTargetInfo,
                                                const bool isCountSize))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    auto result = obj->doActionReplace(_arg0, _arg1, false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileOperateBaseWorkerTest, doCopyLocalByRange)
{
    // Test method: bool doCopyLocalByRange((const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip))
    auto result = obj->doCopyLocalByRange(DFileInfoPointer(), DFileInfoPointer(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, doCopyLocalFile)
{
    // Test method: bool doCopyLocalFile((const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo))
    auto result = obj->doCopyLocalFile(DFileInfoPointer(), DFileInfoPointer());
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, emitSpeedUpdatedNotify)
{
    // Test method: void emitSpeedUpdatedNotify((const qint64 &writSize))
    qint64 _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->emitSpeedUpdatedNotify(_arg0));
}

TEST_F(FileOperateBaseWorkerTest, fileOriginName)
{
    // Test method: QString fileOriginName((const QUrl &trashInfoUrl))
    QUrl _arg0{};
    auto result = obj->fileOriginName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileOperateBaseWorkerTest, getSectorsWritten)
{
    // Test getter: qint64 getSectorsWritten()
    auto result = obj->getSectorsWritten();
    EXPECT_EQ(result, 0);

}

TEST_F(FileOperateBaseWorkerTest, initSignalCopyWorker)
{
    // Test method: void initSignalCopyWorker(())
    EXPECT_NO_FATAL_FAILURE(obj->initSignalCopyWorker());
}

TEST_F(FileOperateBaseWorkerTest, needsSync)
{
    // Test bool getter: needsSync()
    bool result = obj->needsSync();
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, performAsyncSync)
{
    // Test method: void performAsyncSync(())
    EXPECT_NO_FATAL_FAILURE(obj->performAsyncSync());
}

TEST_F(FileOperateBaseWorkerTest, performSync)
{
    // Test method: void performSync(())
    EXPECT_NO_FATAL_FAILURE(obj->performSync());
}

TEST_F(FileOperateBaseWorkerTest, setAllDirPermisson)
{
    // Test method: void setAllDirPermisson(())
    EXPECT_NO_FATAL_FAILURE(obj->setAllDirPermisson());
}

TEST_F(FileOperateBaseWorkerTest, setExpectedSizeForTarget)
{
    // Test setter: void setExpectedSizeForTarget((const QUrl &targetUrl, qint64 size))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setExpectedSizeForTarget(_arg0, 0));
}

TEST_F(FileOperateBaseWorkerTest, setSkipValue)
{
    // Test setter: void setSkipValue((bool *skip, AbstractJobHandler::SupportAction action))
    EXPECT_NO_FATAL_FAILURE(obj->setSkipValue(nullptr, AbstractJobHandler::SupportAction()));
}

TEST_F(FileOperateBaseWorkerTest, shouldUseMultiThreadCopy)
{
    // Test method: bool shouldUseMultiThreadCopy((const DFileInfoPointer &fromInfo))
    DFileInfoPointer _arg0{};
    auto result = obj->shouldUseMultiThreadCopy(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileOperateBaseWorkerTest, waitThreadPoolOver)
{
    // Test method: void waitThreadPoolOver(())
    EXPECT_NO_FATAL_FAILURE(obj->waitThreadPoolOver());
}
