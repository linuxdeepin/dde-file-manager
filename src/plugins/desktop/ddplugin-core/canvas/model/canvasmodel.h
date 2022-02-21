/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include <QtGlobal>

DSB_D_BEGIN_NAMESPACE

class CanvasModelPrivate;
class CanvasModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class FileTreater;
public:
    // tod(wangcl) some need del
    enum Roles {
        kFileIconRole = Qt::DecorationRole,
//        kFilePathRole = Qt::UserRole + 1,
        kFileNameRole = Qt::UserRole + 2,
        kFileSizeRole = Qt::UserRole + 3,
        kFileMimeTypeRole = Qt::UserRole + 4,
//        kFileOwnerRole = Qt::UserRole + 5,
        kFileLastModifiedRole = Qt::UserRole + 6,
//        kFileLastReadRole = Qt::UserRole + 7,
//        kFileCreatedRole = Qt::UserRole + 8,
        kFileDisplayNameRole = Qt::UserRole + 9,
        kFilePinyinName = Qt::UserRole + 10,
        kExtraProperties = Qt::UserRole + 11,
        kFileBaseNameRole = Qt::UserRole + 12,
        kFileSuffixRole = Qt::UserRole + 13,
        kFileNameOfRenameRole = Qt::UserRole + 14,
        kFileBaseNameOfRenameRole = Qt::UserRole + 15,
        kFileSuffixOfRenameRole = Qt::UserRole + 16,
//        kFileSizeInKiloByteRole = Qt::UserRole + 17,
//        kFileLastModifiedDateTimeRole = Qt::UserRole + 18,
//        kFileIconModelToolTipRole = Qt::UserRole + 19,
//        kFileLastReadDateTimeRole = Qt::UserRole + 20,
//        kFileCreatedDateTimeRole = Qt::UserRole + 21,
//        kFileUserRole = Qt::UserRole + 99,
        kUnknowRole = Qt::UserRole + 999
    };

    explicit CanvasModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column = 0,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(const QString &fileUrl, int column = 0);
    QModelIndex index(const DFMLocalFileInfoPointer &fileInfo, int column = 0) const;
    QModelIndexList indexs() const;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    inline QModelIndex rootIndex() const {
        return createIndex((quintptr)this, 0, (void*)this);
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
    bool isRefreshed() const;

    QModelIndex setRootUrl(QUrl url);
    QUrl rootUrl() const;
    QUrl url(const QModelIndex &index) const;
    DFMLocalFileInfoPointer fileInfo(const QModelIndex &index) const;
    QList<QUrl> files() const;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    bool sort();

    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder &order);

    int sortRole() const;
    void setSortRole(dfmbase::AbstractFileInfo::SortKey role, Qt::SortOrder order = Qt::AscendingOrder);

    bool showHiddenFiles() const;
    void setShowHiddenFiles(const bool isShow);

    inline QModelIndex createIndex(int arow, int acolumn, void *adata) const;

signals:
    void fileRenamed(const QUrl &oldUrl, const QUrl &newUrl);

private:
    QSharedPointer<CanvasModelPrivate> d;
};
DSB_D_END_NAMESPACE

#endif   // CANVASMODEL_H
