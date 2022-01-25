/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
#ifndef CANVASMODEL_H
#define CANVASMODEL_H

#include "dfm_desktop_service_global.h"
#include "utils/fileutil.h"
#include "dfm-base/file/local/localfileinfo.h"

#include <QAbstractItemModel>
#include <QDir>

DSB_D_BEGIN_NAMESPACE

class CanvasModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CanvasModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(const QString &fileUrl, int column = 0);
    QModelIndex index(const DFMLocalFileInfoPointer &fileInfo, int column = 0) const;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    inline QModelIndex rootIndex() const {
        return createIndex(-2, 0, (quintptr)this);
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
    bool isRefreshed() const;

    QUrl rootUrl() const;
    QUrl url(const QModelIndex &index) const;
    DFMLocalFileInfoPointer fileInfo(const QModelIndex &index) const;
    const QList<QUrl> &getFiles() const;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    bool enableSort() const;
    void setEnabledSort(bool enabledSort);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    bool sort();

    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder &order);

    int sortRole() const;
    void setSortRole(dfmbase::AbstractFileInfo::SortKey role, Qt::SortOrder order = Qt::AscendingOrder);

    bool whetherShowHiddenFiles() const;
    void setWhetherShowHiddenFiles(const bool isShow);
signals:
    void fileCreated(const QUrl &url);
    void fileDeleted(const QUrl &url);
    void fileRenamed(const QUrl &oldUrl, const QUrl &newUrl);

    void fileRefreshed();
    void fileSorted();
    void enableSortChanged(bool enableSort);

private:
    void connection();
};
DSB_D_END_NAMESPACE
#endif   // CANVASMODEL_H
