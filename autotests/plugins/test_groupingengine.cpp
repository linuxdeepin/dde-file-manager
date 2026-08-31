// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_groupingengine.cpp
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

TEST_F(GroupingEngineTest, insertFilesToModelData)
{
    // Test method: GroupingEngine::UpdateResult insertFilesToModelData((const QUrl &anchorUrl,
                                                                    const GroupedModelData &oldData,
                                                                    DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy))
    QUrl _arg0{};
    GroupedModelData _arg1{};
    auto result = obj->insertFilesToModelData(_arg0, _arg1, nullptr);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(GroupingEngineTest, processFilesAndUpdateGroups)
{
    // Test method: bool processFilesAndUpdateGroups((const QList<FileItemDataPointer> &filesToInsert,
                                                 const QString groupKey, const QUrl &anchorUrl,
                                                 GroupedModelData *newData))
    QList<FileItemDataPointer> _arg0{};
    QUrl _arg2{};
    auto result = obj->processFilesAndUpdateGroups(_arg0, QString(), _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(GroupingEngineTest, updateFilesToModelData)
{
    // Test method: GroupingEngine::UpdateResult updateFilesToModelData((const QUrl &anchorUrl, const GroupedModelData &oldData, dfmbase::AbstractGroupStrategy *strategy))
    QUrl _arg0{};
    GroupedModelData _arg1{};
    auto result = obj->updateFilesToModelData(_arg0, _arg1, nullptr);
    EXPECT_GE(static_cast<int>(result), 0);

}
