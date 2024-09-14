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
    initConnect();
}

void DetailSpaceWidget::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    setFixedWidth(DSizeModeHelper::element(260, 290));
    setFixedWidth(DSizeModeHelper::element(260, 290));
#else
    setFixedWidth(290);
#endif
}

void DetailSpaceWidget::setCurrentUrl(const QUrl &url)
{
#if 0
    quint64 winId = DetailSpaceHelper::findWindowIdByDetailSpace(this);
    if (winId) {
        QList<QUrl> urls = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", winId).value<QList<QUrl>>();
        if (!urls.isEmpty()) {
            setCurrentUrl(urls.first(), 0);
            return;
        }
    }
    setCurrentUrl(url, 0);
#endif
}

void DetailSpaceWidget::setCurrentUrl(const QUrl &url, int widgetFilter)
{
    detailSpaceUrl = url;

    if (!isVisible())
        return;

    removeControls();
    detailView->setUrl(url, widgetFilter);
}

QUrl DetailSpaceWidget::currentUrl() const
{
    return detailSpaceUrl;
}

bool DetailSpaceWidget::insterExpandControl(const int &index, QWidget *widget)
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
    rvLayout->addWidget(detailView);
    setLayout(rvLayout);
}

void DetailSpaceWidget::initConnect()
{
}
