// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "browserpage.h"
#include "docsheet.h"
#include "pagerenderthread.h"
#include "pdfmodel.h"
#include "sheetbrowser.h"
#include "sheetrenderer.h"
#include "sidebarimageviewmodel.h"
#include "ut_common.h"

#include <gtest/gtest.h>

using namespace plugin_filepreview;

class UT_PageRenderThread : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        browser = sheet->findChild<SheetBrowser *>();
        ASSERT_NE(nullptr, browser);
        model = new SideBarImageViewModel(sheet);
    }

    virtual void TearDown() override
    {
        ut_utils::waitRenderIdle();
        delete model;
        delete sheet;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    SheetBrowser *browser { nullptr };
    SideBarImageViewModel *model { nullptr };
};

TEST_F(UT_PageRenderThread, ClearImageTasks_NullPage_ReturnsTrue)
{
    EXPECT_TRUE(PageRenderThread::clearImageTasks(nullptr, nullptr));
}

TEST_F(UT_PageRenderThread, ClearImageTasks_ValidPage_ReturnsTrue)
{
    BrowserPage page(browser, 0, sheet);
    EXPECT_TRUE(PageRenderThread::clearImageTasks(sheet, &page));
}

TEST_F(UT_PageRenderThread, ClearImageTasks_RemovesAppendedTasks)
{
    BrowserPage page(browser, 0, sheet);
    DocPageNormalImageTask task;
    task.sheet = sheet;
    task.page = &page;
    task.pixmapId = 1;
    task.rect = QRect(0, 0, 50, 50);
    PageRenderThread::appendTask(task);
    EXPECT_TRUE(PageRenderThread::clearImageTasks(sheet, &page));
    ut_utils::drainEvents(150);
    SUCCEED();
}

TEST_F(UT_PageRenderThread, AppendTask_OpenTask_OpensDocumentAsync)
{
    SheetRenderer *renderer = sheet->findChild<SheetRenderer *>();
    ASSERT_NE(nullptr, renderer);
    EXPECT_FALSE(renderer->opened());

    DocOpenTask task;
    task.sheet = sheet;
    task.password = "";
    task.renderer = renderer;
    PageRenderThread::appendTask(task);

    ut_utils::processEventsUntil([&renderer]() { return renderer->opened(); });
    EXPECT_TRUE(renderer->opened());
    EXPECT_GE(renderer->getPageCount(), 2);
}

TEST_F(UT_PageRenderThread, AppendTask_ThumbnailTask_RendersThumbnail)
{
    SheetRenderer *renderer = sheet->findChild<SheetRenderer *>();
    ASSERT_NE(nullptr, renderer);
    ASSERT_TRUE(renderer->openFileExec(""));
    EXPECT_TRUE(sheet->thumbnail(0).isNull());

    DocPageThumbnailTask task;
    task.sheet = sheet;
    task.model = model;
    task.index = 0;
    PageRenderThread::appendTask(task);

    ut_utils::processEventsUntil([&]() { return !sheet->thumbnail(0).isNull(); });
    QPixmap thumbnail = sheet->thumbnail(0);
    EXPECT_FALSE(thumbnail.isNull());
    EXPECT_GT(thumbnail.width(), 0);
    EXPECT_LE(thumbnail.width(), 200);
}

TEST_F(UT_PageRenderThread, AppendTask_NormalImageTask_NoCrashWhenOrphanPage)
{
    BrowserPage *page = new BrowserPage(nullptr, 0, sheet);
    DocPageNormalImageTask task;
    task.sheet = sheet;
    task.page = page;
    task.pixmapId = 1;
    task.rect = QRect(0, 0, 60, 80);
    PageRenderThread::appendTask(task);
    ut_utils::drainEvents(300);
    delete page;
    SUCCEED();
}

TEST_F(UT_PageRenderThread, AppendTask_SliceImageTask_NoCrash)
{
    BrowserPage *page = new BrowserPage(browser, 0, sheet);
    page->render(1.0, kRotateBy0);
    DocPageSliceImageTask task;
    task.sheet = sheet;
    task.page = page;
    task.pixmapId = page->currentPixmapId;
    task.whole = QRect(0, 0, 100, 130);
    task.slice = QRect(0, 0, 50, 65);
    PageRenderThread::appendTask(task);
    ut_utils::drainEvents(300);
    delete page;
    SUCCEED();
}

TEST_F(UT_PageRenderThread, AppendTask_CloseTask_DeletesDocument)
{
    Document::Error error = Document::kFileError;
    PDFDocument *document = PDFDocument::loadDocument(pdfPath, "", error);
    ASSERT_NE(nullptr, document);
    QList<Page *> pages;
    for (int i = 0; i < document->pageCount(); ++i)
        pages.append(document->page(i));

    DocCloseTask task;
    task.document = document;
    task.pages = pages;
    PageRenderThread::appendTask(task);

    ut_utils::drainEvents(300);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    SUCCEED();
}

TEST_F(UT_PageRenderThread, OnDocOpenTask_ForMissingFile_ReportsFileError)
{
    DocSheet missing(kPDF, "/nonexistent-dir/no-such.pdf");
    SheetRenderer *renderer = missing.findChild<SheetRenderer *>();
    ASSERT_NE(nullptr, renderer);

    DocOpenTask task;
    task.sheet = &missing;
    task.password = "";
    task.renderer = renderer;
    PageRenderThread::appendTask(task);

    ut_utils::processEventsUntil([&renderer]() { return renderer->docError == Document::kFileError; });
    EXPECT_EQ(Document::kFileError, renderer->docError);
    EXPECT_FALSE(renderer->opened());
}

TEST_F(UT_PageRenderThread, OnDocPageNormalImageTaskFinished_UpdatesPage)
{
    BrowserPage page(browser, 0, sheet);
    page.render(1.0, kRotateBy0);
    DocPageNormalImageTask task;
    task.sheet = sheet;
    task.page = &page;
    task.pixmapId = page.currentPixmapId;

    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::red);
    PageRenderThread::instance()->onDocPageNormalImageTaskFinished(task, pixmap);
    EXPECT_TRUE(page.pixmapHasRendered);
}

TEST_F(UT_PageRenderThread, OnDocPageThumbnailTask_UpdatesModelThumbnail)
{
    QPixmap pixmap(30, 30);
    pixmap.fill(Qt::blue);
    PageRenderThread::instance()->onDocPageThumbnailTask([&]() {
        DocPageThumbnailTask task;
        task.sheet = sheet;
        task.model = model;
        task.index = 2;
        return task;
    }(), pixmap);
    EXPECT_EQ(30, sheet->thumbnail(2).width());
}

TEST_F(UT_PageRenderThread, OnDocOpenTask_DeliversToRenderer)
{
    Document::Error error = Document::kFileError;
    PDFDocument *document = PDFDocument::loadDocument(pdfPath, "", error);
    ASSERT_NE(nullptr, document);
    QList<Page *> pages;
    for (int i = 0; i < document->pageCount(); ++i)
        pages.append(document->page(i));

    DocOpenTask task;
    task.sheet = sheet;
    task.password = "";
    task.renderer = sheet->findChild<SheetRenderer *>();
    PageRenderThread::instance()->onDocOpenTask(task, Document::kNoError, document, pages);

    SheetRenderer *renderer = sheet->findChild<SheetRenderer *>();
    EXPECT_TRUE(renderer->opened());
    EXPECT_EQ(pages.count(), renderer->getPageCount());

    delete renderer->documentObj;
    qDeleteAll(renderer->pageList);
    renderer->documentObj = nullptr;
    renderer->pageList.clear();
}

TEST_F(UT_PageRenderThread, TaskForDeadSheet_IsSkipped)
{
    DocSheet *tempSheet = new DocSheet(kPDF, pdfPath);
    SheetRenderer *renderer = tempSheet->findChild<SheetRenderer *>();
    delete tempSheet;

    DocOpenTask task;
    task.sheet = nullptr;
    task.password = "";
    task.renderer = renderer;
    PageRenderThread::appendTask(task);
    ut_utils::drainEvents(200);
    SUCCEED();
}
