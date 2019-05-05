/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DFILESYSTEMMODEL_H
#define DFILESYSTEMMODEL_H

#include <QAbstractItemModel>
#include <QPointer>
#include <QDir>
#include <QFuture>

// for std::shared_ptr
#include <iostream>
#include <memory>

#include "durl.h"
#include "dabstractfileinfo.h"

QT_BEGIN_NAMESPACE
class QEventLoop;
class QReadWriteLock;
QT_END_NAMESPACE

class FileSystemNode;
class DAbstractFileInfo;
class DFMEvent;
class JobController;
class DFileViewHelper;
class DFMUrlListBaseEvent;

enum _asb_LabelIndex {
    SEARCH_RANGE, FILE_TYPE, SIZE_RANGE, DATE_RANGE, LABEL_COUNT,
    TRIGGER_SEARCH = 114514
};

typedef struct fileFilter {
    QMap<int, QVariant> filterRule; // for advanced search.
    bool filterEnabled = false;
    QPair<quint64, quint64> f_sizeRange;
    QDateTime f_dateRangeStart;
    QDateTime f_dateRangeEnd;
    QString f_typeString;
    bool f_includeSubDir;
    bool f_comboValid[LABEL_COUNT];
} FileFilter;

typedef QExplicitlySharedDataPointer<FileSystemNode> FileSystemNodePointer;
class DFileSystemModelPrivate;
class DFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(bool enabledSort READ enabledSort WRITE setEnabledSort NOTIFY enabledSortChanged)

public:
    enum Roles {
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
        FileNameRole = Qt::UserRole + 2,
        FileSizeRole = Qt::UserRole + 3, // a QString
        FileMimeTypeRole = Qt::UserRole + 4, // a QString
        FileOwnerRole = Qt::UserRole + 5,
        FileLastModifiedRole = Qt::UserRole + 6, // a QString
        FileLastReadRole = Qt::UserRole + 7,
        FileCreatedRole = Qt::UserRole + 8,
        FileDisplayNameRole = Qt::UserRole + 9,
        FilePinyinName = Qt::UserRole + 10,
        ExtraProperties = Qt::UserRole + 11,
        FileBaseNameRole = Qt::UserRole + 12,
        FileSuffixRole = Qt::UserRole + 13,
        FileNameOfRenameRole = Qt::UserRole + 14,
        FileBaseNameOfRenameRole = Qt::UserRole + 15,
        FileSuffixOfRenameRole = Qt::UserRole + 16,
        FileSizeInKiloByteRole = Qt::UserRole + 17,
        FileLastModifiedDateTimeRole = Qt::UserRole + 18,
        FileUserRole = Qt::UserRole + 99,
        UnknowRole = Qt::UserRole + 999
    };

    Q_ENUM(Roles)

    enum State {
        Idle,
        Busy,
        Unknow
    };

    explicit DFileSystemModel(DFileViewHelper *parent);
    ~DFileSystemModel();

    DFileViewHelper *parent() const;

    inline QModelIndex createIndex(int arow, int acolumn, quintptr aid) const
    {
        return QAbstractItemModel::createIndex(arow, acolumn, aid);
    }
    QModelIndex index(const DUrl &fileUrl, int column = 0);
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    inline int columnWidth(int column) const
    {
        return columnWidthByRole(columnToRole(column));
    }
    QVariant columnNameByRole(int role, const QModelIndex &index = QModelIndex()) const;

    int columnWidthByRole(int role) const;

    bool columnDefaultVisibleForRole(int role, const QModelIndex &index = QModelIndex()) const;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

    static QString roleName(int role);

    int columnToRole(int column) const;
    int roleToColumn(int role) const;

    bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;

    QStringList mimeTypes() const Q_DECL_OVERRIDE;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) Q_DECL_OVERRIDE;
    QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;

    QModelIndex setRootUrl(const DUrl &fileUrl);
    DUrl rootUrl() const;
    DUrlList sortedUrls();

    DUrl getUrlByIndex(const QModelIndex &index) const;

    void setSortColumn(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void setSortRole(int role, Qt::SortOrder order = Qt::AscendingOrder);
//    void setActiveIndex(const QModelIndex &index);
    void setNameFilters(const QStringList &nameFilters);
    void setFilters(QDir::Filters filters);
    void setAdvanceSearchFilter(const QMap<int, QVariant> &formData, bool turnOn);
    void applyAdvanceSearchFilter();
    std::shared_ptr<FileFilter> advanceSearchFilter();

    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder &order);
    int sortColumn() const;
    int sortRole() const;
    QStringList nameFilters() const;
    QDir::Filters filters() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) Q_DECL_OVERRIDE;
    bool sort();
    bool sort(bool emitDataChange);

    const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const;
    const DAbstractFileInfoPointer fileInfo(const DUrl &fileUrl) const;
    const DAbstractFileInfoPointer parentFileInfo(const QModelIndex &index) const;
    const DAbstractFileInfoPointer parentFileInfo(const DUrl &fileUrl) const;

    State state() const;

    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    DAbstractFileWatcher *fileWatcher() const;

    bool enabledSort() const;

    bool setColumnCompact(bool compact);
    bool columnIsCompact() const;

    void setColumnActiveRole(int column, int role);
    int columnActiveRole(int column) const;

public slots:
    void updateChildren(QList<DAbstractFileInfoPointer> list);
    void updateChildrenOnNewThread(QList<DAbstractFileInfoPointer> list);
    /// warning: only refresh current url
    void refresh(const DUrl &fileUrl = DUrl());
    void update();
    void toggleHiddenFiles(const DUrl &fileUrl);

    void setEnabledSort(bool enabledSort);

signals:
    void rootUrlDeleted(const DUrl &rootUrl);
    void stateChanged(State state);
    void enabledSortChanged(bool enabledSort);
    void newFileByInternal(const DUrl &url);
    void requestSelectFiles(const QList<DUrl> &urls);

protected:
    bool remove(const DUrl &url);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    inline const FileSystemNodePointer getNodeByIndex(const QModelIndex &index) const;
    QModelIndex createIndex(const FileSystemNodePointer &node, int column) const;
    using QAbstractItemModel::createIndex;

    bool isDir(const FileSystemNodePointer &node) const;

    bool sort(const DAbstractFileInfoPointer &parentInfo, QList<FileSystemNode *> &list) const;

    const FileSystemNodePointer createNode(FileSystemNode *parent, const DAbstractFileInfoPointer &info, QReadWriteLock *lock = nullptr);

    void deleteNode(const FileSystemNodePointer &node);
    void clear();

    void setState(State state);
    void onJobAddChildren(const DAbstractFileInfoPointer &fileInfo);
    void onJobFinished();
    void addFile(const DAbstractFileInfoPointer &fileInfo);

    void emitAllDataChanged();
    void selectAndRenameFile(const DUrl &fileUrl);

    bool beginRemoveRows(const QModelIndex &parent, int first, int last);
    void endRemoveRows();

    friend class FileSystemNode;
    friend class DFileView;
    friend class FileNodeManagerThread;

    QScopedPointer<DFileSystemModelPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_onFileCreated(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileDeleted(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileUpdated(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileUpdated(const DUrl &fileUrl, const int isExternalSource))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileRename(const DUrl &from, const DUrl &to))
    Q_PRIVATE_SLOT(d_func(), void _q_processFileEvent())

    Q_DECLARE_PRIVATE(DFileSystemModel)
    Q_DISABLE_COPY(DFileSystemModel)
};

#endif // DFILESYSTEMMODEL_H
