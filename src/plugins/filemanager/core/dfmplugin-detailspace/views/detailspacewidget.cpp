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
    initializeUi();
    initConnect();
    initUiForSizeMode();
}

void DetailSpaceWidget::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    setFixedWidth(DSizeModeHelper::element(260, 320));
    setFixedWidth(DSizeModeHelper::element(260, 320));
#else
    setFixedWidth(320);
#endif
}

void DetailSpaceWidget::setCurrentUrl(const QUrl &url)
{
    quint64 winId = DetailSpaceHelper::findWindowIdByDetailSpace(this);
    if (winId) {
        QList<QUrl> urls = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", winId).value<QList<QUrl>>();
        if (!urls.isEmpty()) {
            setCurrentUrl(urls.first(), 0);
            return;
        }
    }
    setCurrentUrl(url, 0);
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
    rvLayout->setMargin(0);
    detailView = new DetailView(this);
    DFrame *rightDetailVLine = new DFrame(this);
    rightDetailVLine->setFrameShape(QFrame::VLine);
    rvLayout->addWidget(rightDetailVLine);
    rvLayout->addWidget(detailView, 1);
}

void DetailSpaceWidget::initConnect()
{
}
