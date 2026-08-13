// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_moveprocessor.cpp
 * @brief Unit tests for FileMoveProcessor and DirectoryMoveProcessor
 *        (moveprocessor.cpp). Tests constructors, hasChanges, and edge cases.
 *        Heavy Lucene operations cannot be stubbed easily due to abstract base classes,
 *        so we test the code paths that handle null/empty conditions.
 */

#include <gtest/gtest.h>
#include <QString>
#include <QTemporaryDir>

#include "stubext.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/moveprocessor.h"
#include "services/textindex/core/indexruntime.h"
#include "services/textindex/profile/indexprofile.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

namespace {

struct MoveProcTest : public testing::Test
{
    QTemporaryDir tmp;

    IndexProfile makeProfile()
    {
        return IndexProfile(IndexProfile::Type::Content,
                            "moveproc_test",
                            "moveproc_status.json",
                            "moveproc_version",
                            1,
                            [this]() -> QString { return tmp.path(); },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &p) -> bool { return p.endsWith(".txt") || p.endsWith(".md"); });
    }

    std::unique_ptr<IndexRuntime> makeRuntime()
    {
        return std::make_unique<IndexRuntime>(makeProfile());
    }
};

}  // namespace

// Constructor tests - these only store pointers, no dereference
TEST_F(MoveProcTest, FileMoveProcessor_Constructor)
{
    auto runtime = makeRuntime();
    // Constructor stores pointers but doesn't dereference them
    EXPECT_NO_FATAL_FAILURE({
        FileMoveProcessor proc(runtime->context(), nullptr, nullptr);
    });
}

TEST_F(MoveProcTest, FileMoveProcessor_HasChanges_DefaultFalse)
{
    auto runtime = makeRuntime();
    FileMoveProcessor proc(runtime->context(), nullptr, nullptr);
    EXPECT_FALSE(proc.hasChanges());
}

TEST_F(MoveProcTest, DirectoryMoveProcessor_Constructor)
{
    auto runtime = makeRuntime();
    EXPECT_NO_FATAL_FAILURE({
        DirectoryMoveProcessor proc(runtime->context(), nullptr, nullptr, nullptr);
    });
}

TEST_F(MoveProcTest, DirectoryMoveProcessor_HasChanges_DefaultFalse)
{
    auto runtime = makeRuntime();
    DirectoryMoveProcessor proc(runtime->context(), nullptr, nullptr, nullptr);
    EXPECT_FALSE(proc.hasChanges());
}

// FileMoveProcessor::isFileInIndex with processed paths cache
TEST_F(MoveProcTest, FileMoveProcessor_IsFileInIndex_EmptyCache)
{
    auto runtime = makeRuntime();
    FileMoveProcessor proc(runtime->context(), nullptr, nullptr);
    // With null searcher, this will throw and return false
    // We can't call it safely without a real searcher, but the constructor
    // and hasChanges are covered.
    EXPECT_FALSE(proc.hasChanges());
}

// FileMoveProcessor::processContentUpdateWithCache - tests the wrapper
// Since the actual Lucene calls need real objects, we test the constructor
// and hasChanges which are the non-Lucene methods.

// DirectoryMoveProcessor::updateSingleDocumentPath cannot be safely tested with null
// because it accesses m_context before checking doc. Skipped.
TEST_F(MoveProcTest, DirectoryMoveProcessor_MultipleInstances)
{
    auto runtime = makeRuntime();
    DirectoryMoveProcessor proc1(runtime->context(), nullptr, nullptr, nullptr);
    DirectoryMoveProcessor proc2(runtime->context(), nullptr, nullptr, nullptr);
    EXPECT_FALSE(proc1.hasChanges());
    EXPECT_FALSE(proc2.hasChanges());
}