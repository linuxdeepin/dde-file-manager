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
#ifndef FILEINFOMODEL_P_H
#define FILEINFOMODEL_P_H

#include "fileinfomodel.h"
#include "fileprovider.h"

#include <QReadWriteLock>

DDP_CANVAS_BEGIN_NAMESPACE

class FileInfoModelPrivate : public QObject
{
    Q_OBJECT
public:
    explicit FileInfoModelPrivate(FileInfoModel *qq);
    void doRefresh();
public slots:
    void resetData(const QList<QUrl> &urls);
    void insertData(const QUrl &url);
    void removeData(const QUrl &url);
    void replaceData(const QUrl &oldUrl, const QUrl &newUrl);
    void updateData(const QUrl &url);
public:
    QDir::Filters filters = QDir::NoFilter;
    FileProvider *fileProvider = nullptr;
    QList<QUrl> fileList;
    QMap<QUrl, DFMLocalFileInfoPointer> fileMap;
    QReadWriteLock lock;
private:
    FileInfoModel *q;
};

DDP_CANVAS_END_NAMESPACE

#endif // FILEINFOMODEL_P_H
