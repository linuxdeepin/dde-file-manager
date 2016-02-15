#ifndef DFILESYSTEMMODEL_H
#define DFILESYSTEMMODEL_H

#include "dbusinterface/fileoperations_interface.h"

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

    QModelIndex index(const QString &path, int column = 0);
    QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QModelIndex setRootPath(const QString &path);
    QString rootPath() const;

private:
    QDir m_rootDir;
    FileSystemNode *m_rootNode = Q_NULLPTR;
    QMap<QString, FileSystemNode*> m_pathToNode;

    inline FileSystemNode *getNodeByIndex(const QModelIndex &index) const
    {return static_cast<FileSystemNode*>(index.internalPointer());}

    friend class FileSystemNode;
};

#endif // DFILESYSTEMMODEL_H
