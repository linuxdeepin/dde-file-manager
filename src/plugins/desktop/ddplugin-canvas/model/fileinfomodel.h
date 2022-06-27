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
#ifndef FILEINFOMODEL_H
#define FILEINFOMODEL_H

#include "ddplugin_canvas_global.h"

#include <file/local/localfileinfo.h>

#include <QAbstractItemModel>

namespace ddplugin_canvas {
class FileFilter;
class FileInfoModelPrivate;
class FileInfoModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class FileInfoModelPrivate;
public:
    explicit FileInfoModel(QObject *parent = nullptr);
    ~FileInfoModel();
    QModelIndex setRootUrl(QUrl url);
    Q_INVOKABLE QUrl rootUrl() const;
    Q_INVOKABLE QModelIndex rootIndex() const;

    void installFilter(QSharedPointer<FileFilter> filter);
    void removeFilter(QSharedPointer<FileFilter> filter);

    Q_INVOKABLE QModelIndex index(const QUrl &url, int column = 0) const;
    Q_INVOKABLE DFMLocalFileInfoPointer fileInfo(const QModelIndex &index) const;
    Q_INVOKABLE QUrl fileUrl(const QModelIndex &index) const;
    Q_INVOKABLE QList<QUrl> files() const;
    Q_INVOKABLE void refresh(const QModelIndex &parent);
    Q_INVOKABLE void update();
public:
    QModelIndex index(int row, int column = 0,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int itemRole = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
signals:
    //! the receiver must resolve the case that newurl has been existed if connecting with Qt::DirectConnection
    void dataReplaced(const QUrl &oldUrl, const QUrl &newUrl);
private:
    FileInfoModelPrivate *d;
};

}

#endif // FILEINFOMODEL_H
