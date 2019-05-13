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
#include <QVector>
#include <QSettings>
#include <QScopedPointer>

#include "gridcore.h"
#include "dabstractfileinfo.h"

#include "../global/coorinate.h"
#include "../global/singleton.h"

class GridCore;
class GridManagerPrivate;
class GridManager: public QObject, public Singleton<GridManager>
{
    Q_OBJECT
public:
    void initProfile(const QList<DAbstractFileInfoPointer> &items);
    void initWithoutProfile(const QList<DAbstractFileInfoPointer> &items);

    bool add(QPoint pos, const QString &itemId);
    bool add(const QString &itemId);
    bool move(const QStringList &selectedIds, const QString &itemId, int x, int y);
    bool remove(const QString &itemId);

    bool clear();

    QString firstItemId();
    QString lastItemId();
    QStringList itemIds();

    bool contains(const QString &itemId);
    QPoint position(const QString &itemId);
    QString itemId(int x, int y);
    QString itemId(QPoint pos);
    bool isEmpty(int x, int y);

    const QStringList &overlapItems() const;
    bool shouldArrange() const;
    bool autoArrange() const;
    bool autoMerge() const;
    void toggleArrange();
    void setAutoMerge(bool enable = true);
    void toggleAutoMerge();
    void reArrange();

    int gridCount() const;
    QPoint forwardFindEmpty(QPoint start) const;
    QSize gridSize() const;
    void updateGridSize(int w, int h);

    GridCore *core();

    void setWhetherShowHiddenFiles(bool value)noexcept;
    bool getWhetherShowHiddenFiles()noexcept;

public:
    void dump();

protected:
    bool remove(int x, int y, const QString &itemId);
    bool remove(QPoint pos, const QString &itemId);

    friend class Singleton<GridManager>;

    GridManager();
    ~GridManager();

    QScopedPointer<GridManagerPrivate> d;
};
