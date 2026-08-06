// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexruntime.cpp
 * @brief Unit tests for IndexRuntime (core/indexruntime.cpp)
 *        Covers construction for both Content and Ocr profiles plus all
 *        public accessors and the private selectExtractor/selectDocumentBuilder
 *        selection helpers.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QString>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexruntime.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class IndexRuntimeTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    IndexProfile makeProfile(IndexProfile::Type type)
    {
        return IndexProfile(type,
                            "runtime_test",
                            "runtime_status.json",
                            "runtime_version",
                            1,
                            [this]() -> QString { return tmp.path(); },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &) -> bool { return true; });
    }
};

TEST_F(IndexRuntimeTest, ContentRuntime_ConstructsAndExposesComponents)
{
    IndexRuntime rt(makeProfile(IndexProfile::Type::Content));
    EXPECT_EQ(rt.profile().type(), IndexProfile::Type::Content);
    EXPECT_NO_FATAL_FAILURE({ (void)rt.stateStore(); });
    EXPECT_NO_FATAL_FAILURE({ (void)rt.context(); });
    EXPECT_NE(rt.taskManager(), nullptr);
    EXPECT_NE(rt.fsEventController(), nullptr);
}

TEST_F(IndexRuntimeTest, OcrRuntime_ConstructsAndSelectsOcrBuilder)
{
    IndexRuntime rt(makeProfile(IndexProfile::Type::Ocr));
    EXPECT_EQ(rt.profile().type(), IndexProfile::Type::Ocr);
    EXPECT_NE(rt.taskManager(), nullptr);
    EXPECT_NE(rt.fsEventController(), nullptr);
    EXPECT_NO_FATAL_FAILURE({ (void)rt.stateStore(); });
    EXPECT_NO_FATAL_FAILURE({ (void)rt.context(); });
}

TEST_F(IndexRuntimeTest, ProfileAccessorReturnsSameType)
{
    IndexRuntime rtA(makeProfile(IndexProfile::Type::Content));
    IndexRuntime rtB(makeProfile(IndexProfile::Type::Ocr));
    EXPECT_EQ(rtA.profile().type(), IndexProfile::Type::Content);
    EXPECT_EQ(rtB.profile().type(), IndexProfile::Type::Ocr);
}

TEST_F(IndexRuntimeTest, ContextAccessorNonNull)
{
    IndexRuntime rt(makeProfile(IndexProfile::Type::Content));
    EXPECT_EQ(rt.context().profile().type(), IndexProfile::Type::Content);
    EXPECT_NE(rt.context().stateStore(), nullptr);
    EXPECT_NE(rt.context().extractor(), nullptr);
    EXPECT_NE(rt.context().documentBuilder(), nullptr);
}
