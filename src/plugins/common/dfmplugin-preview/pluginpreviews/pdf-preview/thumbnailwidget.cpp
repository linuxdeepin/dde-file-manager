/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
