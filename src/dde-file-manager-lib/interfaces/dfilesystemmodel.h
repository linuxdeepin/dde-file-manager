/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include <unistd.h>

QT_BEGIN_NAMESPACE
class QEventLoop;
class QReadWriteLock;
QT_END_NAMESPACE

//#define DRAG_EVENT_URLS "UrlsInDragEvent"
#define DRAG_EVENT_URLS ((getuid()==0) ? (QString(getlogin())+"_RootUrlsInDragEvent") :(QString(getlogin())+"_UrlsInDragEvent"))
//临时解决一普通用户使用共享内存的时候，其他用户不能访问问题，增加用户名可解决多个用户同时访问共享内存的问题
//临时解决root用户使用共享内存后其它用户无法使用的问题，但可能会造成内存占用过大的问题 unistd.h在此处被使用，后面若有更换的解决办法，此处应被修改

class FileSystemNode;
class DAbstractFileInfo;
class DFMEvent;
class JobController;
class DFileViewHelper;
class DFMUrlListBaseEvent;
class QSharedMemory;

enum _asb_LabelIndex {
    SEARCH_RANGE, FILE_TYPE, SIZE_RANGE, DATE_RANGE, ACCESS_DATE_RANGE, CREATE_DATE_RANGE, LABEL_COUNT,
    TRIGGER_SEARCH = 114514
};

typedef struct fileFilter {
    QMap<int, QVariant> filterRule; // for advanced search.
    bool filterEnabled = false;
    QPair<quint64, quint64> f_sizeRange;
    QDateTime f_dateRangeStart;
    QDateTime f_dateRangeEnd;
    QDateTime f_accessDateRangeStart;
    QDateTime f_accessDateRangeEnd;
    QDateTime f_createDateRangeStart;
    QDateTime f_createDateRangeEnd;
    QString f_typeString;
    bool f_includeSubDir;
    bool f_comboValid[LABEL_COUNT];
} FileFilter;

Q_DECLARE_METATYPE(FileFilter)

static QList<QUrl> FOR_DRAGEVENT;

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
        FileIconModelToolTipRole = Qt::UserRole + 19, // 用于返回图标视图下的tooltip
        FileLastReadDateTimeRole = Qt::UserRole + 20,   //文件最近访问时间
        FileCreatedDateTimeRole = Qt::UserRole + 21,    //文件创建时间
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
    ~DFileSystemModel() override;

    DFileViewHelper *parent() const;

    QModelIndex index(const DUrl &fileUrl, int column = 0);
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    inline int columnWidth(int column) const
    {
        return columnWidthByRole(columnToRole(column));
    }
    QVariant columnNameByRole(int role, const QModelIndex &index = QModelIndex()) const;

    int columnWidthByRole(int role) const;

    bool columnDefaultVisibleForRole(int role, const QModelIndex &index = QModelIndex()) const;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;

    static QString roleName(int role);

    int columnToRole(int column) const;
    int roleToColumn(int role) const;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    QModelIndex setRootUrl(const DUrl &fileUrl);
    DUrl rootUrl() const;
    DUrlList sortedUrls();
    DUrlList getNoTransparentUrls();

    DUrl getUrlByIndex(const QModelIndex &index) const;

    void setSortColumn(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void setSortRole(int role, Qt::SortOrder order = Qt::AscendingOrder);
    void setNameFilters(const QStringList &nameFilters);
    void setFilters(QDir::Filters filters);
    void setAdvanceSearchFilter(const QMap<int, QVariant> &formData, bool turnOn, bool updateView);
    void applyAdvanceSearchFilter();
    std::shared_ptr<FileFilter> advanceSearchFilter();

    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder &order);
    int sortColumn() const;
    int sortRole() const;
    QStringList nameFilters() const;
    QDir::Filters filters() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
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

//    static QList<QUrl> m_urlForDragEvent;

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
    void sigJobFinished();
    void requestRedirect(const DUrl &rootUrl, const DUrl &newUrl);
    void showFilterButton();
    // fix bug 63938
    // 切换标签时，更新高级选项中的过滤项
    void updateFilterRule(const FileFilter *filter);

protected:
    bool remove(const DUrl &url);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    inline const FileSystemNodePointer getNodeByIndex(const QModelIndex &index) const;
    QModelIndex createIndex(const FileSystemNodePointer &node, int column) const;
    using QAbstractItemModel::createIndex;

    bool isDir(const FileSystemNodePointer &node) const;

    bool sort(const DAbstractFileInfoPointer &parentInfo, QList<FileSystemNodePointer> &list) const;
    bool doSortBusiness(bool emitDataChange);

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
    //fix bug释放jobcontroller
    bool releaseJobController();
    QDir::Filters m_filters; //仅记录非回收站文件过滤规则
    bool isFirstRun = true; //判断是否首次运行
    bool isNeedToBreakBusyCase = false; // 停止那些忙的流程 // bug 26972, 27384
    QMutex   m_mutex; // 对当前文件资源进行单操作 // bug 26972

private:

    QMap<QString, FileSystemNodePointer> m_allFileSystemNodes;

    friend class FileSystemNode;
    friend class DFileView;
    friend class FileNodeManagerThread;
    friend class DListItemDelegate;

    QScopedPointer<DFileSystemModelPrivate> d_ptr;
    QSharedMemory *m_smForDragEvent;

    Q_PRIVATE_SLOT(d_func(), void _q_onFileCreated(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileDeleted(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileUpdated(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileUpdated(const DUrl &fileUrl, const int isExternalSource))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileRename(const DUrl &from, const DUrl &to))
    Q_PRIVATE_SLOT(d_func(), void _q_processFileEvent())

    Q_DECLARE_PRIVATE(DFileSystemModel)
    Q_DISABLE_COPY(DFileSystemModel)
public:
    bool ignoreDropFlag = false; //candrop十分耗时,在不关心Qt::ItemDropEnable的调用时设置其为true，
    //不调用candrop，节省时间,bug#10926
    bool isDesktop = false; //紧急修复，由于修复bug#33209添加了一次事件循环的处理，导致桌面的自动排列在删除，恢复文件时显示异常
};

#endif // DFILESYSTEMMODEL_H
