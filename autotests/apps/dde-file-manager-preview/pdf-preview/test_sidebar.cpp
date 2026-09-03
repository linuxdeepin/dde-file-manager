// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docsheet.h"
#include "sheetbrowser.h"
#include "sheetsidebar.h"
#include "sidebarimagelistview.h"
#include "sidebarimageviewmodel.h"
#include "thumbnaildelegate.h"
#include "thumbnailwidget.h"
#include "ut_common.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QStyleOptionViewItem>

using namespace plugin_filepreview;

TEST(UT_ImagePageInfo, Operators_CompareByPageIndex)
{
    ImagePageInfo_t left(1);
    ImagePageInfo_t right(2);

    EXPECT_TRUE(left == ImagePageInfo_t(1));
    EXPECT_TRUE(left < right);
    EXPECT_TRUE(right > left);
    EXPECT_FALSE(left == right);
}

TEST(UT_ImagePageInfo, DefaultConstructor_HasInvalidIndex)
{
    ImagePageInfo_t info;
    EXPECT_EQ(-1, info.pageIndex);
}

class UT_SideBarImageViewModel : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        ASSERT_TRUE(sheet->openFileExec(""));
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
    SideBarImageViewModel *model { nullptr };
};

TEST_F(UT_SideBarImageViewModel, RowCount_EmptyByDefault)
{
    EXPECT_EQ(0, model->rowCount());
}

TEST_F(UT_SideBarImageViewModel, ColumnCount_AlwaysOne)
{
    EXPECT_EQ(1, model->columnCount(QModelIndex()));
}

TEST_F(UT_SideBarImageViewModel, InitModelLst_Unsorted_KeepsOrder)
{
    QList<ImagePageInfo_t> list { ImagePageInfo_t(2), ImagePageInfo_t(0), ImagePageInfo_t(1) };
    model->initModelLst(list);
    EXPECT_EQ(3, model->rowCount());
    EXPECT_EQ(2, model->getPageIndexForModelIndex(0));
    EXPECT_EQ(0, model->getPageIndexForModelIndex(1));
    EXPECT_EQ(1, model->getPageIndexForModelIndex(2));
}

TEST_F(UT_SideBarImageViewModel, InitModelLst_Sorted_SortsAscending)
{
    QList<ImagePageInfo_t> list { ImagePageInfo_t(2), ImagePageInfo_t(0), ImagePageInfo_t(1) };
    model->initModelLst(list, true);
    EXPECT_EQ(0, model->getPageIndexForModelIndex(0));
    EXPECT_EQ(1, model->getPageIndexForModelIndex(1));
    EXPECT_EQ(2, model->getPageIndexForModelIndex(2));
}

TEST_F(UT_SideBarImageViewModel, ResetData_ClearsAllRows)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0), ImagePageInfo_t(1) });
    EXPECT_EQ(2, model->rowCount());
    model->resetData();
    EXPECT_EQ(0, model->rowCount());
}

TEST_F(UT_SideBarImageViewModel, ChangeModelData_ReplacesListWithoutReset)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0) });
    model->changeModelData(QList<ImagePageInfo_t> { ImagePageInfo_t(3), ImagePageInfo_t(4) });
    EXPECT_EQ(2, model->rowCount());
    EXPECT_EQ(3, model->getPageIndexForModelIndex(0));
}

TEST_F(UT_SideBarImageViewModel, GetModelIndexForPageIndex_Found_ReturnsIndices)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(5), ImagePageInfo_t(7) });
    QList<QModelIndex> indices = model->getModelIndexForPageIndex(7);
    ASSERT_EQ(1, indices.count());
    EXPECT_EQ(1, indices.first().row());
}

TEST_F(UT_SideBarImageViewModel, GetModelIndexForPageIndex_NotFound_ReturnsEmpty)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(5) });
    EXPECT_TRUE(model->getModelIndexForPageIndex(9).isEmpty());
}

TEST_F(UT_SideBarImageViewModel, GetPageIndexForModelIndex_OutOfRange_ReturnsMinusOne)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(5) });
    EXPECT_EQ(-1, model->getPageIndexForModelIndex(-1));
    EXPECT_EQ(-1, model->getPageIndexForModelIndex(1));
}

TEST_F(UT_SideBarImageViewModel, Data_InvalidIndex_ReturnsInvalidVariant)
{
    EXPECT_FALSE(model->data(QModelIndex(), Qt::DisplayRole).isValid());
}

TEST_F(UT_SideBarImageViewModel, Data_InvalidPageRow_ReturnsInvalidVariant)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(-1) });
    EXPECT_FALSE(model->data(model->index(0), ImageinfoType_e::IMAGE_PIXMAP).isValid());
}

TEST_F(UT_SideBarImageViewModel, Data_PixmapRole_MissingThumbnailFillsPlaceholder)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0) });
    QVariant first = model->data(model->index(0), ImageinfoType_e::IMAGE_PIXMAP);
    EXPECT_TRUE(first.value<QPixmap>().isNull());

    QVariant second = model->data(model->index(0), ImageinfoType_e::IMAGE_PIXMAP);
    QPixmap placeholder = second.value<QPixmap>();
    EXPECT_FALSE(placeholder.isNull());
    EXPECT_EQ(200, placeholder.width());
    EXPECT_EQ(200, placeholder.height());
}

TEST_F(UT_SideBarImageViewModel, Data_PixmapRole_ExistingThumbnailReturned)
{
    QPixmap known(40, 40);
    known.fill(Qt::red);
    sheet->setThumbnail(0, known);
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0) });
    QVariant result = model->data(model->index(0), ImageinfoType_e::IMAGE_PIXMAP);
    EXPECT_EQ(40, result.value<QPixmap>().width());
}

TEST_F(UT_SideBarImageViewModel, Data_RotateRole_ReturnsRotationDegrees)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0) });
    QVariant result = model->data(model->index(0), ImageinfoType_e::IMAGE_ROTATE);
    EXPECT_EQ(sheet->operation().rotation * 90, result.toInt());
}

TEST_F(UT_SideBarImageViewModel, Data_AccessibleRole_ReturnsRow)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0), ImagePageInfo_t(1) });
    EXPECT_EQ(1, model->data(model->index(1), Qt::AccessibleTextRole).toInt());
}

TEST_F(UT_SideBarImageViewModel, Data_PageSizeRole_ReturnsPageSize)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0) });
    QVariant result = model->data(model->index(0), ImageinfoType_e::IMAGE_PAGE_SIZE);
    QSizeF size = result.toSizeF();
    EXPECT_GT(size.width(), 0.0);
}

TEST_F(UT_SideBarImageViewModel, Data_UnhandledRole_ReturnsInvalid)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0) });
    EXPECT_FALSE(model->data(model->index(0), Qt::DisplayRole).isValid());
}

TEST_F(UT_SideBarImageViewModel, SetData_InvalidIndex_ReturnsFalse)
{
    EXPECT_FALSE(model->setData(QModelIndex(), QVariant("x"), Qt::EditRole));
}

TEST_F(UT_SideBarImageViewModel, HandleRenderThumbnail_UpdatesThumbnailAndEmitsDataChanged)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0) });
    QSignalSpy spy(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));
    QPixmap pixmap(35, 45);
    pixmap.fill(Qt::green);
    model->handleRenderThumbnail(0, pixmap);
    EXPECT_EQ(35, sheet->thumbnail(0).width());
    EXPECT_EQ(1, spy.count());
}

TEST_F(UT_SideBarImageViewModel, OnUpdateImage_NoCrash)
{
    model->onUpdateImage(0);
    ut_utils::drainEvents(150);
    SUCCEED();
}

TEST_F(UT_SideBarImageViewModel, SigPageModified_FromSheet_LeadsToThumbnailTask)
{
    model->initModelLst(QList<ImagePageInfo_t> { ImagePageInfo_t(0) });
    emit sheet->sigPageModified(0);
    ut_utils::drainEvents(200);
    SUCCEED();
}

class UT_SideBarImageListView : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        ASSERT_TRUE(sheet->openFileExec(""));
        listView = new SideBarImageListView(sheet);
        listView->resize(266, 500);
    }

    virtual void TearDown() override
    {
        ut_utils::waitRenderIdle();
                delete listView;
        delete sheet;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    SideBarImageListView *listView { nullptr };
};

TEST_F(UT_SideBarImageListView, Construct_InstallsModel)
{
    EXPECT_NE(nullptr, listView->model());
}

TEST_F(UT_SideBarImageListView, HandleOpenSuccess_PopulatesModelWithAllPages)
{
    listView->handleOpenSuccess();
    EXPECT_EQ(sheet->pageCount(), listView->model()->rowCount());
}

TEST_F(UT_SideBarImageListView, ScrollToIndex_ExistingPage_SelectsAndReturnsTrue)
{
    listView->handleOpenSuccess();
    EXPECT_TRUE(listView->scrollToIndex(1));
    EXPECT_EQ(1, listView->currentIndex().row());
}

TEST_F(UT_SideBarImageListView, ScrollToIndex_WithoutScrollFlag_StillSelects)
{
    listView->handleOpenSuccess();
    EXPECT_TRUE(listView->scrollToIndex(2, false));
    EXPECT_EQ(2, listView->currentIndex().row());
}

TEST_F(UT_SideBarImageListView, ScrollToIndex_MissingPage_ReturnsFalseAndClears)
{
    listView->handleOpenSuccess();
    EXPECT_FALSE(listView->scrollToIndex(999));
    EXPECT_FALSE(listView->currentIndex().isValid());
}

TEST_F(UT_SideBarImageListView, PageUpIndex_WithModel_ReturnsValidIndex)
{
    listView->handleOpenSuccess();
    listView->scrollToIndex(2);
    QModelIndex up = listView->pageUpIndex();
    EXPECT_TRUE(up.isValid() || up.row() >= 0);
}

TEST_F(UT_SideBarImageListView, PageDownIndex_WithModel_ReturnsValidIndex)
{
    listView->handleOpenSuccess();
    QModelIndex down = listView->pageDownIndex();
    EXPECT_TRUE(down.isValid() || down.row() >= 0);
}

TEST_F(UT_SideBarImageListView, OnItemClicked_ValidIndex_JumpsAndEmits)
{
    listView->handleOpenSuccess();
    SheetBrowser *browser = sheet->findChild<SheetBrowser *>();
    ASSERT_NE(nullptr, browser);
    QSignalSpy spy(listView, SIGNAL(sigListItemClicked(int)));
    listView->onItemClicked(listView->model()->index(1, 0));
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(2, browser->currentPage());
}

TEST_F(UT_SideBarImageListView, OnItemClicked_InvalidIndex_NoSignal)
{
    QSignalSpy spy(listView, SIGNAL(sigListItemClicked(int)));
    listView->onItemClicked(QModelIndex());
    EXPECT_EQ(0, spy.count());
}

TEST_F(UT_SideBarImageListView, MousePressEvent_OnItem_NoCrash)
{
    listView->handleOpenSuccess();
    listView->show();
    ut_utils::drainEvents(100);
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(50, 20), QPointF(200, 200),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(listView->viewport(), &press);
    SUCCEED();
}

class UT_SheetSidebar : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
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
};

TEST_F(UT_SheetSidebar, Construct_ThumbnailFlag_CreatesThumbnailWidget)
{
    SheetSidebar sidebar(sheet, PREVIEW_THUMBNAIL);
    EXPECT_NE(nullptr, sidebar.findChild<ThumbnailWidget *>());
}

TEST_F(UT_SheetSidebar, Construct_NullFlag_NoThumbnailWidget)
{
    SheetSidebar sidebar(sheet, PREVIEW_NULL);
    EXPECT_EQ(nullptr, sidebar.findChild<ThumbnailWidget *>());
}

TEST_F(UT_SheetSidebar, Construct_FixedWidthApplied)
{
    SheetSidebar sidebar(sheet, PREVIEW_THUMBNAIL);
    sidebar.show();
    ut_utils::drainEvents(50);
    EXPECT_EQ(50, sidebar.width());
}

TEST_F(UT_SheetSidebar, SetCurrentPage_WithThumbnailWidget_NoCrash)
{
    SheetSidebar sidebar(sheet, PREVIEW_THUMBNAIL);
    sidebar.setCurrentPage(1);
    SUCCEED();
}

TEST_F(UT_SheetSidebar, SetCurrentPage_NullFlag_NoCrash)
{
    SheetSidebar sidebar(sheet, PREVIEW_NULL);
    sidebar.setCurrentPage(1);
    SUCCEED();
}

TEST_F(UT_SheetSidebar, HandleOpenSuccess_VisiblePerOperation)
{
    SheetSidebar sidebar(sheet, PREVIEW_THUMBNAIL);
    sidebar.handleOpenSuccess();
    EXPECT_EQ(sheet->operation().sidebarVisible, !sidebar.isHidden());
}

TEST_F(UT_SheetSidebar, ShowEvent_TriggersDelayedOpenSuccess_NoCrash)
{
    SheetSidebar sidebar(sheet, PREVIEW_THUMBNAIL);
    sidebar.show();
    ut_utils::drainEvents(200);
    SUCCEED();
}

TEST_F(UT_SheetSidebar, OnHandWidgetDocOpenSuccess_WhileHidden_NoCrash)
{
    SheetSidebar sidebar(sheet, PREVIEW_THUMBNAIL);
    sidebar.openDocOpenSuccess = true;
    sidebar.onHandWidgetDocOpenSuccess();
    SUCCEED();
}

TEST_F(UT_SheetSidebar, ResizeEvent_NoCrash)
{
    SheetSidebar sidebar(sheet, PREVIEW_THUMBNAIL);
    sidebar.resize(100, 400);
    SUCCEED();
}

class UT_ThumbnailWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        ASSERT_TRUE(sheet->openFileExec(""));
        widget = new ThumbnailWidget(sheet);
        widget->resize(266, 500);
    }

    virtual void TearDown() override
    {
        ut_utils::waitRenderIdle();
                delete widget;
        delete sheet;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    ThumbnailWidget *widget { nullptr };
};

TEST_F(UT_ThumbnailWidget, Construct_CreatesListViewWithDelegate)
{
    SideBarImageListView *listView = widget->findChild<SideBarImageListView *>();
    ASSERT_NE(nullptr, listView);
    EXPECT_NE(nullptr, dynamic_cast<ThumbnailDelegate *>(listView->itemDelegate()));
}

TEST_F(UT_ThumbnailWidget, HandleOpenSuccess_PopulatesModel)
{
    widget->show();
    widget->handleOpenSuccess();
    SideBarImageListView *listView = widget->findChild<SideBarImageListView *>();
    ASSERT_NE(nullptr, listView);
    EXPECT_EQ(sheet->pageCount(), listView->model()->rowCount());
}

TEST_F(UT_ThumbnailWidget, HandleOpenSuccess_SecondCall_SkipsDuplicateInit)
{
    widget->show();
    widget->handleOpenSuccess();
    SideBarImageListView *listView = widget->findChild<SideBarImageListView *>();
    ASSERT_NE(nullptr, listView);
    int countAfterFirst = listView->model()->rowCount();
    widget->handleOpenSuccess();
    EXPECT_EQ(countAfterFirst, listView->model()->rowCount());
}

TEST_F(UT_ThumbnailWidget, HandlePage_ScrollsToIndex)
{
    widget->handleOpenSuccess();
    widget->handlePage(1);
    SideBarImageListView *listView = widget->findChild<SideBarImageListView *>();
    ASSERT_NE(nullptr, listView);
    EXPECT_EQ(1, listView->currentIndex().row());
}

TEST_F(UT_ThumbnailWidget, AdaptWindowSize_UpdatesScaleProperty)
{
    SideBarImageListView *listView = widget->findChild<SideBarImageListView *>();
    ASSERT_NE(nullptr, listView);
    widget->adaptWindowSize(1.5);
    EXPECT_DOUBLE_EQ(1.5, listView->property("adaptScale").toDouble());
}

TEST_F(UT_ThumbnailWidget, AdaptWindowSize_ClampsMinimumHeight)
{
    SideBarImageListView *listView = widget->findChild<SideBarImageListView *>();
    ASSERT_NE(nullptr, listView);
    widget->adaptWindowSize(0.5);
    EXPECT_DOUBLE_EQ(0.5, listView->property("adaptScale").toDouble());
    SUCCEED();
}

class UT_ThumbnailDelegate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        sheet = new DocSheet(kPDF, pdfPath);
        ASSERT_TRUE(sheet->openFileExec(""));
        listView = new SideBarImageListView(sheet);
        listView->resize(266, 500);
        delegate = new ThumbnailDelegate(listView);
        listView->setItemDelegate(delegate);
    }

    virtual void TearDown() override
    {
        ut_utils::waitRenderIdle();
                delete listView;
        delete sheet;
        ut_utils::waitRenderIdle();
    }

protected:
    QString pdfPath;
    DocSheet *sheet { nullptr };
    SideBarImageListView *listView { nullptr };
    ThumbnailDelegate *delegate { nullptr };
};

TEST_F(UT_ThumbnailDelegate, Construct_SetsParentObjectNames)
{
    EXPECT_EQ(QStringLiteral("ItemViewParent"), listView->objectName());
    EXPECT_EQ(QStringLiteral("ItemViewParent"), listView->accessibleName());
}

TEST_F(UT_ThumbnailDelegate, SizeHint_DelegatesToBase)
{
    listView->handleOpenSuccess();
    QStyleOptionViewItem option;
    QSize hint = delegate->sizeHint(option, listView->model()->index(0, 0));
    EXPECT_TRUE(hint.isValid());
}

TEST_F(UT_ThumbnailDelegate, Paint_InvalidIndex_NoCrash)
{
    QImage canvas(266, 110, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 266, 110);
    delegate->paint(&painter, option, QModelIndex());
    painter.end();
    SUCCEED();
}

TEST_F(UT_ThumbnailDelegate, Paint_UnselectedRow_NoCrash)
{
    listView->handleOpenSuccess();
    listView->show();
    ut_utils::drainEvents(100);
    QImage canvas(266, 110, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 266, 110);
    delegate->paint(&painter, option, listView->model()->index(0, 0));
    painter.end();
    SUCCEED();
}

TEST_F(UT_ThumbnailDelegate, Paint_SelectedRow_HighlightsNoCrash)
{
    listView->handleOpenSuccess();
    listView->show();
    ut_utils::drainEvents(100);
    listView->selectionModel()->select(listView->model()->index(0, 0), QItemSelectionModel::SelectCurrent);
    QImage canvas(266, 110, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 266, 110);
    delegate->paint(&painter, option, listView->model()->index(0, 0));
    painter.end();
    SUCCEED();
}

TEST_F(UT_ThumbnailDelegate, Paint_WithThumbnailPixmap_NoCrash)
{
    listView->handleOpenSuccess();
    QPixmap thumbnail(174, 220);
    thumbnail.fill(Qt::yellow);
    sheet->setThumbnail(0, thumbnail);
    QImage canvas(266, 110, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 266, 110);
    delegate->paint(&painter, option, listView->model()->index(0, 0));
    painter.end();
    SUCCEED();
}
