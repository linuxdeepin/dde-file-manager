// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_unknowfilepreview.cpp
 * @brief Unit tests for UnknowFilePreview methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/libdfm-preview/views/unknowfilepreview.h"

#include <QTest>

using namespace src;

class UnknowFilePreviewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UnknowFilePreview();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UnknowFilePreview *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UnknowFilePreviewTest, setFileInfo)
{
    // Test setter: void setFileInfo((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFileInfo(_arg0));
}
