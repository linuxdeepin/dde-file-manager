#ifndef DFILESYSTEMMODEL_H
#define DFILESYSTEMMODEL_H

#include <QAbstractItemModel>
#include <QFileSystemModel>

class FileSystemNode;
class AbstractFileInfo;
class DFileView;
class FMEvent;

class DFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
        FileNameRole = Qt::UserRole + 2,
        FileSizeRole = Qt::UserRole + 3,
        FileMimeTypeRole = Qt::UserRole + 4,
        FileOwnerRole = Qt::UserRole + 5,
        FileLastModified = Qt::UserRole + 6,
        FileLastRead = Qt::UserRole + 7,
        FileCreated = Qt::UserRole + 8,
        FileDisplayNameRole = Qt::UserRole + 9
    };

    explicit DFileSystemModel(DFileView *parent = 0);
    ~DFileSystemModel();

    DFileView *parent() const;

    QModelIndex index(const QString &fileUrl, int column = 0);
    QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

    int headerDataToRole(QVariant data) const;

    bool canFetchMore(const QModelIndex & parent) const Q_DECL_OVERRIDE;
    void fetchMore(const QModelIndex & parent) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;

    QStringList mimeTypes() const Q_DECL_OVERRIDE;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) Q_DECL_OVERRIDE;
    QMimeData *mimeData(const QModelIndexList & indexes) const Q_DECL_OVERRIDE;

    QModelIndex setRootPath(const QString &fileUrl);
    QString rootPath() const;

    QString getUrlByIndex(const QModelIndex &index) const;

    void setSortColumn(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void setSortRole(int role, Qt::SortOrder order = Qt::AscendingOrder);
    void setActiveIndex(const QModelIndex &index);

    Qt::SortOrder sortOrder() const;
    int sortColumn() const;
    int sortRole() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) Q_DECL_OVERRIDE;

    AbstractFileInfo *fileInfo(const QModelIndex &index) const;
    AbstractFileInfo *fileInfo(const QString &fileUrl) const;
    AbstractFileInfo *parentFileInfo(const QModelIndex &index) const;
    AbstractFileInfo *parentFileInfo(const QString &fileUrl) const;

public slots:
    void updateChildren(const FMEvent &event, QList<AbstractFileInfo*> list);
    void refresh(const QString &fileUrl);

private slots:
    void onFileCreated(const QString &fileUrl);
    void onFileDeleted(const QString &fileUrl);

private:
    FileSystemNode *m_rootNode = Q_NULLPTR;

    QHash<QString, FileSystemNode*> m_urlToNode;

    int m_sortRole = Qt::DisplayRole;
    int m_sortColumn = 0;

    Qt::SortOrder m_srotOrder = Qt::AscendingOrder;
    QModelIndex m_activeIndex;

    bool (*sortFun)(const AbstractFileInfo*, const AbstractFileInfo*) = Q_NULLPTR;

    inline FileSystemNode *getNodeByIndex(const QModelIndex &index) const;
    QModelIndex createIndex(const FileSystemNode *node, int column) const;
    using QAbstractItemModel::createIndex;

    bool isDir(const FileSystemNode *node) const;

    void sort(QList<AbstractFileInfo*> &list) const;

    FileSystemNode *createNode(FileSystemNode *parent, AbstractFileInfo *info);

    void deleteNode(FileSystemNode *node);
    void deleteNodeByUrl(const QString &url);

    friend class FileSystemNode;
};

#endif // DFILESYSTEMMODEL_H
