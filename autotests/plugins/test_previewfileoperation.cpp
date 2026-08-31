// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_previewfileoperation.cpp
 * @brief Unit tests for PreviewFileOperation methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/libdfm-preview/utils/previewfileoperation.h"

#include <QTest>

using namespace src;

class PreviewFileOperationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PreviewFileOperation();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PreviewFileOperation *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PreviewFileOperationTest, openFileHandle)
{
    // Test method: bool openFileHandle((quint64 winID, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->openFileHandle(0, _arg1);
    EXPECT_FALSE(result);

}
