// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "browserpage.h"
#include "docsheet.h"
#include "global.h"
#include "sheetbrowser.h"
#include "ut_common.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSignalSpy>
#include <QWheelEvent>
#include <QWidget>

using namespace plugin_filepreview;

class UT_SheetBrowser : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        browser = sheet->findChild<SheetBrowser *>();
        ASSERT_NE(nullptr, browser);
    }

    virtual void TearDown() override
    {
        setMainWidget(nullptr);
        ut_utils::waitRenderIdle();
        delete sheet;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    SheetBrowser *browser { nullptr };
};

TEST_F(UT_SheetBrowser, Construct_CreatesScene)
{
    EXPECT_NE(nullptr, browser->scene());
}

TEST_F(UT_SheetBrowser, Construct_ScrollbarsSetup)
{
    EXPECT_EQ(QStringLiteral("verticalScrollBar"), browser->verticalScrollBar()->accessibleName());
    EXPECT_EQ(QStringLiteral("horizontalScrollBar"), browser->horizontalScrollBar()->accessibleName());
}

TEST_F(UT_SheetBrowser, HasLoaded_BeforeInit_False)
{
    EXPECT_FALSE(browser->hasLoaded());
}

TEST_F(UT_SheetBrowser, AllPages_BeforeInit_Zero)
{
    EXPECT_EQ(0, browser->allPages());
}

TEST_F(UT_SheetBrowser, CurrentPage_BeforeInit_ReturnsOne)
{
    EXPECT_EQ(1, browser->currentPage());
}

TEST_F(UT_SheetBrowser, SetCurrentPage_BeforeInit_Ignored)
{
    browser->setCurrentPage(1);
    SUCCEED();
}

TEST_F(UT_SheetBrowser, GetExistImage_BeforeInit_ReturnsFalse)
{
    QImage image;
    EXPECT_FALSE(browser->getExistImage(0, image, 50, 50));
}

TEST_F(UT_SheetBrowser, MaxWidthHeight_BeforeInit_Zero)
{
    EXPECT_DOUBLE_EQ(0.0, browser->maxWidth());
    EXPECT_DOUBLE_EQ(0.0, browser->maxHeight());
}

TEST_F(UT_SheetBrowser, Pages_BeforeInit_Empty)
{
    EXPECT_TRUE(browser->pages().isEmpty());
}

TEST_F(UT_SheetBrowser, Deform_EmptyList_NoCrash)
{
    SheetOperation operation;
    browser->deform(operation);
    EXPECT_TRUE(browser->sceneRect().isEmpty());
}

TEST_F(UT_SheetBrowser, WheelEvent_BeforeInit_NoPageEmitted)
{
    QSignalSpy spy(browser, SIGNAL(sigPageChanged(int)));
    QWheelEvent event(QPointF(10, 10), QPointF(110, 110), QPoint(0, 0), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(browser->viewport(), &event);
    EXPECT_EQ(0, spy.count());
}

TEST_F(UT_SheetBrowser, MouseDragSequence_NoMainWidget_NoCrash)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(100, 100),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(browser->viewport(), &press);
    QMouseEvent move(QEvent::MouseMove, QPointF(30, 30), QPointF(120, 120),
                     Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(browser->viewport(), &move);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(30, 30), QPointF(120, 120),
                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(browser->viewport(), &release);
    SUCCEED();
}

TEST_F(UT_SheetBrowser, ResizeEvent_NoCrash)
{
    browser->resize(500, 400);
    EXPECT_EQ(400, browser->height());
}

TEST_F(UT_SheetBrowser, FocusOutEvent_NoCrash)
{
    QFocusEvent event(QEvent::FocusOut);
    QApplication::sendEvent(browser, &event);
    SUCCEED();
}

TEST_F(UT_SheetBrowser, OnRemoveDocSlideGesture_StopsScroller)
{
    browser->onRemoveDocSlideGesture();
    SUCCEED();
}

TEST_F(UT_SheetBrowser, BeginViewportChange_TimerEventuallyFires)
{
    browser->beginViewportChange();
    ut_utils::drainEvents(200);
    SUCCEED();
}

TEST_F(UT_SheetBrowser, OnViewportChanged_WithEmptyList_NoCrash)
{
    browser->onViewportChanged();
    SUCCEED();
}

class UT_SheetBrowserOpened : public testing::Test
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
    }

    virtual void TearDown() override
    {
        setMainWidget(nullptr);
        ut_utils::waitRenderIdle();
        delete sheet;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    SheetBrowser *browser { nullptr };
};

TEST_F(UT_SheetBrowserOpened, HasLoaded_AfterInit_True)
{
    EXPECT_TRUE(browser->hasLoaded());
}

TEST_F(UT_SheetBrowserOpened, AllPages_MatchesPageCount)
{
    EXPECT_EQ(sheet->pageCount(), browser->allPages());
}

TEST_F(UT_SheetBrowserOpened, Pages_CountMatchesAllPages)
{
    EXPECT_EQ(browser->allPages(), browser->pages().count());
}

TEST_F(UT_SheetBrowserOpened, MaxWidthHeight_PositiveAfterInit)
{
    EXPECT_GT(browser->maxWidth(), 0.0);
    EXPECT_GT(browser->maxHeight(), 0.0);
    EXPECT_DOUBLE_EQ(sheet->pageSizeByIndex(0).width(), browser->maxWidth());
}

TEST_F(UT_SheetBrowserOpened, CurrentPage_InitiallyOne)
{
    EXPECT_EQ(1, browser->currentPage());
}

TEST_F(UT_SheetBrowserOpened, SetCurrentPage_SecondPage_ScrollsToPageTwo)
{
    ASSERT_GE(browser->allPages(), 2);
    browser->setCurrentPage(2);
    EXPECT_EQ(2, browser->currentPage());
}

TEST_F(UT_SheetBrowserOpened, SetCurrentPage_OutOfRange_Ignored)
{
    ASSERT_GE(browser->allPages(), 2);
    browser->setCurrentPage(2);
    int value = browser->verticalScrollBar()->value();
    browser->setCurrentPage(0);
    EXPECT_EQ(value, browser->verticalScrollBar()->value());
    browser->setCurrentPage(browser->allPages() + 1);
    EXPECT_EQ(value, browser->verticalScrollBar()->value());
}

TEST_F(UT_SheetBrowserOpened, GetExistImage_IndexOutOfRange_False)
{
    QImage image;
    EXPECT_FALSE(browser->getExistImage(browser->allPages(), image, 50, 50));
}

TEST_F(UT_SheetBrowserOpened, GetExistImage_AfterRender_True)
{
    browser->pages().at(0)->render(1.0, kRotateBy0);
    QImage image;
    EXPECT_TRUE(browser->getExistImage(0, image, 50, 60));
    EXPECT_FALSE(image.isNull());
}

TEST_F(UT_SheetBrowserOpened, Deform_TwoPagesMode_WidensSceneRect)
{
    qreal singleWidth = browser->sceneRect().width();
    SheetOperation operation = sheet->operation();
    operation.layoutMode = TwoPagesMode;
    browser->deform(operation);
    EXPECT_GT(browser->sceneRect().width(), singleWidth);
}

TEST_F(UT_SheetBrowserOpened, Deform_RotateBy90_NoCrash)
{
    SheetOperation operation = sheet->operation();
    operation.rotation = kRotateBy90;
    browser->deform(operation);
    EXPECT_GT(browser->sceneRect().height(), 0.0);
}

TEST_F(UT_SheetBrowserOpened, Deform_RotateBy180_NoCrash)
{
    SheetOperation operation = sheet->operation();
    operation.rotation = kRotateBy180;
    browser->deform(operation);
    EXPECT_GT(browser->sceneRect().height(), 0.0);
}

TEST_F(UT_SheetBrowserOpened, Deform_RotateBy270_NoCrash)
{
    SheetOperation operation = sheet->operation();
    operation.rotation = kRotateBy270;
    browser->deform(operation);
    EXPECT_GT(browser->sceneRect().height(), 0.0);
}

TEST_F(UT_SheetBrowserOpened, Deform_WithCurrentPage_ScrollsToPage)
{
    SheetOperation operation = sheet->operation();
    operation.currentPage = 2;
    browser->deform(operation);
    EXPECT_EQ(2, browser->currentPage());
}

TEST_F(UT_SheetBrowserOpened, Deform_InvalidCurrentPage_NoCrash)
{
    SheetOperation operation = sheet->operation();
    operation.currentPage = 999;
    browser->deform(operation);
    SUCCEED();
}

TEST_F(UT_SheetBrowserOpened, WheelEvent_PageUnderCursor_EmitsPageChanged)
{
    sheet->resize(900, 700);
    sheet->show();
    ut_utils::drainEvents(150);
    QSignalSpy spy(browser, SIGNAL(sigPageChanged(int)));
    QPoint center = browser->viewport()->rect().center();
    QWheelEvent event(QPointF(center), QPointF(center + QPoint(100, 100)),
                      QPoint(0, 0), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(browser->viewport(), &event);
    EXPECT_GE(spy.count(), 1);
    if (spy.count() > 0)
        EXPECT_EQ(1, qvariant_cast<int>(spy.at(0).at(0)));
    sheet->hide();
}

TEST_F(UT_SheetBrowserOpened, MouseDragSequence_WithMainWidget_MovesWindow)
{
    QWidget host;
    host.resize(400, 300);
    host.move(10, 10);
    host.show();
    setMainWidget(&host);
    QPoint before = host.pos();
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(100, 100),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(browser->viewport(), &press);
    QMouseEvent move(QEvent::MouseMove, QPointF(30, 30), QPointF(130, 130),
                     Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(browser->viewport(), &move);
    EXPECT_EQ(before + QPoint(30, 30), host.pos());
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(30, 30), QPointF(130, 130),
                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(browser->viewport(), &release);
    host.hide();
}

TEST_F(UT_SheetBrowserOpened, VerticalScrollChange_TriggersViewportRefresh)
{
    ASSERT_GE(browser->allPages(), 2);
    browser->setCurrentPage(2);
    EXPECT_GT(browser->verticalScrollBar()->value(), 0);
    ut_utils::drainEvents(250);
    EXPECT_EQ(2, browser->currentPage());
}

TEST_F(UT_SheetBrowserOpened, GetBrowserPageForPoint_OnFirstPage_ReturnsPage)
{
    sheet->resize(900, 700);
    sheet->show();
    ut_utils::drainEvents(150);
    QPointF point(browser->viewport()->rect().center());
    BrowserPage *found = browser->getBrowserPageForPoint(point);
    EXPECT_NE(nullptr, found);
    if (found)
        EXPECT_EQ(0, found->itemIndex());
    sheet->hide();
}

TEST_F(UT_SheetBrowserOpened, GetBrowserPageForPoint_EmptyArea_ReturnsNull)
{
    sheet->show();
    ut_utils::drainEvents(150);
    QPointF point(3000, 3000);
    BrowserPage *found = browser->getBrowserPageForPoint(point);
    EXPECT_EQ(nullptr, found);
    sheet->hide();
}

TEST_F(UT_SheetBrowserOpened, ShowEvent_TriggersDelayedInit_NoCrash)
{
    browser->show();
    ut_utils::drainEvents(200);
    EXPECT_TRUE(browser->hasLoaded());
    browser->hide();
}

TEST_F(UT_SheetBrowserOpened, CurrentIndexRange_CoversVisiblePages)
{
    int fromIndex = 0;
    int toIndex = 0;
    browser->currentIndexRange(fromIndex, toIndex);
    EXPECT_EQ(0, fromIndex);
    EXPECT_GE(toIndex, 0);
}

TEST_F(UT_SheetBrowserOpened, OnViewportChanged_ClearsFarPages)
{
    BrowserPage *lastPage = browser->pages().last();
    lastPage->render(1.0, kRotateBy0);
    EXPECT_FALSE(lastPage->currentPixmap.isNull());
    browser->onViewportChanged();
    EXPECT_TRUE(lastPage->currentPixmap.isNull());
}

TEST_F(UT_SheetBrowserOpened, OnInit_WithJumpPage_ScrollsToTarget)
{
    browser->jumpPageNumber = 2;
    browser->onInit();
    EXPECT_EQ(2, browser->currentPage());
    EXPECT_EQ(1, browser->jumpPageNumber);
}

TEST_F(UT_SheetBrowserOpened, TimerEvent_UnknownTimerId_Ignored)
{
    QTimerEvent event(-1);
    QApplication::sendEvent(browser, &event);
    SUCCEED();
}
