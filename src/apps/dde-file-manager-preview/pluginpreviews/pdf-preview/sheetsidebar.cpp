// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sheetsidebar.h"
#include "docsheet.h"
#include "thumbnailwidget.h"
#include "model.h"

#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QTimer>
#include <QResizeEvent>

DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;
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
