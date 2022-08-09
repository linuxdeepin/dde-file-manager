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
#ifndef FILEPROXYMODEL_P_H
#define FILEPROXYMODEL_P_H

#include "fileproxymodel.h"

#include <file/local/localfileinfo.h>

#include <QTimer>

namespace ddplugin_organizer {

class FileProxyModelPrivate : public QObject
{
    Q_OBJECT
public:
    explicit FileProxyModelPrivate(FileProxyModel *qq);
    void reset();
    void clearMapping();
    void createMapping();
    void doRefresh(bool global);
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
    QMap<QUrl, DFMLocalFileInfoPointer> fileMap;
    QSharedPointer<QTimer> refreshTimer;
private:
    FileProxyModel *q;
};

}

#endif // FILEPROXYMODEL_P_H
