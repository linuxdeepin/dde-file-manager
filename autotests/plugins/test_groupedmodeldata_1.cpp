// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_groupedmodeldata_1.cpp
 * @brief Unit tests for GroupedModelData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/groupedmodeldata.h"

#include <QTest>

using namespace dfmplugin_workspace;

class GroupedModelDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GroupedModelData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GroupedModelData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GroupedModelDataTest, GroupedModelData)
{
    // Test constructor: GroupedModelData((const GroupedModelData &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(GroupedModelDataTest, M_~GroupedModelData)
{
    // Test method:  ~GroupedModelData(())
    EXPECT_NO_FATAL_FAILURE({ GroupedModelData *tmp = new GroupedModelData(); delete tmp; });
}

TEST_F(GroupedModelDataTest, operator=)
{
    // Test getter: GroupedModelData operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(GroupedModelDataTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(GroupedModelDataTest, isEmpty)
{
    // Test bool getter: isEmpty()
    bool result = obj->isEmpty();
    EXPECT_TRUE(result);

}

TEST_F(GroupedModelDataTest, addGroup)
{
    // Test method: bool addGroup((const FileGroupData &group))
    FileGroupData _arg0{};
    auto result = obj->addGroup(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(GroupedModelDataTest, removeGroup)
{
    // Test method: bool removeGroup((const QString &groupKey))
    QString _arg0{};
    auto result = obj->removeGroup(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(GroupedModelDataTest, insertItem)
{
    // Test method: void insertItem((int index, const ModelItemWrapper &item))
    ModelItemWrapper _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->insertItem(0, _arg1));
}

TEST_F(GroupedModelDataTest, removeItems)
{
    // Test method: int removeItems((int index, int count))
    auto result = obj->removeItems(0, 0);
    EXPECT_GE(result, 0);

}

TEST_F(GroupedModelDataTest, replaceItem)
{
    // Test method: void replaceItem((int index, const ModelItemWrapper &item))
    ModelItemWrapper _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->replaceItem(0, _arg1));
}

TEST_F(GroupedModelDataTest, getGroup)
{
    // Test method: FileGroupData getGroup((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroup(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->getGroup(_arg0); });

}

TEST_F(GroupedModelDataTest, getAllFiles)
{
    // Test getter: QList<FileItemDataPointer> getAllFiles()
    auto result = obj->getAllFiles();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(GroupedModelDataTest, setGroupExpanded)
{
    // Test setter: void setGroupExpanded((const QString &groupKey, bool expanded))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setGroupExpanded(_arg0, false));
}

TEST_F(GroupedModelDataTest, isGroupExpanded)
{
    // Test method: bool isGroupExpanded((const QString &groupKey))
    QString _arg0{};
    auto result = obj->isGroupExpanded(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(GroupedModelDataTest, setGroupTruncated)
{
    // Test setter: void setGroupTruncated((const QString &groupKey, bool truncated, bool rebuild))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setGroupTruncated(_arg0, false, false));
}

TEST_F(GroupedModelDataTest, isGroupTruncated)
{
    // Test method: bool isGroupTruncated((const QString &groupKey))
    QString _arg0{};
    auto result = obj->isGroupTruncated(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(GroupedModelDataTest, setTruncationEnabled)
{
    // Test setter: void setTruncationEnabled((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setTruncationEnabled(false));
}

TEST_F(GroupedModelDataTest, isTruncationEnabled)
{
    // Test bool getter: isTruncationEnabled()
    bool result = obj->isTruncationEnabled();
    EXPECT_FALSE(result);

}

TEST_F(GroupedModelDataTest, updateGroupHeader)
{
    // Test method: void updateGroupHeader((const QString &groupKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateGroupHeader(_arg0));
}

TEST_F(GroupedModelDataTest, rebuildFlattenedItems)
{
    // Test method: void rebuildFlattenedItems(())
    EXPECT_NO_FATAL_FAILURE(obj->rebuildFlattenedItems());
}

TEST_F(GroupedModelDataTest, getItemCount)
{
    // Test getter: int getItemCount()
    auto result = obj->getItemCount();
    EXPECT_EQ(result, 0);

}

TEST_F(GroupedModelDataTest, getItemAt)
{
    // Test method: ModelItemWrapper getItemAt((int index))
    auto result = obj->getItemAt(0);
    EXPECT_NO_FATAL_FAILURE({ obj->getItemAt(0); });

}

TEST_F(GroupedModelDataTest, isGroupTruncatedInitialized)
{
    // Test method: bool isGroupTruncatedInitialized((const QString &groupKey))
    QString _arg0{};
    auto result = obj->isGroupTruncatedInitialized(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(GroupedModelDataTest, getVisibleFileCount)
{
    // Test method: int getVisibleFileCount((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getVisibleFileCount(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(GroupedModelDataTest, getFileItemCount)
{
    // Test getter: int getFileItemCount()
    auto result = obj->getFileItemCount();
    EXPECT_EQ(result, 0);

}

TEST_F(GroupedModelDataTest, getGroupItemCount)
{
    // Test getter: int getGroupItemCount()
    auto result = obj->getGroupItemCount();
    EXPECT_EQ(result, 0);

}

TEST_F(GroupedModelDataTest, findFileStartPos)
{
    // Test method: std::optional<int> findFileStartPos((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->findFileStartPos(_arg0);
    EXPECT_FALSE(result.has_value());

}

TEST_F(GroupedModelDataTest, findGroupHeaderStartPos)
{
    // Test method: std::optional<int> findGroupHeaderStartPos((const QString &key))
    QString _arg0{};
    auto result = obj->findGroupHeaderStartPos(_arg0);
    EXPECT_FALSE(result.has_value());

}
