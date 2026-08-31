// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskhandlers.cpp
 * @brief Unit tests for TaskHandlers methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/taskhandler.h"

#include <QTest>

using namespace src;

class TaskHandlersTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TaskHandlers();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TaskHandlers *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TaskHandlersTest, createFileListProvider)
{
    // Test method: std::unique_ptr<FileProvider> createFileListProvider((const IndexContext &context, const QStringList &fileList))
    IndexContext _arg0{};
    QStringList _arg1{};
    auto result = obj->createFileListProvider(_arg0, _arg1);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(TaskHandlersTest, createFileProvider)
{
    // Test method: std::unique_ptr<FileProvider> createFileProvider((const IndexContext &context, const QString &path))
    IndexContext _arg0{};
    QString _arg1{};
    auto result = obj->createFileProvider(_arg0, _arg1);
    EXPECT_NE(result.get(), nullptr);

}
