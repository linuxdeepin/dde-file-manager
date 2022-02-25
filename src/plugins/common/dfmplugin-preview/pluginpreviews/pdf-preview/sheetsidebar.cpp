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

#include "sheetsidebar.h"
#include "docsheet.h"
#include "thumbnailwidget.h"
#include "model.h"

#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QTimer>
#include <QResizeEvent>

DWIDGET_USE_NAMESPACE
PREVIEW_USE_NAMESPACE
const int LEFTMINWIDTH = 50;
const int LEFTMAXWIDTH = 380;
SheetSidebar::SheetSidebar(DocSheet *parent, PreviewWidgesFlags widgesFlag)
    : QWidget(parent), docSheet(parent), widgetsFlag(widgesFlag)
{
    initWidget();
}

SheetSidebar::~SheetSidebar()
{
}

void SheetSidebar::initWidget()
{
    scaleRatio = 1.0;
    oldVisible = false;
    openDocOpenSuccess = false;
    thumbnailWidget = nullptr;

    setFixedWidth(50);
    QVBoxLayout *pVBoxLayout = new QVBoxLayout;
    pVBoxLayout->setContentsMargins(0, 0, 0, 0);
    pVBoxLayout->setSpacing(0);
    this->setLayout(pVBoxLayout);

    stackLayout = new QStackedLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(15, 0, 15, 0);

    if (widgetsFlag.testFlag(PREVIEW_THUMBNAIL)) {
        thumbnailWidget = new ThumbnailWidget(docSheet, this);
        stackLayout->addWidget(thumbnailWidget);
    }

    //! remove last spaceitem
    QLayoutItem *item = hLayout->takeAt(hLayout->count() - 1);
    if (item) delete item;

    pVBoxLayout->addLayout(stackLayout);
    pVBoxLayout->addLayout(hLayout);
    this->setVisible(false);
}

void SheetSidebar::setCurrentPage(int page)
{
    if (thumbnailWidget) thumbnailWidget->handlePage(page - 1);
}

void SheetSidebar::handleOpenSuccess()
{
    openDocOpenSuccess = true;
    this->setVisible(docSheet->operation().sidebarVisible);
    onHandleOpenSuccessDelay();
}

void SheetSidebar::onHandleOpenSuccessDelay()
{
    if (openDocOpenSuccess) {
        QTimer::singleShot(100, this, SLOT(onHandWidgetDocOpenSuccess()));
    }
}

void SheetSidebar::onHandWidgetDocOpenSuccess()
{
    if (!this->isVisible())
        return;

    QWidget *curWidget = stackLayout->currentWidget();
    if (curWidget == thumbnailWidget) {
        thumbnailWidget->handleOpenSuccess();
    }
}

void SheetSidebar::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
}

void SheetSidebar::showEvent(QShowEvent *event)
{
    DWidget::showEvent(event);
    onHandleOpenSuccessDelay();
}
