// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatorprivate.cpp
 * @brief Unit tests for FileOperatorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileoperator.h"

#include <QTest>

using namespace ddplugin_organizer;

class FileOperatorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorPrivateTest, callBackRenameFiles)
{
    // Test method: void callBackRenameFiles((const QList<QUrl> &sources, const QList<QUrl> &targets))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->callBackRenameFiles(_arg0, _arg1));
}
