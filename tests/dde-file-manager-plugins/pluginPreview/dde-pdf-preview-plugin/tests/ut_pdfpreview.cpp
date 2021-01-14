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
#include <QPrinter>
#include <QPainter>

#include "pdfwidget.h"
#include "pdfpreview.h"


DFM_BEGIN_NAMESPACE
namespace  {
class TestPDFpreview : public testing::Test
{
public:
    void SetUp() override
    {
        QPrinter text_printer;
        text_printer.setOutputFormat(QPrinter::PdfFormat);
        text_printer.setOutputFileName("./test.pdf");
        QPainter painter_pixmap;
        painter_pixmap.begin(&text_printer);
        painter_pixmap.drawText(10, 30, "hello world");
        painter_pixmap.end();

        m_url = DUrl("file:./test.pdf");
        m_pdfPreview = new PDFPreview(nullptr);
    }

    void TearDown() override
    {
        QThread::sleep(2);
        delete m_pdfPreview;
        m_pdfPreview = nullptr;
        QFile file("./test.pdf");
        file.remove();
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
