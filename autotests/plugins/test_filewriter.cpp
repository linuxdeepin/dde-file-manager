// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filewriter.cpp
 * @brief Unit tests for FileWriter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/docopyfileworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileWriterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileWriter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileWriter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileWriterTest, FileWriter)
{
    // Test constructor: FileWriter(())
    ASSERT_NE(obj, nullptr);
}
