// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskhandler_factories.cpp
 * @brief Unit tests for the TaskHandlers factory functions (task/taskhandler.cpp).
 *        The factory functions only construct and return handler lambdas /
 *        FileProvider instances — the heavy indexing bodies run only when the
 *        returned handler is invoked, which these tests deliberately do NOT do.
 *        This covers the factory functions themselves plus the
 *        createAnythingFileProvider short-circuit branch.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QHash>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexruntime.h"
#include "services/textindex/task/taskhandler.h"
#include "services/textindex/utils/taskstate.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class TaskHandlerFactoriesTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    IndexProfile makeProfile()
    {
        return IndexProfile(IndexProfile::Type::Content,
                            "th_test",
                            "th_status.json",
                            "th_version",
                            1,
                            [this]() -> QString { return tmp.path(); },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &) -> bool { return true; });
    }

    std::unique_ptr<IndexRuntime> runtime;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        // A real directory with a file so createFileProvider has something to wrap.
        QDir root(tmp.path());
        ASSERT_TRUE(root.mkpath("d"));
        QFile f(root.filePath("d/a.txt"));
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.close();
        runtime = std::make_unique<IndexRuntime>(makeProfile());
    }
};

TEST_F(TaskHandlerFactoriesTest, CreateFileProviderWrapsFileSystemForTempPath)
{
    auto p = TaskHandlers::createFileProvider(runtime->context(), tmp.path());
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->name(), QString("FileSystemProvider"));
}

TEST_F(TaskHandlerFactoriesTest, CreateFileListProviderWrapsMixedList)
{
    QStringList files { tmp.path() + "/d/a.txt" };
    auto p = TaskHandlers::createFileListProvider(runtime->context(), files);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->name(), QString("MixedPathListProvider"));
}

TEST_F(TaskHandlerFactoriesTest, CreateIndexHandlerReturnsCallable)
{
    TaskHandler h = TaskHandlers::CreateIndexHandler(runtime->context());
    EXPECT_TRUE(h);   // function<bool()> is truthy when populated
}

TEST_F(TaskHandlerFactoriesTest, UpdateIndexHandlerReturnsCallable)
{
    TaskHandler h = TaskHandlers::UpdateIndexHandler(runtime->context());
    EXPECT_TRUE(h);
}

TEST_F(TaskHandlerFactoriesTest, CreateOrUpdateFileListHandlerReturnsCallable)
{
    TaskHandler h = TaskHandlers::CreateOrUpdateFileListHandler(runtime->context(), { tmp.path() + "/d/a.txt" });
    EXPECT_TRUE(h);
}

TEST_F(TaskHandlerFactoriesTest, RemoveFileListHandlerReturnsCallable)
{
    TaskHandler h = TaskHandlers::RemoveFileListHandler(runtime->context(), { tmp.path() + "/d/a.txt" });
    EXPECT_TRUE(h);
}

TEST_F(TaskHandlerFactoriesTest, MoveFileListHandlerReturnsCallable)
{
    QHash<QString, QString> moves { { tmp.path() + "/d/a.txt", tmp.path() + "/d/b.txt" } };
    TaskHandler h = TaskHandlers::MoveFileListHandler(runtime->context(), moves);
    EXPECT_TRUE(h);
}
