// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_domovetotrashfilesworker.cpp
 * @brief Unit tests for DoMoveToTrashFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/trashfiles/domovetotrashfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoMoveToTrashFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoMoveToTrashFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoMoveToTrashFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoMoveToTrashFilesWorkerTest, canWriteFile)
{
    // Test method: bool canWriteFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->canWriteFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DoMoveToTrashFilesWorkerTest, trashTargetUrl)
{
    // Test method: QUrl trashTargetUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->trashTargetUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}
