// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONMODEL_P_H
#define COLLECTIONMODEL_P_H

#include "collectionmodel.h"

#include <dfm-base/file/local/syncfileinfo.h>

#include <QTimer>

namespace ddplugin_organizer {

class CollectionModelPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CollectionModelPrivate(CollectionModel *qq);
    ~CollectionModelPrivate();
    void reset();
    void clearMapping();
    void createMapping();
    void doRefresh(bool global, bool file);
public slots:
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
    FileInfoModelShell *shell = nullptr;
    ModelDataHandler *handler = nullptr;
    QList<QUrl> fileList;
    QMap<QUrl, FileInfoPointer> fileMap;
    QSharedPointer<QTimer> refreshTimer;

private:
    CollectionModel *q;
};

}

#endif   // COLLECTIONMODEL_P_H
