// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmbaseview.h"
#include "views/dfilemanagerwindow.h"

#include <QObject>

DFM_BEGIN_NAMESPACE

DFMBaseView::DFMBaseView()
{

}

DFMBaseView::~DFMBaseView()
{

}

void DFMBaseView::deleteLater()
{
    if (QObject *obj = dynamic_cast<QObject*>(this))
        return obj->deleteLater();

    delete this;
}

DFMBaseView::ViewState DFMBaseView::viewState() const
{
    return ViewIdle;
}

QList<QAction *> DFMBaseView::toolBarActionList() const
{
    return QList<QAction*>();
}

void DFMBaseView::refresh()
{

}

void DFMBaseView::notifyUrlChanged()
{
    if (DFileManagerWindow *w = qobject_cast<DFileManagerWindow*>(widget()->window())) {
        w->currentUrlChanged();
    }
}

void DFMBaseView::notifyStateChanged()
{
    if (DFileManagerWindow *w = qobject_cast<DFileManagerWindow*>(widget()->window())) {
        w->currentViewStateChanged();
    }
}

void DFMBaseView::requestCdTo(const DUrl &url)
{
    if (DFileManagerWindow *w = qobject_cast<DFileManagerWindow*>(widget()->window())) {
        w->cdForTabByView(this, url);
    }
}

void DFMBaseView::notifySelectUrlChanged(const QList<DUrl> &urlList)
{
    if (DFileManagerWindow *w = qobject_cast<DFileManagerWindow*>(widget()->window())) {
        emit w->selectUrlChanged(urlList);
    }
}

DFM_END_NAMESPACE
