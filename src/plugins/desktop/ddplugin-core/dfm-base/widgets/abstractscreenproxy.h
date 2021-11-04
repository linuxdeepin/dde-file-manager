/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#ifndef ABSTRACTSCREENPROXY_H
#define ABSTRACTSCREENPROXY_H

#include "abstractscreen.h"
#include "dfm-base/dfm_base_global.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE

class AbstractScreenProxy : public QObject
{
    Q_OBJECT
public:
    explicit AbstractScreenProxy(QObject *parent = nullptr);

    virtual ~AbstractScreenProxy(){
        for (auto && val :screenList) {
            delete val;
            screenList.removeOne(val);
        }
    }

    virtual QList<dfmbase::AbstractScreen*> allScreen();

signals:
    void screenChanged(dfmbase::AbstractScreen *screen);
    void screenRemoved(dfmbase::AbstractScreen *screen);
    void screenAdded(dfmbase::AbstractScreen *screen);

protected:
    QList<dfmbase::AbstractScreen*> screenList;
};

DFMBASE_END_NAMESPACE

#endif // ABSTRACTSCREENPROXY_H
