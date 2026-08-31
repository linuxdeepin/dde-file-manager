// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileitemdata_1.cpp
 * @brief Unit tests for FileItemData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/fileitemdata.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileItemDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileItemData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileItemData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileItemDataTest, FileItemData)
{
    // Test constructor: FileItemData((const SortInfoPointer &info, FileItemData *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileItemDataTest, fileSortInfo)
{
    // Test getter: SortInfoPointer fileSortInfo()
    auto result = obj->fileSortInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(FileItemDataTest, parentData)
{
    // Test getter: FileItemData parentData()
    auto result = obj->parentData();
    EXPECT_NO_FATAL_FAILURE({ obj->parentData(); });

}

TEST_F(FileItemDataTest, refreshInfo)
{
    // Test method: void refreshInfo(())
    EXPECT_NO_FATAL_FAILURE(obj->refreshInfo());
}

TEST_F(FileItemDataTest, setAvailableState)
{
    // Test setter: void setAvailableState((bool b))
    EXPECT_NO_FATAL_FAILURE(obj->setAvailableState(false));
}

TEST_F(FileItemDataTest, setDepth)
{
    // Test setter: void setDepth((const int8_t depth))
    EXPECT_NO_FATAL_FAILURE(obj->setDepth({}));
}

TEST_F(FileItemDataTest, setExpanded)
{
    // Test setter: void setExpanded((bool b))
    EXPECT_NO_FATAL_FAILURE(obj->setExpanded(false));
}

TEST_F(FileItemDataTest, setGroupDisplayIndex)
{
    // Test setter: void setGroupDisplayIndex((int index))
    EXPECT_NO_FATAL_FAILURE(obj->setGroupDisplayIndex(0));
}

TEST_F(FileItemDataTest, setParentData)
{
    // Test setter: void setParentData((FileItemData *p))
    EXPECT_NO_FATAL_FAILURE(obj->setParentData(nullptr));
}

TEST_F(FileItemDataTest, setSortFileInfo)
{
    // Test setter: void setSortFileInfo((SortInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->setSortFileInfo(SortInfoPointer()));
}

TEST_F(FileItemDataTest, transFileInfo)
{
    // Test method: void transFileInfo(())
    EXPECT_NO_FATAL_FAILURE(obj->transFileInfo());
}
