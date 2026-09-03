// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docsheet.h"
#include "pdfmodel.h"
#include "sheetrenderer.h"
#include "ut_common.h"

#include <gtest/gtest.h>

#include <QSignalSpy>

using namespace plugin_filepreview;

class UT_SheetRenderer : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        renderer = sheet->findChild<SheetRenderer *>();
        ASSERT_NE(nullptr, renderer);
    }

    virtual void TearDown() override
    {
        ut_utils::waitRenderIdle();
        delete sheet;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    SheetRenderer *renderer { nullptr };
};

TEST_F(UT_SheetRenderer, Construct_DefaultState)
{
    EXPECT_FALSE(renderer->opened());
    EXPECT_EQ(0, renderer->getPageCount());
}

TEST_F(UT_SheetRenderer, GetImage_BeforeOpen_ReturnsNull)
{
    EXPECT_TRUE(renderer->getImage(0, 50, 50).isNull());
}

TEST_F(UT_SheetRenderer, GetPageSize_BeforeOpen_ReturnsEmpty)
{
    EXPECT_EQ(QSizeF(), renderer->getPageSize(0));
    EXPECT_EQ(QSizeF(), renderer->getPageSize(5));
}

TEST_F(UT_SheetRenderer, HandleOpened_Error_KeepsClosed)
{
    QSignalSpy spy(renderer, SIGNAL(sigOpened(Document::Error)));
    renderer->handleOpened(Document::kFileError, nullptr, QList<Page *>());
    EXPECT_FALSE(renderer->opened());
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(Document::kFileError, qvariant_cast<Document::Error>(spy.at(0).at(0)));
}

TEST_F(UT_SheetRenderer, HandleOpened_Success_SetsDocumentAndPages)
{
    Document::Error error = Document::kFileError;
    PDFDocument *document = PDFDocument::loadDocument(pdfPath, "", error);
    ASSERT_NE(nullptr, document);
    QList<Page *> pages;
    for (int i = 0; i < document->pageCount(); ++i)
        pages.append(document->page(i));

    QSignalSpy spy(renderer, SIGNAL(sigOpened(Document::Error)));
    renderer->handleOpened(Document::kNoError, document, pages);
    EXPECT_TRUE(renderer->opened());
    EXPECT_EQ(pages.count(), renderer->getPageCount());
    EXPECT_EQ(1, spy.count());

    delete renderer->documentObj;
    qDeleteAll(renderer->pageList);
    renderer->documentObj = nullptr;
    renderer->pageList.clear();
}

TEST_F(UT_SheetRenderer, OpenFileExec_ValidPdf_ReturnsTrue)
{
    EXPECT_TRUE(renderer->openFileExec(""));
    EXPECT_TRUE(renderer->opened());
    EXPECT_GE(renderer->getPageCount(), 2);
}

TEST_F(UT_SheetRenderer, OpenFileAsync_ValidPdf_EmitsOpenedWithNoError)
{
    QSignalSpy spy(renderer, SIGNAL(sigOpened(Document::Error)));
    renderer->openFileAsync("");
    ut_utils::processEventsUntil([&spy]() { return spy.count() >= 1; });
    ASSERT_GE(spy.count(), 1);
    EXPECT_EQ(Document::kNoError, qvariant_cast<Document::Error>(spy.at(0).at(0)));
    EXPECT_TRUE(renderer->opened());
}

class UT_SheetRendererOpened : public UT_SheetRenderer
{
protected:
    void SetUp() override
    {
        UT_SheetRenderer::SetUp();
        ASSERT_TRUE(renderer->openFileExec(""));
    }
};

TEST_F(UT_SheetRendererOpened, GetPageSize_ValidIndex_ReturnsPositiveSize)
{
    QSizeF size = renderer->getPageSize(0);
    EXPECT_GT(size.width(), 0.0);
    EXPECT_GT(size.height(), 0.0);
}

TEST_F(UT_SheetRendererOpened, GetImage_ValidIndex_ReturnsRenderedImage)
{
    QImage image = renderer->getImage(0, 60, 80);
    EXPECT_FALSE(image.isNull());
}

TEST_F(UT_SheetRendererOpened, GetImage_OutOfRange_ReturnsNull)
{
    EXPECT_TRUE(renderer->getImage(renderer->getPageCount(), 60, 80).isNull());
}

TEST_F(UT_SheetRendererOpened, ReopenSameFile_SucceedsAgain)
{
    EXPECT_TRUE(renderer->openFileExec(""));
    EXPECT_TRUE(renderer->opened());
}

TEST_F(UT_SheetRendererOpened, Destructor_WithLoadedDocument_AppendsCloseTask)
{
    DocSheet *tempSheet = new DocSheet(kPDF, pdfPath);
    SheetRenderer *tempRenderer = tempSheet->findChild<SheetRenderer *>();
    ASSERT_NE(nullptr, tempRenderer);
    ASSERT_TRUE(tempRenderer->openFileExec(""));
    ut_utils::waitRenderIdle();
    delete tempSheet;
    ut_utils::waitRenderIdle();
    SUCCEED();
}
