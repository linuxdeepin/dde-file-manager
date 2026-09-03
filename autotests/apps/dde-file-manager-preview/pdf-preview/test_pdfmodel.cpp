// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model.h"
#include "pdfmodel.h"
#include "ut_common.h"

#include <gtest/gtest.h>

#include <QImage>
#include <QSizeF>

using namespace plugin_filepreview;

class UT_PdfModel : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        Document::Error error = Document::kFileError;
        document = PDFDocument::loadDocument(pdfPath, "", error);
        ASSERT_NE(nullptr, document);
        ASSERT_EQ(Document::kNoError, error);
    }

    virtual void TearDown() override
    {
        delete document;
        document = nullptr;
        ut_utils::drainEvents();
    }

protected:
    QString pdfPath;
    PDFDocument *document { nullptr };
};

TEST_F(UT_PdfModel, LoadDocument_ValidPdf_ReturnsDocumentWithErrorCleared)
{
    Document::Error error = Document::kFileError;
    PDFDocument *doc = PDFDocument::loadDocument(pdfPath, "", error);
    EXPECT_NE(nullptr, doc);
    EXPECT_EQ(Document::kNoError, error);
    delete doc;
}

TEST_F(UT_PdfModel, LoadDocument_NonExistentFile_ReturnsNullWithFileError)
{
    Document::Error error = Document::kNoError;
    PDFDocument *doc = PDFDocument::loadDocument("/nonexistent-dir/no-such.pdf", "", error);
    EXPECT_EQ(nullptr, doc);
    EXPECT_EQ(Document::kFileError, error);
}

TEST_F(UT_PdfModel, LoadDocument_NotPdfContent_ReturnsNullWithFileError)
{
    QString textPath = ut_utils::prepareFile("/etc/hostname");
    Document::Error error = Document::kNoError;
    PDFDocument *doc = PDFDocument::loadDocument(textPath, "", error);
    EXPECT_EQ(nullptr, doc);
    EXPECT_EQ(Document::kFileError, error);
}

TEST_F(UT_PdfModel, LoadDocument_ValidPdfWithPassword_ReturnsDocument)
{
    Document::Error error = Document::kFileError;
    PDFDocument *doc = PDFDocument::loadDocument(pdfPath, "whatever", error);
    EXPECT_NE(nullptr, doc);
    EXPECT_EQ(Document::kNoError, error);
    delete doc;
}

TEST_F(UT_PdfModel, DocumentFactory_PdfType_ReturnsPdfDocument)
{
    Document::Error error = Document::kFileError;
    Document *doc = DocumentFactory::getDocument(kPDF, pdfPath, "", error);
    EXPECT_NE(nullptr, doc);
    EXPECT_EQ(Document::kNoError, error);
    delete doc;
}

TEST_F(UT_PdfModel, DocumentFactory_NonPdfType_ReturnsNull)
{
    Document::Error error = Document::kNoError;
    Document *doc = DocumentFactory::getDocument(kDJVU, pdfPath, "", error);
    EXPECT_EQ(nullptr, doc);
    delete doc;
}

TEST_F(UT_PdfModel, PageCount_MultiPagePdf_ReturnsPositiveCount)
{
    EXPECT_GE(document->pageCount(), 2);
}

TEST_F(UT_PdfModel, Page_ValidIndex_ReturnsPageWithPositiveSize)
{
    Page *page = document->page(0);
    ASSERT_NE(nullptr, page);
    QSizeF size = page->sizeF();
    EXPECT_GT(size.width(), 0.0);
    EXPECT_GT(size.height(), 0.0);
    delete page;
}

TEST_F(UT_PdfModel, Page_OutOfRangeIndex_ReturnsNull)
{
    EXPECT_EQ(nullptr, document->page(document->pageCount()));
    EXPECT_EQ(nullptr, document->page(-1));
}

TEST_F(UT_PdfModel, PageRender_FullPage_ReturnsNonEmptyImage)
{
    Page *page = document->page(0);
    ASSERT_NE(nullptr, page);
    QImage image = page->render(120, 160);
    EXPECT_FALSE(image.isNull());
    delete page;
}

TEST_F(UT_PdfModel, PageRender_WithSlice_ReturnsNonEmptyImage)
{
    Page *page = document->page(0);
    ASSERT_NE(nullptr, page);
    QImage image = page->render(200, 260, QRect(10, 10, 60, 80));
    EXPECT_FALSE(image.isNull());
    delete page;
}

TEST_F(UT_PdfModel, PageRender_LargeScale_ReturnsNonEmptyImage)
{
    Page *page = document->page(0);
    ASSERT_NE(nullptr, page);
    QImage image = page->render(600, 800);
    EXPECT_FALSE(image.isNull());
    delete page;
}

TEST_F(UT_PdfModel, DocumentDestructor_AfterPageFetch_NoCrash)
{
    Document::Error error = Document::kFileError;
    PDFDocument *doc = PDFDocument::loadDocument(pdfPath, "", error);
    ASSERT_NE(nullptr, doc);
    Page *page = doc->page(0);
    ASSERT_NE(nullptr, page);
    QImage image = page->render(50, 60);
    EXPECT_FALSE(image.isNull());
    delete page;
    delete doc;
}
