// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "pdfpreviewplugin.h"


class TestPDFPreviewPlugin : public testing::Test {
public:

    void SetUp() override
    {
        m_pdfPreviewPlugin = new PDFPreviewPlugin();
    }

    void TearDown() override
    {
        delete m_pdfPreviewPlugin;
        m_pdfPreviewPlugin = nullptr;
    }

public:
    PDFPreviewPlugin * m_pdfPreviewPlugin;
};

TEST_F(TestPDFPreviewPlugin, use_create)
{
    auto *ptr = m_pdfPreviewPlugin->create("dde-file-manager");
    EXPECT_TRUE(ptr != nullptr);
    if (ptr) {
        delete ptr;
        ptr = nullptr;
    }
}
