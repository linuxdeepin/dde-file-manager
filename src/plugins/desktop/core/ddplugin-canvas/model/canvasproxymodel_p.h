/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef CANVASPROXYMODEL_P_H
#define CANVASPROXYMODEL_P_H

#include "canvasproxymodel.h"
#include "fileinfomodel.h"
#include "modelhookinterface.h"
#include "canvasmodelfilter.h"

#include <dfm_global_defines.h>

#include <QTimer>

namespace ddplugin_canvas {

class CanvasProxyModelPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CanvasProxyModelPrivate(CanvasProxyModel *qq);
    void clearMapping();
    void createMapping();
    QModelIndexList indexs() const;
    bool doSort(QList<QUrl> &files) const;
    bool lessThan(const QUrl &left, const QUrl &right) const;
public slots:
    void doRefresh(bool global);
    void sourceDataChanged(const QModelIndex &sourceTopleft,
                              const QModelIndex &sourceBottomright,
                              const QVector<int> &roles);
    void sourceAboutToBeReset();
    void sourceReset();

    void sourceRowsInserted(const QModelIndex &sourceParent,
                               int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent,
                                       int start, int end);
    void sourceDataRenamed(const QUrl &oldUrl, const QUrl &newUrl);
public:
    bool insertFilter(const QUrl &url);
    bool resetFilter(QList<QUrl> &urls);
    bool updateFilter(const QUrl &url);
    bool removeFilter(const QUrl &url);
    bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl);
protected:
   void standardSort(QList<QUrl> &files) const;
   void specialSort(QList<QUrl> &files) const;
private:
    void sortMainDesktopFile(QList<QUrl> &files, Qt::SortOrder order) const;
public:
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    QList<QUrl> fileList;
    QMap<QUrl, DFMLocalFileInfoPointer> fileMap;
    FileInfoModel *srcModel = nullptr;
    QSharedPointer<QTimer> refreshTimer;
    int fileSortRole = DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole;
    Qt::SortOrder fileSortOrder = Qt::AscendingOrder;
    ModelHookInterface *hookIfs = nullptr;
    QList<QSharedPointer<CanvasModelFilter>> modelFilters;
    bool isNotMixDirAndFile = false;
private:
    CanvasProxyModel *q;
};


}

#endif // CANVASPROXYMODEL_P_H
