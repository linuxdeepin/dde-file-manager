// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docsheet.h"
#include "encryptionpage.h"
#include "sheetbrowser.h"
#include "sheetrenderer.h"
#include "sheetsidebar.h"
#include "sidebarimageviewmodel.h"
#include "thumbnailwidget.h"
#include "ut_common.h"

#include <gtest/gtest.h>

#include <QMimeDatabase>
#include <QMimeType>
#include <QScrollBar>
#include <QSignalSpy>
#include <QWidget>

using namespace plugin_filepreview;

class UT_DocSheet : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
    }

    virtual void TearDown() override
    {
        ut_utils::waitRenderIdle();
        delete sheet;
        sheet = nullptr;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
};

TEST_F(UT_DocSheet, Construct_RegistersSheetInGlobalList)
{
    EXPECT_TRUE(DocSheet::existSheet(sheet));
    EXPECT_TRUE(DocSheet::getSheets().contains(sheet));
    EXPECT_EQ(sheet, DocSheet::getSheetByFilePath(pdfPath));
}

TEST_F(UT_DocSheet, Construct_PdfType_CreatesThumbnailSidebar)
{
    EXPECT_NE(nullptr, sheet->findChild<SheetSidebar *>());
}

TEST_F(UT_DocSheet, Construct_PdfType_CreatesBrowserAndRenderer)
{
    EXPECT_NE(nullptr, sheet->findChild<SheetBrowser *>());
    EXPECT_NE(nullptr, sheet->renderer());
}

TEST_F(UT_DocSheet, Construct_UnknownFileType_CreatesSidebarWithoutThumbnails)
{
    DocSheet *unknownSheet = new DocSheet(kUnknown, "/tmp/unknown-file");
    SheetSidebar *sidebar = unknownSheet->findChild<SheetSidebar *>();
    ASSERT_NE(nullptr, sidebar);
    EXPECT_EQ(nullptr, sidebar->findChild<ThumbnailWidget *>());
    delete unknownSheet;
}

TEST_F(UT_DocSheet, Destructor_RemovesSheetFromGlobalList)
{
    DocSheet *temp = new DocSheet(kPDF, "/tmp/removed.pdf");
    ASSERT_TRUE(DocSheet::existSheet(temp));
    delete temp;
    EXPECT_FALSE(DocSheet::existSheet(temp));
    EXPECT_EQ(nullptr, DocSheet::getSheetByFilePath("/tmp/removed.pdf"));
}

TEST_F(UT_DocSheet, Opened_BeforeOpen_ReturnsFalse)
{
    EXPECT_FALSE(sheet->opened());
}

TEST_F(UT_DocSheet, PageCount_BeforeOpen_ReturnsZero)
{
    EXPECT_EQ(0, sheet->pageCount());
}

TEST_F(UT_DocSheet, CurrentPage_BeforeOpen_ClampsToOne)
{
    EXPECT_EQ(1, sheet->currentPage());
    EXPECT_EQ(0, sheet->currentIndex());
}

TEST_F(UT_DocSheet, Format_ReturnsPdf)
{
    EXPECT_EQ(QString("PDF"), sheet->format());
}

TEST_F(UT_DocSheet, Operation_DefaultValues)
{
    SheetOperation op = sheet->operation();
    EXPECT_EQ(SinglePageMode, op.layoutMode);
    EXPECT_EQ(kRotateBy0, op.rotation);
    EXPECT_DOUBLE_EQ(0.9, op.scaleFactor);
    EXPECT_EQ(1, op.currentPage);
    EXPECT_TRUE(op.sidebarVisible);
}

TEST_F(UT_DocSheet, FileType_ReturnsConstructorValue)
{
    EXPECT_EQ(kPDF, sheet->fileType());
}

TEST_F(UT_DocSheet, FilePath_ReturnsConstructorValue)
{
    EXPECT_EQ(pdfPath, sheet->filePath());
    EXPECT_EQ(pdfPath, sheet->openedFilePath());
}

TEST_F(UT_DocSheet, SetThumbnail_ThenThumbnailReturnsPixmap)
{
    QPixmap pixmap(10, 10);
    pixmap.fill(Qt::red);
    sheet->setThumbnail(3, pixmap);
    EXPECT_EQ(pixmap.toImage(), sheet->thumbnail(3).toImage());
}

TEST_F(UT_DocSheet, Thumbnail_UnknownIndex_ReturnsNull)
{
    EXPECT_TRUE(sheet->thumbnail(99).isNull());
}

TEST_F(UT_DocSheet, GetImage_BeforeOpen_ReturnsNull)
{
    EXPECT_TRUE(sheet->getImage(0, 50, 50).isNull());
}

TEST_F(UT_DocSheet, GetImage_InvalidIndex_ReturnsNull)
{
    EXPECT_TRUE(sheet->getImage(9999, 50, 50).isNull());
}

TEST_F(UT_DocSheet, PageSizeByIndex_BeforeOpen_ReturnsEmpty)
{
    EXPECT_EQ(QSizeF(), sheet->pageSizeByIndex(0));
}

TEST_F(UT_DocSheet, MaxScaleFactor_BeforeOpen_BoundedToFive)
{
    EXPECT_DOUBLE_EQ(5.0, sheet->maxScaleFactor());
}

TEST_F(UT_DocSheet, MaxScaleFactor_AlwaysWithinBounds)
{
    qreal factor = sheet->maxScaleFactor();
    EXPECT_GE(factor, 0.1);
    EXPECT_LE(factor, 5.0);
}

TEST_F(UT_DocSheet, ScaleFactorList_WithinMaxFactor)
{
    QList<qreal> factors = sheet->scaleFactorList();
    EXPECT_FALSE(factors.isEmpty());
    EXPECT_TRUE(factors.contains(1.0));
    qreal max = sheet->maxScaleFactor();
    foreach (qreal factor, factors) {
        EXPECT_LE(factor, max + 0.0001);
    }
}

TEST_F(UT_DocSheet, SetSidebarVisible_False_UpdatesOperation)
{
    sheet->setSidebarVisible(false);
    EXPECT_FALSE(sheet->operation().sidebarVisible);
    sheet->setSidebarVisible(true);
    EXPECT_TRUE(sheet->operation().sidebarVisible);
}

TEST_F(UT_DocSheet, SetSidebarVisible_NotifyFalse_KeepsOperation)
{
    sheet->setSidebarVisible(false, false);
    EXPECT_TRUE(sheet->operation().sidebarVisible);
}

TEST_F(UT_DocSheet, SetLayoutMode_TwoPages_UpdatesOperation)
{
    sheet->setLayoutMode(TwoPagesMode);
    EXPECT_EQ(TwoPagesMode, sheet->operation().layoutMode);
}

TEST_F(UT_DocSheet, SetLayoutMode_InvalidMode_Ignored)
{
    sheet->setLayoutMode(static_cast<LayoutMode>(99));
    EXPECT_EQ(SinglePageMode, sheet->operation().layoutMode);
}

TEST_F(UT_DocSheet, OpenFileExec_MissingFile_ReturnsFalse)
{
    DocSheet missing(kPDF, "/nonexistent-dir/no-such.pdf");
    EXPECT_FALSE(missing.openFileExec(""));
    EXPECT_FALSE(missing.opened());
}

TEST_F(UT_DocSheet, OpenFileExec_NotPdfContent_ReturnsFalse)
{
    QString textPath = ut_utils::prepareFile("/etc/hostname");
    DocSheet textSheet(kPDF, textPath);
    EXPECT_FALSE(textSheet.openFileExec(""));
}

TEST_F(UT_DocSheet, OpenFileAsync_MissingFile_EmitsOpenedWithError)
{
    qRegisterMetaType<Document::Error>("Document::Error");
    DocSheet missing(kPDF, "/nonexistent-dir/no-such.pdf");
    QSignalSpy spy(&missing, SIGNAL(sigFileOpened(DocSheet *, Document::Error)));
    missing.openFileAsync("");
    ut_utils::processEventsUntil([&spy]() { return spy.count() >= 1; });
    ASSERT_GE(spy.count(), 1);
    EXPECT_EQ(Document::kFileError, qvariant_cast<Document::Error>(spy.at(0).at(1)));
    EXPECT_FALSE(missing.opened());
}

TEST_F(UT_DocSheet, OnBrowserPageChanged_UpdatesOperationCurrentPage)
{
    sheet->onBrowserPageChanged(5);
    EXPECT_EQ(5, sheet->operation().currentPage);
    EXPECT_EQ(1, sheet->currentPage());
}

TEST_F(UT_DocSheet, OnBrowserPageChanged_InvalidPage_Clamped)
{
    sheet->onBrowserPageChanged(0);
    EXPECT_EQ(1, sheet->currentPage());
    EXPECT_EQ(0, sheet->currentIndex());
}

TEST_F(UT_DocSheet, JumpToPage_BeforeOpen_NoCrash)
{
    sheet->jumpToPage(0);
    sheet->jumpToPage(1);
    sheet->jumpToIndex(0);
}

TEST_F(UT_DocSheet, ResizeEvent_NoEncryptionPage_NoCrash)
{
    sheet->resize(600, 400);
    EXPECT_EQ(QSize(600, 400), sheet->size());
}

TEST_F(UT_DocSheet, ResizeEvent_WithEncryptionPage_AdjustsGeometry)
{
    sheet->onOpened(Document::kNeedPassword);
    EncryptionPage *page = sheet->findChild<EncryptionPage *>();
    ASSERT_NE(nullptr, page);
    sheet->resize(800, 500);
    EXPECT_GE(sheet->width(), 800);
    EXPECT_EQ(500, sheet->height());
}

TEST_F(UT_DocSheet, OnOpened_NeedPassword_ShowsEncryptionPage)
{
    sheet->onOpened(Document::kNeedPassword);
    EXPECT_NE(nullptr, sheet->findChild<EncryptionPage *>());
}

TEST_F(UT_DocSheet, OnOpened_WrongPassword_ShowsEncryptionPage)
{
    sheet->onOpened(Document::kWrongPassword);
    EXPECT_NE(nullptr, sheet->findChild<EncryptionPage *>());
}

TEST_F(UT_DocSheet, OnOpened_NoErrorWithoutPassword_SkipsEncryptionCleanup)
{
    sheet->onOpened(Document::kNeedPassword);
    EncryptionPage *page = sheet->findChild<EncryptionPage *>();
    ASSERT_NE(nullptr, page);
    sheet->onOpened(Document::kNoError);
    EXPECT_NE(nullptr, sheet->findChild<EncryptionPage *>());
}

TEST_F(UT_DocSheet, ChildEvent_AddChild_NoCrash)
{
    QWidget *child = new QWidget(sheet);
    child->deleteLater();
    ut_utils::drainEvents(50);
}

TEST_F(UT_DocSheet, SigPageModified_ConnectedBySideBarModel)
{
    SideBarImageViewModel *model = sheet->findChild<SideBarImageViewModel *>();
    ASSERT_NE(nullptr, model);
    EXPECT_TRUE(sheet->isSignalConnected(
            QMetaMethod::fromSignal(&DocSheet::sigPageModified)));
}

class UT_DocSheetOpened : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        opened = sheet->openFileExec("");
        ASSERT_TRUE(opened);
    }

    virtual void TearDown() override
    {
        ut_utils::waitRenderIdle();
        delete sheet;
        sheet = nullptr;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    bool opened { false };
};

TEST_F(UT_DocSheetOpened, OpenFileExec_ValidPdf_ReturnsTrue)
{
    EXPECT_TRUE(opened);
    EXPECT_TRUE(sheet->opened());
}

TEST_F(UT_DocSheetOpened, SigFileOpened_EmittedWithNoError)
{
    QSignalSpy spy(sheet, SIGNAL(sigFileOpened(DocSheet *, Document::Error)));
    sheet->openFileAsync("");
    ut_utils::processEventsUntil([&spy]() { return spy.count() >= 1; });
    ASSERT_GE(spy.count(), 1);
    EXPECT_EQ(Document::kNoError, qvariant_cast<Document::Error>(spy.at(0).at(1)));
}

TEST_F(UT_DocSheetOpened, PageCount_AfterOpen_MatchesRenderer)
{
    EXPECT_GE(sheet->pageCount(), 2);
    EXPECT_EQ(sheet->pageCount(), sheet->renderer()->getPageCount());
}

TEST_F(UT_DocSheetOpened, Browser_HasLoadedAfterOpen)
{
    SheetBrowser *browser = sheet->findChild<SheetBrowser *>();
    ASSERT_NE(nullptr, browser);
    EXPECT_TRUE(browser->hasLoaded());
    EXPECT_EQ(sheet->pageCount(), browser->allPages());
}

TEST_F(UT_DocSheetOpened, JumpToPage_Two_ScrollsBrowserToPageTwo)
{
    SheetBrowser *browser = sheet->findChild<SheetBrowser *>();
    ASSERT_NE(nullptr, browser);
    ASSERT_GE(sheet->pageCount(), 2);
    sheet->jumpToPage(2);
    EXPECT_EQ(2, browser->currentPage());
    sheet->jumpToIndex(0);
    EXPECT_EQ(1, browser->currentPage());
}

TEST_F(UT_DocSheetOpened, JumpToPage_OutOfRange_Ignored)
{
    SheetBrowser *browser = sheet->findChild<SheetBrowser *>();
    ASSERT_NE(nullptr, browser);
    sheet->jumpToPage(2);
    int value = browser->verticalScrollBar()->value();
    sheet->jumpToPage(0);
    EXPECT_EQ(value, browser->verticalScrollBar()->value());
    sheet->jumpToPage(sheet->pageCount() + 1);
    EXPECT_EQ(value, browser->verticalScrollBar()->value());
}

TEST_F(UT_DocSheetOpened, CurrentPage_WithinRange_ReturnsOperationPage)
{
    sheet->onBrowserPageChanged(3);
    EXPECT_EQ(3, sheet->currentPage());
    EXPECT_EQ(2, sheet->currentIndex());
}

TEST_F(UT_DocSheetOpened, CurrentPage_OutOfRange_Clamps)
{
    sheet->onBrowserPageChanged(sheet->pageCount() + 5);
    EXPECT_EQ(1, sheet->currentPage());
    EXPECT_EQ(0, sheet->currentIndex());
}

TEST_F(UT_DocSheetOpened, SetLayoutMode_TwoPages_DeformsBrowser)
{
    SheetBrowser *browser = sheet->findChild<SheetBrowser *>();
    ASSERT_NE(nullptr, browser);
    qreal singleWidth = browser->sceneRect().width();
    sheet->setLayoutMode(TwoPagesMode);
    EXPECT_EQ(TwoPagesMode, sheet->operation().layoutMode);
    EXPECT_GT(browser->sceneRect().width(), singleWidth);
}

TEST_F(UT_DocSheetOpened, GetImage_ValidIndex_ReturnsRenderedImage)
{
    QImage image = sheet->getImage(0, 60, 80);
    EXPECT_FALSE(image.isNull());
}

TEST_F(UT_DocSheetOpened, PageSizeByIndex_ValidIndex_ReturnsPositiveSize)
{
    QSizeF size = sheet->pageSizeByIndex(0);
    EXPECT_GT(size.width(), 0.0);
    EXPECT_GT(size.height(), 0.0);
}

TEST_F(UT_DocSheetOpened, PageSizeByIndex_OutOfRange_ReturnsEmpty)
{
    EXPECT_EQ(QSizeF(), sheet->pageSizeByIndex(sheet->pageCount()));
}

TEST_F(UT_DocSheetOpened, MaxScaleFactor_PositiveAfterOpen)
{
    EXPECT_GT(sheet->maxScaleFactor(), 0.1);
}

TEST_F(UT_DocSheetOpened, OpenedFilePath_AfterOpen_EqualsPath)
{
    EXPECT_EQ(pdfPath, sheet->openedFilePath());
}

TEST_F(UT_DocSheetOpened, OnExtractPassword_ValidPassword_ReopensAndHidesEncryption)
{
    sheet->onOpened(Document::kNeedPassword);
    ASSERT_NE(nullptr, sheet->findChild<EncryptionPage *>());
    QSignalSpy spy(sheet, SIGNAL(sigFileOpened(DocSheet *, Document::Error)));
    sheet->onExtractPassword("valid-password");
    ut_utils::processEventsUntil([&spy]() { return spy.count() >= 1; });
    ASSERT_GE(spy.count(), 1);
    EXPECT_EQ(Document::kNoError, qvariant_cast<Document::Error>(spy.at(0).at(1)));
    ut_utils::drainEvents(100);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    EXPECT_EQ(nullptr, sheet->findChild<EncryptionPage *>());
}
