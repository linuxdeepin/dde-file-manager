// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pdfwidget.cpp
 * @brief Unit tests for PdfWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/pdfwidget.h"

#include <QTest>

using namespace src;

class PdfWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PdfWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PdfWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PdfWidgetTest, onOpened)
{
    // Test method: void onOpened((DocSheet *sheet, Document::Error error))
    EXPECT_NO_FATAL_FAILURE(obj->onOpened(nullptr, Document::Error()));
}
