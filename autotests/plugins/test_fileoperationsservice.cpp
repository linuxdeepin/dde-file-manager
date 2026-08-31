// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperationsservice.cpp
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

TEST_F(FileOperationsServiceTest, copy)
{
    // Test method: JobHandlePointer copy((const QList<QUrl> &sources, const QUrl &target,
                                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags _arg2{};
    auto result = obj->copy(_arg0, _arg1, _arg2);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileOperationsServiceTest, deletes)
{
    // Test method: JobHandlePointer deletes((const QList<QUrl> &sources,
                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg0{};
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags _arg1{};
    auto result = obj->deletes(_arg0, _arg1);
    EXPECT_NE(result.get(), nullptr);

}
