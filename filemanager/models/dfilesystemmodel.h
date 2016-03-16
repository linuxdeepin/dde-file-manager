#ifndef DFILESYSTEMMODEL_H
#define DFILESYSTEMMODEL_H

#include "fileinfo.h"

#include <QAbstractItemModel>
#include <QFileSystemModel>

class FileSystemNode;

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
        FileCreated = Qt::UserRole + 8
    };

    explicit DFileSystemModel(QObject *parent = 0);

    QModelIndex index(const QString &url, int column = 0);
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

    QModelIndex setRootPath(const QString &url);
    QString rootPath() const;

    QString getUrlByIndex(const QModelIndex &index) const;

public slots:
    void updateChildren(const QString &url, const FileInfoList &list);
    void updateIcon(const QString &url, const QIcon &icon);
    void refresh(const QString &url);

private:
    FileSystemNode *m_rootNode = Q_NULLPTR;
    QMap<QString, FileSystemNode*> m_urlToNode;
    mutable QHash<QString, QIcon> m_typeToIcon;

    inline FileSystemNode *getNodeByIndex(const QModelIndex &index) const;
    QModelIndex createIndex(const FileSystemNode *node, int column) const;
    using QAbstractItemModel::createIndex;

    bool isDir(const FileSystemNode *node) const;

    friend class FileSystemNode;
};

#endif // DFILESYSTEMMODEL_H
