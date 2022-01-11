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
#ifndef CANVASGRID_H
#define CANVASGRID_H

#include "dfm_desktop_service_global.h"

#include <QObject>

extern uint qHash(const QPoint &key, uint seed);

DSB_D_BEGIN_NAMESPACE

class CanvasGridPrivate;
class CanvasGrid : public QObject
{
    Q_OBJECT
    friend class CanvasGridPrivate;
public:
    enum class Mode {Custom = 0, Align};
public:
    static CanvasGrid *instance();
    void initSurface(int count = 0);

    void updateSize(int index, const QSize &size);
    QSize surfaceSize(int index) const;
    int gridCount(int index = -1) const;

    void setMode(Mode mode);
    Mode mode() const;

    void setItems(const QStringList &items);
    QStringList items(int index = -1) const;
    QString item(int index, const QPoint &pos) const;
    QHash<QString, QPoint> points(int index) const;
    bool point(const QString &item, QPair<int, QPoint> &pos) const;

    QStringList overloadItems(int index) const;

    bool drop(int index, const QPoint &pos, const QString &item);
    bool move(int toIndex, const QPoint &toPos, const QString &focus, const QStringList &items);
    bool remove(int index, const QString &item);
    void append(const QString &item);
    void append(QStringList items);
    void tryAppendAfter(const QStringList &items, int index, const QPoint &begin);
    void popOverload();
signals:

public slots:

protected:
    explicit CanvasGrid(QObject *parent = nullptr);
    ~CanvasGrid();
private:
    CanvasGridPrivate *d;
};

#define GridIns DSB_D_NAMESPACE::CanvasGrid::instance()
DSB_D_END_NAMESPACE

#endif // CANVASGRID_H
