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
#ifndef ABSTRACTSCREEN_H
#define ABSTRACTSCREEN_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QRect>

DFMBASE_BEGIN_NAMESPACE
class AbstractScreen : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractScreen)
public:
    explicit AbstractScreen(QObject *parent = nullptr): QObject (parent){}
    virtual ~AbstractScreen(){}
    virtual bool isValid() const {return false;}
    virtual QString name() const {return "";}
    virtual QRect geometry() const {return QRect(0,0,0,0);}
    virtual QRect availableGeometry() const {return QRect(0,0,0,0);}
    virtual QRect handleGeometry() const {return QRect(0,0,0,0);}
signals:
    void geometryChanged(const QRect &);
    void availableGeometryChanged(const QRect &);
};
DFMBASE_END_NAMESPACE

#endif // ABSTRACTSCREEN_H
