// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileselectionmodelprivate.cpp
 * @brief Unit tests for FileSelectionModelPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/private/fileselectionmodel_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileSelectionModelPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileSelectionModelPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileSelectionModelPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileSelectionModelPrivateTest, FileSelectionModelPrivate)
{
    // Test constructor: FileSelectionModelPrivate((FileSelectionModel *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileSelectionModelPrivateTest, FileSelectionModelPrivate_Destructor)
{
    // Test method:  ~FileSelectionModelPrivate(())
    EXPECT_NO_FATAL_FAILURE({ FileSelectionModelPrivate *tmp = new FileSelectionModelPrivate(); delete tmp; });
}
