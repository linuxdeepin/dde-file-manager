/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-27

 */

#include <gtest/gtest.h>

#include <QDir>

#include "pdfwidget.h"
#include "pdfpreview.h"


DFM_BEGIN_NAMESPACE
namespace  {
class TestPDFpreview : public testing::Test
{
public:
    void SetUp() override
    {
        m_url = DUrl("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/tests/test.pdf");
        m_pdfPreview = new PDFPreview(nullptr);
    }

    void TearDown() override
    {
        delete m_pdfPreview;
        m_pdfPreview = nullptr;
    }

public:
    PDFPreview * m_pdfPreview;
    DUrl m_url;
};
}

TEST_F(TestPDFpreview, set_file_Url)
{
    EXPECT_TRUE(m_pdfPreview->setFileUrl(m_url));
}

TEST_F(TestPDFpreview, get_file_url)
{
    EXPECT_TRUE(m_pdfPreview->setFileUrl(m_url));
    EXPECT_TRUE(!m_pdfPreview->fileUrl().isValid());
}

TEST_F(TestPDFpreview, get_content_widget)
{
    EXPECT_TRUE(m_pdfPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_pdfPreview->contentWidget());
}

TEST_F(TestPDFpreview, get_title)
{
    EXPECT_TRUE(m_pdfPreview->setFileUrl(m_url));
    EXPECT_TRUE(!m_pdfPreview->title().isEmpty());
}

TEST_F(TestPDFpreview, get_show_statusbar_separator)
{
    EXPECT_TRUE(m_pdfPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_pdfPreview->showStatusBarSeparator());
}

DFM_END_NAMESPACE
