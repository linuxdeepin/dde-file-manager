// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASPROXYMODEL_P_H
#define CANVASPROXYMODEL_P_H

#include "canvasproxymodel.h"
#include "fileinfomodel.h"
#include "modelhookinterface.h"
#include "canvasmodelfilter.h"

#include <dfm-base/dfm_global_defines.h>

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
    QModelIndexList indexs(const QList<QUrl> &files) const;
    bool doSort(QList<QUrl> &files) const;
    bool lessThan(const QUrl &left, const QUrl &right) const;
public slots:
    void doRefresh(bool global, bool updateFile);
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
    bool updateFilter(const QUrl &url , const QVector<int> &roles);
    bool removeFilter(const QUrl &url);
    bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl);

protected:
    void standardSort(QList<QUrl> &files) const;
    void specialSort(QList<QUrl> &files) const;

private:
    void sortMainDesktopFile(QList<QUrl> &files, Qt::SortOrder order) const;
    void sendLoadReport();

public:
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    QList<QUrl> fileList;
    QMap<QUrl, FileInfoPointer> fileMap;
    FileInfoModel *srcModel = nullptr;
    QSharedPointer<QTimer> refreshTimer;
    int fileSortRole = DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole;
    Qt::SortOrder fileSortOrder = Qt::AscendingOrder;
    ModelHookInterface *hookIfs = nullptr;
    QList<QSharedPointer<CanvasModelFilter>> modelFilters;
    bool isNotMixDirAndFile = false;

private:
    CanvasProxyModel *q = nullptr;
};

}

#endif   // CANVASPROXYMODEL_P_H
