// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexruntime_extra.cpp
 * @brief Additional tests for IndexRuntime covering:
 *        - context() returns correct extractor/builder for each profile type
 *        - profile() and stateStore() accessors
 *        - taskManager() and fsEventController() non-null
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>

#include "stubext.h"
#include <dfm-search/dsearch_global.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexruntime.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

class IndexRuntimeExtraTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());

        stub.set_lamda(ADDR(Global, contentIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/content";
                       });
        stub.set_lamda(ADDR(Global, isContentIndexAvailable),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
        stub.set_lamda(ADDR(Global, isPathInContentIndexDirectory),
                       [this](const QString &path) -> bool {
                           __DBG_STUB_INVOKE__
                           return path.startsWith(tmp.path());
                       });
        stub.set_lamda(ADDR(Global, defaultIndexedDirectory),
                       [this]() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList { tmp.path() };
                       });
        stub.set_lamda(ADDR(Global, defaultBlacklistPaths),
                       []() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });
    }

    QTemporaryDir ocrTmp;
    stub_ext::StubExt stub;
};

TEST_F(IndexRuntimeExtraTest, ContentProfile_ContextHasExtractor)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::content());
    const auto &ctx = runtime->context();
    EXPECT_NE(ctx.extractor(), nullptr);
    EXPECT_NE(ctx.documentBuilder(), nullptr);
    EXPECT_NE(ctx.stateStore(), nullptr);
}

TEST_F(IndexRuntimeExtraTest, ContentProfile_TaskManagerNotNull)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::content());
    EXPECT_NE(runtime->taskManager(), nullptr);
}

TEST_F(IndexRuntimeExtraTest, ContentProfile_FSEventControllerNotNull)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::content());
    EXPECT_NE(runtime->fsEventController(), nullptr);
}

TEST_F(IndexRuntimeExtraTest, ContentProfile_ProfileAccessor)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::content());
    const auto &profile = runtime->profile();
    EXPECT_EQ(profile.type(), IndexProfile::Type::Content);
    EXPECT_FALSE(profile.id().isEmpty());
    EXPECT_GE(profile.runtimeIndexVersion(), 1);
}

TEST_F(IndexRuntimeExtraTest, ContentProfile_StateStoreAccessor)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::content());
    const auto &stateStore = runtime->stateStore();
    EXPECT_NO_FATAL_FAILURE({ (void)stateStore.isCompatibleVersion(); });
}

TEST_F(IndexRuntimeExtraTest, OcrProfile_ContextHasExtractor)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::ocr());
    const auto &ctx = runtime->context();
    EXPECT_NE(ctx.extractor(), nullptr);
    EXPECT_NE(ctx.documentBuilder(), nullptr);
    EXPECT_NE(ctx.stateStore(), nullptr);
}

TEST_F(IndexRuntimeExtraTest, OcrProfile_TaskManagerNotNull)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::ocr());
    EXPECT_NE(runtime->taskManager(), nullptr);
}

TEST_F(IndexRuntimeExtraTest, OcrProfile_FSEventControllerNotNull)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::ocr());
    EXPECT_NE(runtime->fsEventController(), nullptr);
}

TEST_F(IndexRuntimeExtraTest, OcrProfile_ProfileAccessor)
{
    auto runtime = std::make_unique<IndexRuntime>(IndexProfile::ocr());
    const auto &profile = runtime->profile();
    EXPECT_EQ(profile.type(), IndexProfile::Type::Ocr);
    EXPECT_FALSE(profile.id().isEmpty());
}

TEST_F(IndexRuntimeExtraTest, MultipleRuntimes_SameType)
{
    // Create two content runtimes - they should each have their own TaskManager
    auto runtime1 = std::make_unique<IndexRuntime>(IndexProfile::content());
    auto runtime2 = std::make_unique<IndexRuntime>(IndexProfile::content());
    EXPECT_NE(runtime1->taskManager(), runtime2->taskManager());
}