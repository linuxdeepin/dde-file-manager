/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include <QMap>
#include <QSettings>
#include <QScopedPointer>

#include "../global/coorinate.h"
#include "../global/singleton.h"

class GridManagerPrivate;
class GridManager: public QObject, public Singleton<GridManager>
{
    Q_OBJECT
public:
    bool isInited() const;
    void initProfile(const QStringList &items);

    bool add(const QString &itemId);
    bool move(const QStringList &selectedIds, const QString &itemId, int x, int y);
    bool remove(const QString &itemId);

    bool clear();

    QString firstItemId();
    QString lastItemId();

    bool contains(const QString &itemId);
    QPoint position(const QString &itemId);
    QString itemId(int x, int y);
    QString itemId(QPoint pos);
    bool isEmpty(int x, int y);

    const QStringList &overlapItems() const;
    bool autoAlign();
    void toggleAlign();
    void reAlign();

    QPoint forwardFindEmpty(QPoint start) const;
    QSize gridSize() const;
    void updateGridSize(int w, int h);

protected:
    bool remove(int x, int y, const QString &itemId);
    bool remove(QPoint pos, const QString &itemId);
    bool add(QPoint pos, const QString &itemId);

    friend class Singleton<GridManager>;

    GridManager();
    ~GridManager();

    QScopedPointer<GridManagerPrivate> d;
};
