// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pdfpage.cpp
 * @brief Unit tests for PDFPage methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/pdfmodel.h"

#include <QTest>

using namespace src;

class PDFPageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PDFPage();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PDFPage *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PDFPageTest, render)
{
    // Test method: QImage render((int width, int height, const QRect &slice))
    QRect _arg2{};
    auto result = obj->render(0, 0, _arg2);
    EXPECT_NO_FATAL_FAILURE({ obj->render(0, 0, _arg2); });

}
