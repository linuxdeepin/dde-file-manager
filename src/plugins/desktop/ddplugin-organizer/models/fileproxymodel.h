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
#ifndef FILEPROXYMODEL_H
#define FILEPROXYMODEL_H

#include "ddplugin_organizer_global.h"

#include <file/local/localfileinfo.h>

#include <QAbstractProxyModel>

DDP_ORGANIZER_BEGIN_NAMESPACE
class ModelDataHandler;
class FileInfoModelShell;
class FileProxyModelPrivate;
class FileProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    friend class FileProxyModelPrivate;
public:
    explicit FileProxyModel(QObject *parent = nullptr);
    ~FileProxyModel();
    void setModelShell(FileInfoModelShell *shell);
    void setHandler(ModelDataHandler *);
    ModelDataHandler *handler() const;
    QModelIndex rootIndex() const;
    QModelIndex index(const QUrl &url, int column = 0) const;
    QList<QUrl> files() const;
    QUrl fileUrl(const QModelIndex &index) const;
    void refresh(const QModelIndex &parent, bool global = false, int ms = 50);
    void update();
public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
signals:
    void dataReplaced(const QUrl &oldUrl, const QUrl &newUrl);
private:
    void setSourceModel(QAbstractItemModel *sourceModel) override;
private:
    FileProxyModelPrivate *d;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // FILEPROXYMODEL_H
