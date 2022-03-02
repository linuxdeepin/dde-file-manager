/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef OPERSTATE_H
#define OPERSTATE_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QModelIndex>

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasView;
class OperState : public QObject
{
    Q_OBJECT
public:
    explicit OperState(QObject *parent = nullptr);
    void setView(CanvasView *v);
    QModelIndex current() const;
    void setCurrent(const QModelIndex &value);

    inline QModelIndex getContBegin() const {
        return contBegin;
    }

    inline void setContBegin(const QModelIndex &value) {
        contBegin = value;
    }
signals:

public slots:
protected slots:
    void selectionChanged();
protected:
    // todo(zy) using url to instead index.
    CanvasView *view = nullptr;
    QPersistentModelIndex contBegin; //for shift
};
DDP_CANVAS_END_NAMESPACE
#endif // OPERSTATE_H
