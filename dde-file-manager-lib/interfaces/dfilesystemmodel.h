#ifndef DFILESYSTEMMODEL_H
#define DFILESYSTEMMODEL_H

#include <QAbstractItemModel>
#include <QPointer>
#include <QDir>
#include <QFuture>

#include "durl.h"
#include "dabstractfileinfo.h"

QT_BEGIN_NAMESPACE
class QEventLoop;
QT_END_NAMESPACE

class FileSystemNode;
class DAbstractFileInfo;
class DFMEvent;
class JobController;
class DFileViewHelper;

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
        FileSizeRole = Qt::UserRole + 3,
        FileMimeTypeRole = Qt::UserRole + 4,
        FileOwnerRole = Qt::UserRole + 5,
        FileLastModifiedRole = Qt::UserRole + 6,
        FileLastReadRole = Qt::UserRole + 7,
        FileCreatedRole = Qt::UserRole + 8,
        FileDisplayNameRole = Qt::UserRole + 9,
        FilePinyinName = Qt::UserRole + 10,
        FileUserRole = Qt::UserRole + 11,
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
    { return QAbstractItemModel::createIndex(arow, acolumn, aid);}
    QModelIndex index(const DUrl &fileUrl, int column = 0);
    QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    inline int columnWidth(int column) const
    { return columnWidthByRole(columnToRole(column));}
    QVariant columnNameByRole(int role, const QModelIndex &index = QModelIndex()) const;

    int columnWidthByRole(int role) const;

    bool columnDefaultVisibleForRole(int role, const QModelIndex &index = QModelIndex()) const;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

    static QString roleName(int role);

    int columnToRole(int column) const;
    int roleToColumn(int role) const;

    bool canFetchMore(const QModelIndex & parent) const Q_DECL_OVERRIDE;
    void fetchMore(const QModelIndex & parent) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;

    QStringList mimeTypes() const Q_DECL_OVERRIDE;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) Q_DECL_OVERRIDE;
    QMimeData *mimeData(const QModelIndexList & indexes) const Q_DECL_OVERRIDE;

    QModelIndex setRootUrl(const DUrl &fileUrl);
    DUrl rootUrl() const;

    DUrl getUrlByIndex(const QModelIndex &index) const;

    void setSortColumn(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void setSortRole(int role, Qt::SortOrder order = Qt::AscendingOrder);
//    void setActiveIndex(const QModelIndex &index);
    void setNameFilters(const QStringList &nameFilters);
    void setFilters(QDir::Filters filters);

    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder& order);
    int sortColumn() const;
    int sortRole() const;
    QStringList nameFilters() const;
    QDir::Filters filters() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) Q_DECL_OVERRIDE;
    void sort();

    const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const;
    const DAbstractFileInfoPointer fileInfo(const DUrl &fileUrl) const;
    const DAbstractFileInfoPointer parentFileInfo(const QModelIndex &index) const;
    const DAbstractFileInfoPointer parentFileInfo(const DUrl &fileUrl) const;

    State state() const;

    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    DAbstractFileWatcher *fileWatcher() const;

    bool enabledSort() const;

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

private:
    inline const FileSystemNodePointer getNodeByIndex(const QModelIndex &index) const;
    QModelIndex createIndex(const FileSystemNodePointer &node, int column) const;
    using QAbstractItemModel::createIndex;

    bool isDir(const FileSystemNodePointer &node) const;

    void sort(const DAbstractFileInfoPointer &parentInfo, QList<DAbstractFileInfoPointer> &list) const;

    const FileSystemNodePointer createNode(FileSystemNode *parent, const DAbstractFileInfoPointer &info);

    void deleteNode(const FileSystemNodePointer &node);
    void clear();

    void setState(State state);
    void onJobAddChildren(const DAbstractFileInfoPointer &fileInfo);
    void onJobFinished();
    void addFile(const DAbstractFileInfoPointer &fileInfo);

    void emitAllDataChanged();
    void selectAndRenameFile(const DUrl &fileUrl);

    friend class FileSystemNode;

    QScopedPointer<DFileSystemModelPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_onFileCreated(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileDeleted(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileUpdated(const DUrl &fileUrl))
    Q_PRIVATE_SLOT(d_func(), void _q_onFileRename(const DUrl &from, const DUrl &to))
    Q_PRIVATE_SLOT(d_func(), void _q_processFileEvent())

    Q_DECLARE_PRIVATE(DFileSystemModel)
    Q_DISABLE_COPY(DFileSystemModel)
};

#endif // DFILESYSTEMMODEL_H
