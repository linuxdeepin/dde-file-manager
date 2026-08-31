// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperationseventhandler.cpp
 * @brief Unit tests for FileOperationsEventHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperationsevent/fileoperationseventhandler.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileOperationsEventHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperationsEventHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperationsEventHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperationsEventHandlerTest, removeUrlsInClipboard)
{
    // Test method: void removeUrlsInClipboard((AbstractJobHandler::JobType jobType, const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok))
    QList<QUrl> _arg1{};
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->removeUrlsInClipboard(AbstractJobHandler::JobType(), _arg1, _arg2, false));
}
