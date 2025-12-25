// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspacewidget.h"
#include "detailview.h"
#include "utils/detailspacehelper.h"

#include <dfm-framework/event/event.h>
#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QHBoxLayout>
#include <QApplication>

using namespace dfmplugin_detailspace;

DetailSpaceWidget::DetailSpaceWidget(QFrame *parent)
    : AbstractFrame(parent)
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &DetailSpaceWidget::initUiForSizeMode);
#endif

    initUiForSizeMode();
    initializeUi();
}

void DetailSpaceWidget::initUiForSizeMode()
{
    // Set size constraints for splitter-based resizing
    // Width is now controlled by the window's detailSplitter
    setMinimumWidth(kMinimumWidth);
    setMaximumWidth(kMaximumWidth);
}

void DetailSpaceWidget::setCurrentUrl(const QUrl &url)
{
    QUrl targetUrl = url;

    // If no URL provided, try to get selected URL from workspace
    if (!targetUrl.isValid()) {
        quint64 winId = DetailSpaceHelper::findWindowIdByDetailSpace(this);
        if (winId) {
            const QList<QUrl> &urls = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", winId).value<QList<QUrl>>();
            if (!urls.isEmpty()) {
                targetUrl = urls.first();
            }
        }
    }

    detailSpaceUrl = targetUrl;

    if (!isVisible())
        return;

    removeControls();
    detailView->setUrl(targetUrl);
}

QUrl DetailSpaceWidget::currentUrl() const
{
    return detailSpaceUrl;
}

bool DetailSpaceWidget::insertExpandControl(int index, QWidget *widget)
{
    return detailView->insertCustomControl(index, widget);
}

void DetailSpaceWidget::removeControls()
{
    detailView->removeControl();
}

void DetailSpaceWidget::initializeUi()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::ColorRole::Base);

    QHBoxLayout *rvLayout = new QHBoxLayout(this);
    rvLayout->setContentsMargins(0, 0, 0, 0);
    detailView = new DetailView(this);
    rvLayout->addWidget(detailView, 1);
    setLayout(rvLayout);
}
