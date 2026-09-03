// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "browserpage.h"
#include "docsheet.h"
#include "sheetbrowser.h"
#include "ut_common.h"

#include <gtest/gtest.h>

#include <QImage>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

using namespace plugin_filepreview;

class UT_BrowserPage : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        ASSERT_TRUE(sheet->openFileExec(""));
        browser = sheet->findChild<SheetBrowser *>();
        ASSERT_NE(nullptr, browser);
        page = new BrowserPage(browser, 0, sheet);
    }

    virtual void TearDown() override
    {
        ut_utils::waitRenderIdle();
        delete page;
        delete sheet;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    SheetBrowser *browser { nullptr };
    BrowserPage *page { nullptr };
};

TEST_F(UT_BrowserPage, Construct_WithOpenedSheet_SetsOriginSize)
{
    QSizeF origin = page->originSizeF;
    EXPECT_GT(origin.width(), 0.0);
    EXPECT_GT(origin.height(), 0.0);
}

TEST_F(UT_BrowserPage, Construct_InitialIndexZero)
{
    EXPECT_EQ(0, page->itemIndex());
}

TEST_F(UT_BrowserPage, BoundingRect_BeforeRender_NegativeScale)
{
    EXPECT_LT(page->boundingRect().width(), 0.0);
}

TEST_F(UT_BrowserPage, Render_SetsScaleFactorAndPositiveBoundingRect)
{
    page->render(1.0, kRotateBy0);
    EXPECT_DOUBLE_EQ(1.0, page->currentScaleFactor);
    EXPECT_GT(page->boundingRect().width(), 0.0);
    EXPECT_GT(page->boundingRect().height(), 0.0);
    EXPECT_NEAR(kImageBrowserWidth, page->boundingRect().width(), 0.01);
}

TEST_F(UT_BrowserPage, Render_Rotation90_SwapsRectDimensions)
{
    page->render(1.0, kRotateBy0);
    QRectF rect0 = page->rect();
    page->render(1.0, kRotateBy90);
    QRectF rect90 = page->rect();
    EXPECT_NEAR(rect90.width(), rect0.height(), 0.01);
    EXPECT_NEAR(rect90.height(), rect0.width(), 0.01);
}

TEST_F(UT_BrowserPage, Render_Rotation180_SetsItemRotation)
{
    page->render(1.0, kRotateBy180);
    EXPECT_EQ(qreal(180.0), page->rotation());
    EXPECT_EQ(kRotateBy180, page->currentRotation);
}

TEST_F(UT_BrowserPage, Render_Rotation270_SetsItemRotation)
{
    page->render(1.0, kRotateBy270);
    EXPECT_EQ(qreal(270.0), page->rotation());
}

TEST_F(UT_BrowserPage, Render_RenderLaterWithSameRotation_SkipsUpdate)
{
    page->render(1.0, kRotateBy90, true);
    qreal rotationAfterFirst = page->rotation();
    page->render(1.0, kRotateBy90, true);
    EXPECT_EQ(rotationAfterFirst, page->rotation());
    EXPECT_LT(page->renderPixmapScaleFactor, 0.0);
}

TEST_F(UT_BrowserPage, Render_RenderLaterFalse_CreatesPixmapAndTask)
{
    page->render(1.0, kRotateBy0);
    EXPECT_FALSE(page->currentPixmap.isNull());
    EXPECT_GT(page->currentPixmapId, 0);
    EXPECT_DOUBLE_EQ(1.0, page->renderPixmapScaleFactor);
}

TEST_F(UT_BrowserPage, Render_RealRenderTask_UpdatesPixmapContent)
{
    page->render(1.0, kRotateBy0);
    QImage before = page->getCurrentImage(80, 100);
    ASSERT_FALSE(before.isNull());
    ut_utils::processEventsUntil([&]() {
        QImage after = page->getCurrentImage(80, 100);
        return !after.isNull() && after != before;
    });
    QImage after = page->getCurrentImage(80, 100);
    EXPECT_FALSE(after.isNull());
}

TEST_F(UT_BrowserPage, Render_DifferentScale_ReplacesPixmap)
{
    page->render(1.0, kRotateBy0);
    int idAfterFirst = page->currentPixmapId;
    page->render(0.5, kRotateBy0);
    EXPECT_GT(page->currentPixmapId, idAfterFirst);
    EXPECT_DOUBLE_EQ(0.5, page->renderPixmapScaleFactor);
}

TEST_F(UT_BrowserPage, RenderRect_NullBrowserParent_ReturnsEarly)
{
    BrowserPage *orphan = new BrowserPage(nullptr, 0, sheet);
    orphan->renderRect(QRectF(0, 0, 10, 10));
    delete orphan;
}

TEST_F(UT_BrowserPage, RenderRect_WithParent_NoCrash)
{
    page->renderRect(QRectF(0, 0, 10, 10));
    SUCCEED();
}

TEST_F(UT_BrowserPage, RenderViewPort_NullBrowserParent_ReturnsEarly)
{
    BrowserPage *orphan = new BrowserPage(nullptr, 0, sheet);
    orphan->renderViewPort();
    delete orphan;
}

TEST_F(UT_BrowserPage, RenderViewPort_WithParent_MarksViewportRendered)
{
    sheet->resize(800, 600);
    browser->resize(800, 600);
    ut_utils::drainEvents(50);
    page->render(1.0, kRotateBy0);
    page->renderViewPort();
    EXPECT_TRUE(page->viewportRendered);
}

TEST_F(UT_BrowserPage, GetCurrentImage_NullPixmap_ReturnsNull)
{
    EXPECT_TRUE(page->getCurrentImage(50, 60).isNull());
}

TEST_F(UT_BrowserPage, GetCurrentImage_RequestedLargerThanOriginal_ReturnsNull)
{
    page->render(0.1, kRotateBy0);
    int pixmapWidth = page->currentPixmap.width();
    int pixmapHeight = page->currentPixmap.height();
    int big = qMax(pixmapWidth, pixmapHeight) + 10;
    EXPECT_TRUE(page->getCurrentImage(big, big).isNull());
}

TEST_F(UT_BrowserPage, GetCurrentImage_ValidRequest_ReturnsScaledImage)
{
    page->render(1.0, kRotateBy0);
    QImage image = page->getCurrentImage(60, 80);
    EXPECT_FALSE(image.isNull());
    EXPECT_LE(image.width(), 60);
    EXPECT_LE(image.height(), 80);
}

TEST_F(UT_BrowserPage, ClearPixmap_BeforeAnyRender_NoEffect)
{
    page->clearPixmap();
    EXPECT_LT(page->renderPixmapScaleFactor, 0.0);
}

TEST_F(UT_BrowserPage, ClearPixmap_AfterRender_ClearsImage)
{
    page->render(1.0, kRotateBy0);
    EXPECT_FALSE(page->currentPixmap.isNull());
    page->clearPixmap();
    EXPECT_TRUE(page->currentPixmap.isNull());
    EXPECT_LT(page->renderPixmapScaleFactor, 0.0);
    EXPECT_TRUE(page->getCurrentImage(50, 60).isNull());
}

TEST_F(UT_BrowserPage, GetTopLeftPos_RotateBy0_ReturnsPosition)
{
    page->setPos(100, 200);
    page->render(1.0, kRotateBy0);
    QPointF topLeft = page->getTopLeftPos();
    EXPECT_DOUBLE_EQ(100.0, topLeft.x());
    EXPECT_DOUBLE_EQ(200.0, topLeft.y());
}

TEST_F(UT_BrowserPage, GetTopLeftPos_RotateBy90_ShiftsLeftByHeight)
{
    page->render(1.0, kRotateBy0);
    page->setPos(100, 200);
    page->render(1.0, kRotateBy90);
    QPointF topLeft = page->getTopLeftPos();
    EXPECT_DOUBLE_EQ(100.0 - page->rect().width(), topLeft.x());
    EXPECT_DOUBLE_EQ(200.0, topLeft.y());
}

TEST_F(UT_BrowserPage, GetTopLeftPos_RotateBy180_ShiftsByRectSize)
{
    page->render(1.0, kRotateBy0);
    page->setPos(100, 200);
    page->render(1.0, kRotateBy180);
    QPointF topLeft = page->getTopLeftPos();
    EXPECT_DOUBLE_EQ(100.0 - page->rect().width(), topLeft.x());
    EXPECT_DOUBLE_EQ(200.0 - page->rect().height(), topLeft.y());
}

TEST_F(UT_BrowserPage, GetTopLeftPos_RotateBy270_ShiftsUpByHeight)
{
    page->render(1.0, kRotateBy0);
    page->setPos(100, 200);
    page->render(1.0, kRotateBy270);
    QPointF topLeft = page->getTopLeftPos();
    EXPECT_DOUBLE_EQ(100.0, topLeft.x());
    EXPECT_DOUBLE_EQ(200.0 - page->rect().height(), topLeft.y());
}

TEST_F(UT_BrowserPage, HandleRenderFinished_MatchingId_UpdatesPixmap)
{
    page->render(1.0, kRotateBy0);
    int id = page->currentPixmapId;
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::blue);
    page->handleRenderFinished(id, pixmap);
    EXPECT_TRUE(page->pixmapHasRendered);
    EXPECT_EQ(QColor(Qt::blue), QColor(page->currentPixmap.toImage().pixelColor(5, 5)));
}

TEST_F(UT_BrowserPage, HandleRenderFinished_StaleId_Ignored)
{
    page->render(1.0, kRotateBy0);
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::blue);
    page->handleRenderFinished(page->currentPixmapId + 100, pixmap);
    EXPECT_FALSE(page->pixmapHasRendered);
}

TEST_F(UT_BrowserPage, HandleRenderFinished_WithSlice_PaintsIntoPixmap)
{
    page->render(1.0, kRotateBy0);
    int id = page->currentPixmapId;
    QPixmap slice(10, 10);
    slice.fill(Qt::green);
    page->handleRenderFinished(id, slice, QRect(0, 0, 10, 10));
    EXPECT_EQ(QColor(Qt::green), QColor(page->currentPixmap.toImage().pixelColor(3, 3)));
    EXPECT_FALSE(page->pixmapHasRendered);
}

TEST_F(UT_BrowserPage, Paint_Offscreen_NoCrash)
{
    page->render(1.0, kRotateBy0);
    QImage canvas(400, 500, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    QStyleOptionGraphicsItem option;
    option.rect = QRect(0, 0, 400, 500);
    page->paint(&painter, &option);
    painter.end();
    SUCCEED();
}

TEST_F(UT_BrowserPage, Destructor_ClearsPendingTasks_NoCrash)
{
    BrowserPage *temp = new BrowserPage(browser, 1, sheet);
    temp->render(1.0, kRotateBy0);
    delete temp;
    SUCCEED();
}
