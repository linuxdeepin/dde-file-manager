/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
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
