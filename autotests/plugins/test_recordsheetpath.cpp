// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recordsheetpath.cpp
 * @brief Unit tests for recordSheetPath methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/pdfwidget.h"

#include <QTest>

using namespace src;

class recordSheetPathTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new recordSheetPath();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    recordSheetPath *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(recordSheetPathTest, indexOfFilePath)
{
    // Test method: int indexOfFilePath((const QString &filePath))
    QString _arg0{};
    auto result = obj->indexOfFilePath(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(recordSheetPathTest, removeSheet)
{
    // Test method: void removeSheet((DocSheet *sheet))
    EXPECT_NO_FATAL_FAILURE(obj->removeSheet(nullptr));
}
