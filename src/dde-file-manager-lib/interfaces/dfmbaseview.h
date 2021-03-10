/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
