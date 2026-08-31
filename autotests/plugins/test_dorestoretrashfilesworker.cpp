// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dorestoretrashfilesworker.cpp
 * @brief Unit tests for DoRestoreTrashFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/trashfiles/dorestoretrashfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoRestoreTrashFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoRestoreTrashFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoRestoreTrashFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoRestoreTrashFilesWorkerTest, createParentDir)
{
    // Test method: DFileInfoPointer createParentDir((const QUrl &fromUrl,
                                                            const DFileInfoPointer &restoreInfo,
                                                            bool *result))
    QUrl _arg0{};
    DFileInfoPointer _arg1{};
    auto result = obj->createParentDir(_arg0, _arg1, nullptr);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(DoRestoreTrashFilesWorkerTest, statisticsFilesSize)
{
    // Test bool getter: statisticsFilesSize()
    bool result = obj->statisticsFilesSize();
    EXPECT_FALSE(result);

}
