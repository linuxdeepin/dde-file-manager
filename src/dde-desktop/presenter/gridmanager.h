// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
class GridManager: public QObject, public DDEDesktop::Singleton<GridManager>
{
    Q_OBJECT
public:
    enum SyncOperation {soAutoMerge, soRename, soIconSize, soSort
    , soHideEditing, soUpdate, soAutoMergeUpdate
    , soHidenSwitch, soGsettingUpdate, soExpandItemUpdate
                       };
    DUrl getInitRootUrl();
    void initGridItemsInfos();
    void initAutoMerge(const QList<DAbstractFileInfoPointer> &items);
    void initArrage(const QStringList &items);
    void initCustom(QStringList &items);
    void initCustom(const QStringList &orderedItems, const QHash<QString, bool> &indexHash);

    bool add(int screenNum, QPoint pos, const QString &id);
    bool add(int screenNum, const QString &id);
    bool move(int screenNum, const QStringList &selecteds, const QString &current, int x, int y);
    bool move(int fromScreen, int toScreen, const QStringList &selectedIds, const QString &itemId, int x, int y);
    bool remove(int screenNum, const QString &id);
    void popOverlap(); //弹出堆叠
    int addToOverlap(const QString &itemId);
    int emptyPostionCount(int screenNum) const;

    bool clear();
    void restCoord();

    void addCoord(int screenNum, QPair<int, int> coordInfo);
    QString firstItemId(int screenNum);
    QString lastItemId(int screenNum);
    QString lastItemTop(int screenNum);
    QStringList itemIds(int screenNum);
    QStringList allItems() const;

    bool contains(int screebNum, const QString &id);
    QPoint position(int screenNum, const QString &id);
    bool find(const QString &itemId, QPair<int, QPoint> &pos);
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
    QList<int> allScreenNum();
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
    QVariant isGsettingShow(const QString &targetkey, const QVariant defaultValue);
    bool desktopFileShow(const DUrl &url, const bool defaultValue);

public:
    void dump();
    static void sortMainDesktopFile(QStringList &list, int role, Qt::SortOrder order);
    QString m_needRenameItem; //用于排完顺序后打开编辑框
signals:
    void sigSyncOperation(int so, QVariant var = QVariant());
    void sigSyncSelection(class CanvasGridView *, DUrlList url);
protected:
    //bool remove(int screenNum, int x, int y, const QString &itemId);
    bool remove(int screenNum, QPoint pos, const QString &id);

    friend class DDEDesktop::Singleton<GridManager>;

    GridManager();
    ~GridManager();

    QScopedPointer<GridManagerPrivate> d;
};
