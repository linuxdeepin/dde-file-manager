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

#if 0 //多屏图标自定义配置做修改： older
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
#endif
#if 1 //多屏图标自定义配置做修改： newer
class GridCore;
class GridManagerPrivate;
class GridManager: public QObject, public Singleton<GridManager>
{
    Q_OBJECT
public:

    DUrl getInitRootUrl();
    void initGridItemsInfos();
    void initProfile(const QList<DAbstractFileInfoPointer> &items);
    void initWithoutProfile(const QList<DAbstractFileInfoPointer> &items);

    bool add(int screenNum, QPoint pos, const QString &itemId);
    bool add(int screenNum, const QString &itemId);
    bool move(int screenNum, const QStringList &selectedIds, const QString &itemId, int x, int y);
    bool move(int fromScreen, int toScreen, const QStringList &selectedIds, const QString &itemId, int x, int y);
    bool remove(int screenNum, const QString &itemId);

    bool clear();

    void addCoord(int screenNum, QPair<int, int> coordInfo);
    QString firstItemId(int screenNum);
    QString lastItemId(int screenNum);
    QStringList itemIds(int screenNum);

    bool contains(int screebNum, const QString &itemId);
    QPoint position(int screenNum, const QString &itemId);
    QString itemId(int screenNum, int x, int y);
    QString itemId(int screenNum, QPoint pos);
    bool isEmpty(int screenNum, int x, int y);

    const QStringList &overlapItems() const;
    bool shouldArrange() const;
    bool autoArrange() const;
    bool autoMerge() const;
    void toggleArrange(int screenNum);
    void setAutoMerge(bool enable = true);
    void toggleAutoMerge();
    void reArrange(int screenNum);

    int gridCount(int screenNum) const;
    QPair<int, QPoint> forwardFindEmpty(int screenNum, QPoint start) const;
    QSize gridSize(int screenNum) const;
    void updateGridSize(int screenNum, int w, int h);

    GridCore *core();

    void setWhetherShowHiddenFiles(bool value)noexcept;
    bool getWhetherShowHiddenFiles()noexcept;

public:
    void dump();

protected:
    bool remove(int screenNum, int x, int y, const QString &itemId);
    bool remove(int screenNum, QPoint pos, const QString &itemId);

    friend class Singleton<GridManager>;

    GridManager();
    ~GridManager();

    QScopedPointer<GridManagerPrivate> d;
};
#endif
