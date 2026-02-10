// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEVIEW_H
#define BASEVIEW_H

#include <dfm-base/dfm_base_global.h>

#include <QWidget>
#include <QUrl>

namespace dfmbase {

class AbstractBaseView
{
public:
    enum class ViewState : uint8_t {
        kViewBusy,
        kViewIdle
    };

    AbstractBaseView();
    virtual ~AbstractBaseView();

    void deleteLater();

    virtual QWidget *widget() const = 0;
    virtual QUrl rootUrl() const = 0;
    virtual ViewState viewState() const;
    virtual bool setRootUrl(const QUrl &url) = 0;
    virtual QList<QAction *> toolBarActionList() const;
    virtual QList<QUrl> selectedUrlList() const;
    virtual void refresh();
    virtual QWidget *contentWidget() const;

protected:
    void notifyStateChanged();
    void requestCdTo(const QUrl &url);
    void notifySelectUrlChanged(const QList<QUrl> &urlList);
};

}

#endif   // BASEVIEW_H
