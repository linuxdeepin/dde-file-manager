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
#ifndef CLICKSELECTER_H
#define CLICKSELECTER_H
#include "dfm_desktop_service_global.h"

#include <QObject>
#include <QModelIndex>

DSB_D_BEGIN_NAMESPACE
class CanvasView;
class ClickSelecter : public QObject
{
    Q_OBJECT
public:
    explicit ClickSelecter(CanvasView *parent);
    void click(const QModelIndex &index);
    void release(const QModelIndex &index);
signals:

public slots:
protected:
    static void order(const QPoint &p1, const QPoint &p2, QPoint &from, QPoint &to);
    static QList<QPoint> horizontalTraversal(const QPoint &from, const QPoint &to, const QSize &size);
    void clear();
    void toggleSelect(const QModelIndex &index);
    void continuesSelect(const QModelIndex &index);
    void traverseSelect(const QModelIndex &from, const QModelIndex &to);
    void traverseSelect(const QPoint &p1, const QPoint &p2);
private:
    void singleSelect(const QModelIndex &index);
protected:
    CanvasView *view;
    QPersistentModelIndex lastPressedIndex; // todo using url
};
DSB_D_END_NAMESPACE
#endif // CLICKSELECTER_H
