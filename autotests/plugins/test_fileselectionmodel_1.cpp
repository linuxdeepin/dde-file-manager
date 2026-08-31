// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileselectionmodel_1.cpp
 * @brief Unit tests for FileSelectionModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/fileselectionmodel.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileSelectionModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileSelectionModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileSelectionModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileSelectionModelTest, selectedCount)
{
    // Test getter: int selectedCount()
    auto result = obj->selectedCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileSelectionModelTest, updateSelecteds)
{
    // Test method: void updateSelecteds(())
    EXPECT_NO_FATAL_FAILURE(obj->updateSelecteds());
}
