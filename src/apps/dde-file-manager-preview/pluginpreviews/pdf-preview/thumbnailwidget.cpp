// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailwidget.h"
#include "docsheet.h"
#include "sidebarimagelistview.h"
#include "sidebarimageviewmodel.h"
#include "thumbnaildelegate.h"

#include <DHorizontalLine>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;
const int LEFTMINHEIGHT = 110;

ThumbnailWidget::ThumbnailWidget(DocSheet *sheet, DWidget *parent)
    : DWidget(parent), docSheet(sheet)
{
    initWidget();
}

ThumbnailWidget::~ThumbnailWidget()
{
}

void ThumbnailWidget::initWidget()
{
    pImageListView = new SideBarImageListView(docSheet, this);
    pImageListView->setAccessibleName("View_ImageList");
    ThumbnailDelegate *imageDelegate = new ThumbnailDelegate(pImageListView);
    pImageListView->setItemDelegate(imageDelegate);

    QVBoxLayout *vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setSpacing(0);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addWidget(pImageListView);

    this->setLayout(vBoxLayout);
    pImageListView->setItemSize(QSize(LEFTMINWIDTH, LEFTMINHEIGHT));
}

void ThumbnailWidget::handleOpenSuccess()
{
    if (bIshandOpenSuccess)
        return;
    bIshandOpenSuccess = true;
    pImageListView->handleOpenSuccess();
    scrollToCurrentPage();
}

void ThumbnailWidget::handlePage(int index)
{
    pImageListView->scrollToIndex(index);
}

void ThumbnailWidget::adaptWindowSize(const double &scale)
{
    pImageListView->setProperty("adaptScale", scale);
    pImageListView->setItemSize(QSize(static_cast<int>(LEFTMINWIDTH * scale), static_cast<int>(qMax(LEFTMINHEIGHT * scale, LEFTMINHEIGHT * 1.0))));
    pImageListView->reset();
    scrollToCurrentPage();
}

void ThumbnailWidget::scrollToCurrentPage()
{
    pImageListView->scrollToIndex(docSheet->currentIndex());
}
