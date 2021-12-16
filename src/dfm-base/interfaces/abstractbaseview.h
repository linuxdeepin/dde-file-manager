/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef BASEVIEW_H
#define BASEVIEW_H

#include "dfm-base/dfm_base_global.h"

#include <QWidget>

DFMBASE_BEGIN_NAMESPACE

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
    virtual void refresh();

protected:
    void notifyUrlChanged();
    void notifyStateChanged();
    void requestCdTo(const QUrl &url);
    void notifySelectUrlChanged(const QList<QUrl> &urlList);
};

DFMBASE_END_NAMESPACE

#endif   // BASEVIEW_H
