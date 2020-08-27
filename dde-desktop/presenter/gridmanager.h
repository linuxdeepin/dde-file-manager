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
    enum SyncOperation{soAutoMerge,soRename,soIconSize,soSort
                       ,soHideEditing,soUpdate,soAutoMergeUpdate
                       ,soHidenSwitch};
    DUrl getInitRootUrl();
    void initGridItemsInfos();
    void initAutoMerge(const QList<DAbstractFileInfoPointer> &items);
    void initArrage(const QStringList &items);
    void initCustom(const QStringList &orderedItems, const QHash<QString, bool> &indexHash);
    void initCustom(const QStringList &items);

    bool add(int screenNum, QPoint pos, const QString &itemId);
    bool add(int screenNum, const QString &itemId);
    bool move(int screenNum, const QStringList &selectedIds, const QString &itemId, int x, int y);
    bool move(int fromScreen, int toScreen, const QStringList &selectedIds, const QString &itemId, int x, int y);
    bool remove(int screenNum, const QString &itemId);
    void popOverlap(); //弹出堆叠
    int emptyPostionCount(int screenNum) const;

    bool clear();
    void restCoord();

    void addCoord(int screenNum, QPair<int, int> coordInfo);
    QString firstItemId(int screenNum);
    QString lastItemId(int screenNum);
    QString lastItemTop(int screenNum);
    QStringList itemIds(int screenNum);
    QStringList allItems() const;

    bool contains(int screebNum, const QString &itemId);
    QPoint position(int screenNum, const QString &itemId);
    bool find(const QString &itemId, QPair<int,QPoint> &pos);
    QString itemId(int screenNum, int x, int y);
    QString itemId(int screenNum, QPoint pos);
    QString itemTop(int screenNum, int x, int y); //调整显示方式，如果是堆叠，则将最后一个pos的换成堆叠的最后一个项目
    QString itemTop(int screenNum, QPoint pos);
    bool isEmpty(int screenNum, int x, int y);

    QStringList overlapItems(int screen) const;
    bool shouldArrange() const;
    bool autoArrange() const;
    bool autoMerge() const;
    bool doneInit()const;
    void toggleArrange();
    void setAutoMerge(bool enable = true);
    void toggleAutoMerge();
    void reArrange();

    int gridCount() const;
    int gridCount(int screenNum) const;
    QPair<int, QPoint> forwardFindEmpty(int screenNum, QPoint start) const;
    QSize gridSize(int screenNum) const;
    void updateGridSize(int screenNum, int w, int h);

    GridCore *core();

    void setWhetherShowHiddenFiles(bool value)noexcept;
    bool getWhetherShowHiddenFiles()noexcept;

    bool getCanvasFullStatus(int screenId);

    void setDisplayMode(bool single);
    void delaySyncAllProfile(int ms = 100);

    void setCurrentVirtualExpandUrl(const DUrl url);
    DUrl getCurrentVirtualExpandUrl();
    void setCurrentAllItems(const QList<DAbstractFileInfoPointer> &infoList);

public:
    void dump();
    static void sortMainDesktopFile(QStringList &list, int role, Qt::SortOrder order);
    QString m_needRenameItem; //用于排完顺序后打开编辑框
signals:
    void sigSyncOperation(int so,QVariant var = QVariant());
    void sigSyncSelection(class CanvasGridView *, DUrlList url);
protected:
    //bool remove(int screenNum, int x, int y, const QString &itemId);
    bool remove(int screenNum, QPoint pos, const QString &itemId);

    friend class Singleton<GridManager>;

    GridManager();
    ~GridManager();

    QScopedPointer<GridManagerPrivate> d;
};
#endif
