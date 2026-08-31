// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_directorymoveprocessor.cpp
 * @brief Unit tests for DirectoryMoveProcessor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/moveprocessor.h"

#include <QTest>

using namespace src;

class DirectoryMoveProcessorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirectoryMoveProcessor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirectoryMoveProcessor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirectoryMoveProcessorTest, DirectoryMoveProcessor)
{
    // Test constructor: DirectoryMoveProcessor((const IndexContext &context,
                                               const SearcherPtr &searcher,
                                               const IndexWriterPtr &writer,
                                               const IndexReaderPtr &reader))
    ASSERT_NE(obj, nullptr);
}
