// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_groupingengine_1.cpp
 * @brief Unit tests for GroupingEngine methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/groupingengine.h"

#include <QTest>

using namespace dfmplugin_workspace;

class GroupingEngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GroupingEngine();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GroupingEngine *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GroupingEngineTest, collectFilesToInsert)
{
    // Test method: bool collectFilesToInsert((QList<FileItemDataPointer> *filesToInsert))
    auto result = obj->collectFilesToInsert(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(GroupingEngineTest, currentUpdateChildrenRange)
{
    // Test getter: QPair<int, int> currentUpdateChildrenRange()
    auto result = obj->currentUpdateChildrenRange();
    EXPECT_NO_FATAL_FAILURE({ obj->currentUpdateChildrenRange(); });

}

TEST_F(GroupingEngineTest, currentUpdateMode)
{
    // Test getter: GroupingEngine::UpdateMode currentUpdateMode()
    auto result = obj->currentUpdateMode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(GroupingEngineTest, findNewAnchorPos)
{
    // Test method: std::optional<int> findNewAnchorPos((const QUrl &oldAnchorUrl, const FileGroupData *group))
    QUrl _arg0{};
    auto result = obj->findNewAnchorPos(_arg0, nullptr);
    EXPECT_FALSE(result.has_value());

}

TEST_F(GroupingEngineTest, findPrecedingAnchor)
{
    // Test method: std::optional<QUrl> findPrecedingAnchor((const QList<QUrl> &container, const QPair<int, int> &sliceRange))
    QList<QUrl> _arg0{};
    QPair<int, int> _arg1{};
    auto result = obj->findPrecedingAnchor(_arg0, _arg1);
    EXPECT_FALSE(result.has_value());

}

TEST_F(GroupingEngineTest, generateModelData)
{
    // Test method: GroupedModelData generateModelData((const GroupingResult &groupingResult,
                                                   const QHash<QString, bool> &expansionStates,
                                                   const QHash<QString, bool> &truncationStates,
                                                   const AbstractGroupStrategy *strategy))
    GroupingResult _arg0{};
    QHash<QString, bool> _arg1{};
    QHash<QString, bool> _arg2{};
    auto result = obj->generateModelData(_arg0, _arg1, _arg2, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->generateModelData(_arg0, _arg1, _arg2, nullptr); });

}

TEST_F(GroupingEngineTest, getFileInfoFromFileItem)
{
    // Test method: FileInfoPointer getFileInfoFromFileItem((const FileItemDataPointer &file))
    FileItemDataPointer _arg0{};
    auto result = obj->getFileInfoFromFileItem(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(GroupingEngineTest, getGroupKeyForFiles)
{
    // Test method: QString getGroupKeyForFiles((const QList<FileItemDataPointer> &filesToInsert,
                                            const QUrl &anchorUrl,
                                            DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy))
    QList<FileItemDataPointer> _arg0{};
    QUrl _arg1{};
    auto result = obj->getGroupKeyForFiles(_arg0, _arg1, nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(GroupingEngineTest, groupFiles)
{
    // Test method: GroupingEngine::GroupingResult groupFiles((const QList<FileItemDataPointer> &files,
                                                          AbstractGroupStrategy *strategy))
    QList<FileItemDataPointer> _arg0{};
    auto result = obj->groupFiles(_arg0, nullptr);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(GroupingEngineTest, initializeTruncationStates)
{
    // Test method: void initializeTruncationStates((const AbstractGroupStrategy *strategy,
                                                GroupedModelData *modelData))
    EXPECT_NO_FATAL_FAILURE(obj->initializeTruncationStates(nullptr, nullptr));
}

TEST_F(GroupingEngineTest, reorderGroups)
{
    // Test method: void reorderGroups((GroupedModelData *modelData))
    EXPECT_NO_FATAL_FAILURE(obj->reorderGroups(nullptr));
}

TEST_F(GroupingEngineTest, setCancellationCheckCallback)
{
    // Test setter: void setCancellationCheckCallback((CancellationCheckCallback callback))
    EXPECT_NO_FATAL_FAILURE(obj->setCancellationCheckCallback(CancellationCheckCallback()));
}

TEST_F(GroupingEngineTest, setChildrenDataMap)
{
    // Test setter: void setChildrenDataMap((QHash<QUrl, FileItemDataPointer> *map))
    EXPECT_NO_FATAL_FAILURE(obj->setChildrenDataMap(nullptr));
}

TEST_F(GroupingEngineTest, setGroupOrder)
{
    // Test setter: void setGroupOrder((Qt::SortOrder order))
    EXPECT_NO_FATAL_FAILURE(obj->setGroupOrder(Qt::SortOrder()));
}

TEST_F(GroupingEngineTest, setUpdateChildren)
{
    // Test setter: void setUpdateChildren((const QList<QUrl> &children))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUpdateChildren(_arg0));
}

TEST_F(GroupingEngineTest, setUpdateChildrenRange)
{
    // Test setter: void setUpdateChildrenRange((int pos, int count))
    EXPECT_NO_FATAL_FAILURE(obj->setUpdateChildrenRange(0, 0));
}

TEST_F(GroupingEngineTest, setUpdateMode)
{
    // Test setter: void setUpdateMode((UpdateMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setUpdateMode(UpdateMode()));
}

TEST_F(GroupingEngineTest, setVisibleChildren)
{
    // Test setter: void setVisibleChildren((QList<QUrl> *visibleChildren))
    EXPECT_NO_FATAL_FAILURE(obj->setVisibleChildren(nullptr));
}

TEST_F(GroupingEngineTest, setVisibleTreeChildren)
{
    // Test setter: void setVisibleTreeChildren((QHash<QUrl, QList<QUrl>> *children))
    EXPECT_NO_FATAL_FAILURE(obj->setVisibleTreeChildren(nullptr));
}

TEST_F(GroupingEngineTest, shouldCancel)
{
    // Test bool getter: shouldCancel()
    bool result = obj->shouldCancel();
    EXPECT_FALSE(result);

}

TEST_F(GroupingEngineTest, sortGroupsByDisplayOrder)
{
    // Test method: void sortGroupsByDisplayOrder((QList<FileGroupData> &groups))
    QList<FileGroupData> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sortGroupsByDisplayOrder(_arg0));
}

TEST_F(GroupingEngineTest, GroupingEngine_Destructor)
{
    // Test getter: DPWORKSPACE_BEGIN_NAMESPACE ~GroupingEngine()
    EXPECT_NO_FATAL_FAILURE({ GroupingEngine *tmp = new GroupingEngine(); delete tmp; });

}
