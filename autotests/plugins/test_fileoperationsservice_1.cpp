// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperationsservice_1.cpp
 * @brief Unit tests for FileOperationsService methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationsservice.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileOperationsServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperationsService();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperationsService *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperationsServiceTest, cleanTrash)
{
    // Test method: JobHandlePointer cleanTrash((const QList<QUrl> &sources))
    QList<QUrl> _arg0{};
    auto result = obj->cleanTrash(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileOperationsServiceTest, copyFromTrash)
{
    // Test method: JobHandlePointer copyFromTrash((const QList<QUrl> &sources, const QUrl &target, const AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    AbstractJobHandler::JobFlags _arg2{};
    auto result = obj->copyFromTrash(_arg0, _arg1, _arg2);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileOperationsServiceTest, cut)
{
    // Test method: JobHandlePointer cut((const QList<QUrl> &sources, const QUrl &target,
                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags _arg2{};
    auto result = obj->cut(_arg0, _arg1, _arg2);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileOperationsServiceTest, handleWorkerFinish)
{
    // Test method: void handleWorkerFinish(())
    EXPECT_NO_FATAL_FAILURE(obj->handleWorkerFinish());
}

TEST_F(FileOperationsServiceTest, moveToTrash)
{
    // Test method: JobHandlePointer moveToTrash((const QList<QUrl> &sources,
                                                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg0{};
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags _arg1{};
    auto result = obj->moveToTrash(_arg0, _arg1);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileOperationsServiceTest, restoreFromTrash)
{
    // Test method: JobHandlePointer restoreFromTrash((const QList<QUrl> &sources, const QUrl &target,
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags _arg2{};
    auto result = obj->restoreFromTrash(_arg0, _arg1, _arg2);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileOperationsServiceTest, FileOperationsService_Destructor)
{
    // Test method:  ~FileOperationsService(())
    EXPECT_NO_FATAL_FAILURE({ FileOperationsService *tmp = new FileOperationsService(); delete tmp; });
}
