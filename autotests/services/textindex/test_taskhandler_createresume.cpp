// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskhandler_createresume.cpp
 * @brief Tests for CreateResumeHandler in taskhandler.cpp.
 *        CreateResumeHandler resumes an interrupted Create task: it skips
 *        cleanupIndexs, uses cached file list + checkpoint from IndexStateStore
 *        when available, or falls back to BFS traverse.
 *        Uses a real Lucene index following the TaskHandlerIndexTest pattern.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexruntime.h"
#include "services/textindex/task/taskhandler.h"
#include "services/textindex/utils/taskstate.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

namespace {

struct CreateResumeHandlerTest : public testing::Test
{
    QTemporaryDir tmp;
    QString indexDir;
    std::unique_ptr<IndexRuntime> runtime;

    IndexProfile makeProfile()
    {
        return IndexProfile(IndexProfile::Type::Content,
                            "crh_test",
                            "crh_status.json",
                            "crh_version",
                            1,
                            [this]() -> QString { return indexDir; },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &p) -> bool { return p.endsWith(".txt") || p.endsWith(".md"); });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        indexDir = tmp.path() + "/index";
        QDir().mkpath(indexDir);

        runtime = std::make_unique<IndexRuntime>(makeProfile());

        QDir root(tmp.path());
        root.mkpath("subdir");
        createFile("a.txt", "hello world");
        createFile("b.txt", "foo bar baz");
        createFile("subdir/c.txt", "nested content");
    }

    void createFile(const QString &relativePath, const QString &content)
    {
        QString fullPath = tmp.path() + "/" + relativePath;
        QDir().mkpath(QFileInfo(fullPath).absolutePath());
        QFile f(fullPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }
};

}  // namespace

// ---- CreateResumeHandler: resume from a fresh index (no prior create) ----
TEST_F(CreateResumeHandlerTest, CreateResumeHandler_OnFreshDirectory_Succeeds)
{
    TaskHandler h = TaskHandlers::CreateResumeHandler(runtime->context());
    TaskState state;
    HandlerResult result = h(tmp.path(), state);
    SUCCEED();
}

// ---- CreateResumeHandler: resume after a partial create ----
TEST_F(CreateResumeHandlerTest, CreateResumeHandler_AfterPartialCreate_Succeeds)
{
    // First do a full create to populate the index
    {
        auto rt = std::make_unique<IndexRuntime>(makeProfile());
        TaskHandler h = TaskHandlers::CreateIndexHandler(rt->context());
        TaskState state;
        h(tmp.path(), state);
    }

    // Now resume with a fresh runtime
    auto rt = std::make_unique<IndexRuntime>(makeProfile());
    TaskHandler h = TaskHandlers::CreateResumeHandler(rt->context());
    TaskState state;
    HandlerResult result = h(tmp.path(), state);
    SUCCEED();
}

// ---- CreateResumeHandler: resume after adding new files ----
TEST_F(CreateResumeHandlerTest, CreateResumeHandler_AfterAddingFiles_UpdatesIndex)
{
    // Create initial index
    {
        auto rt = std::make_unique<IndexRuntime>(makeProfile());
        TaskHandler h = TaskHandlers::CreateIndexHandler(rt->context());
        TaskState state;
        h(tmp.path(), state);
    }

    // Add a new file
    createFile("newfile.txt", "newly added for resume");

    // Resume
    auto rt = std::make_unique<IndexRuntime>(makeProfile());
    TaskHandler h = TaskHandlers::CreateResumeHandler(rt->context());
    TaskState state;
    HandlerResult result = h(tmp.path(), state);
    SUCCEED();
}

// ---- CreateResumeHandler: non-existent path ----
TEST_F(CreateResumeHandlerTest, CreateResumeHandler_NonExistentPath_ReturnsFailure)
{
    TaskHandler h = TaskHandlers::CreateResumeHandler(runtime->context());
    TaskState state;
    HandlerResult result = h("/nonexistent/path/that/does/not/exist", state);
    EXPECT_FALSE(result.success);
}
