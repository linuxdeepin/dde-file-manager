// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMBASEVIEW_H
#define DFMBASEVIEW_H

#include "dfmglobal.h"
#include "durl.h"

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMBaseView
{
public:
    enum ViewState {
        ViewBusy,
        ViewIdle
    };

    DFMBaseView();
    virtual ~DFMBaseView();

    void deleteLater();

    virtual QWidget *widget() const = 0;
    virtual DUrl rootUrl() const = 0;
    virtual ViewState viewState() const;
    virtual bool setRootUrl(const DUrl &url) = 0;
    virtual QList<QAction*> toolBarActionList() const;
    virtual void refresh();

protected:
    void notifyUrlChanged();
    void notifyStateChanged();
    void requestCdTo(const DUrl &url);
    void notifySelectUrlChanged(const QList<DUrl> &urlList);
};

DFM_END_NAMESPACE

#endif // DFMBASEVIEW_H
