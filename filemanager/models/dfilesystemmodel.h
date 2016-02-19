#ifndef DFILESYSTEMMODEL_H
#define DFILESYSTEMMODEL_H

#include "dbusinterface/fileoperations_interface.h"
#include "dbusinterface/dbustype.h"

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
    };

    explicit DFileSystemModel(QObject *parent = 0);

    QModelIndex index(const QUrl &url, int column = 0);
    QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    bool canFetchMore(const QModelIndex & parent) const Q_DECL_OVERRIDE;
    void fetchMore(const QModelIndex & parent) Q_DECL_OVERRIDE;

    QModelIndex setRootPath(const QUrl &url);
    QString rootPath() const;

    QUrl getUrlByIndex(const QModelIndex &index) const;

public slots:
    void updateChildren(const QUrl &url, const FileItemInfoList &list);
    void updateIcon(const QUrl &url, const QIcon &icon);

private:
    FileSystemNode *m_rootNode = Q_NULLPTR;
    QMap<QUrl, FileSystemNode*> m_urlToNode;
    mutable QHash<QString, QIcon> m_typeToIcon;

    inline FileSystemNode *getNodeByIndex(const QModelIndex &index) const;
    QModelIndex createIndex(const FileSystemNode *node) const;
    using QAbstractItemModel::createIndex;

    friend class FileSystemNode;
};

#endif // DFILESYSTEMMODEL_H
