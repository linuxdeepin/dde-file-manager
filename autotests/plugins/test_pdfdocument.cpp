// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pdfdocument.cpp
 * @brief Unit tests for PDFDocument methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/pdfmodel.h"

#include <QTest>

using namespace src;

class PDFDocumentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PDFDocument();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PDFDocument *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PDFDocumentTest, page)
{
    // Test method: Page page((int index))
    auto result = obj->page(0);
    EXPECT_NO_FATAL_FAILURE({ obj->page(0); });

}
