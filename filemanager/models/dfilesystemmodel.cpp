#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "dbusinterface/dbustype.h"
#include "dbusinterface/listjob_interface.h"
#include "dbusinterface/fileInfo_interface.h"

#include <QDebug>
#include <QDBusPendingCallWatcher>

class FileSystemNode
{
public:
    QString abstractPath;
    FileItemInfo fileInfo;
    FileSystemNode *parent = Q_NULLPTR;
    QHash<QString, FileSystemNode*> children;
    QList<QString> visibleChildren;

    FileSystemNode(DFileSystemModel *model, FileSystemNode *parent,
                   const QString &path, bool initChildren = false) :
        abstractPath(path),
        parent(parent),
        m_model(model)
    {
        model->m_pathToNode[path] = this;

        if(initChildren)
            initListJob(path);
    }

    ~FileSystemNode()
    {
        m_model->m_pathToNode.remove(abstractPath);
    }

    void initListJob(const QString &path)
    {
        FileOperationsInterface *dbusInterface = dbusController->getFileOperationsInterface();

        ASYN_CALL(dbusInterface->NewListJob(path, 0), {
                      getChildren(args[0].toString(),
                                  qvariant_cast<QDBusObjectPath>(args[1]),
                                  args[2].toString());
                  }, this);
    }

    void getChildren(const QString &service, const QDBusObjectPath &path, const QString &interface)
    {
        ListJobInterface *listJob = new ListJobInterface(service, path.path(), interface);

        ASYN_CALL(listJob->Execute(), {
                      QDBusPendingReply<FileItemInfoList> fileInfoList = *watcher;

                      for(FileItemInfo &fileInfo : fileInfoList.value()) {
                          FileSystemNode *chileNode = new FileSystemNode(m_model, this,
                                                                        abstractPath + fileInfo.BaseName + "/");

                          chileNode->fileInfo = std::move(fileInfo);
                          chileNode->fileInfo.Icon = dbusController->getFileInfoInterface()->GetThemeIcon(fileInfo.BaseName, 100);
                          children[fileInfo.BaseName] = chileNode;
                          visibleChildren << fileInfo.BaseName;
                      }
                  }, this, listJob);
    }

private:
    DFileSystemModel *m_model;
};

DFileSystemModel::DFileSystemModel(QObject *parent) :
    QAbstractItemModel(parent)
{

}

QModelIndex DFileSystemModel::index(const QString &path, int column)
{
    FileSystemNode *node = m_pathToNode.value(path);

    if (!node)
        return QModelIndex();

   QModelIndex idx = createIndex(0, 0, node);

    if (idx.column() != column)
        idx = idx.sibling(idx.row(), column);

    return idx;
}

QModelIndex DFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    FileSystemNode *parentNode = parent.isValid()
                                 ? getNodeByIndex(parent)
                                 : m_rootNode;

    if(!parentNode)
        return QModelIndex();

    const QString &childName = parentNode->visibleChildren.value(row);
    FileSystemNode *childNode = parentNode->children.value(childName);

    if(!childNode)
        return QModelIndex();

    return createIndex(row, column, childNode);
}

QModelIndex DFileSystemModel::parent(const QModelIndex &child) const
{
    FileSystemNode *indexNode = getNodeByIndex(child);

    if(!indexNode || !indexNode->parent)
        return QModelIndex();

    return createIndex(0, 0, indexNode->parent);
}

int DFileSystemModel::rowCount(const QModelIndex &parent) const
{
    FileSystemNode *parentNode = parent.isValid()
                                 ? getNodeByIndex(parent)
                                 : m_rootNode;

    if(!parentNode)
        return 0;

    return parentNode->visibleChildren.count();
}

int DFileSystemModel::columnCount(const QModelIndex &) const
{
    return 1;
}

bool DFileSystemModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) // drives
        return false;

    const FileSystemNode *indexNode = getNodeByIndex(parent);
    Q_ASSERT(indexNode);

    return indexNode->fileInfo.FileType == 2;
}

QVariant DFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    FileSystemNode *indexNode = getNodeByIndex(index);

    Q_ASSERT(indexNode);

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return indexNode->fileInfo.DisplayName;
        case 1: return indexNode->fileInfo.Size;
        case 2: return indexNode->fileInfo.FileType;
        case 3: return indexNode->fileInfo.MIME;
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case FilePathRole:
        return indexNode->fileInfo.URI;
    case FileNameRole:
        return indexNode->fileInfo.BaseName;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            QIcon icon = QIcon(indexNode->fileInfo.Icon);

            return icon;
        }
        break;
    case Qt::TextAlignmentRole:
        if (index.column() == 1)
            return Qt::AlignRight;
        break;
    }

    return QVariant();
}

QModelIndex DFileSystemModel::setRootPath(const QString &path)
{
    QDir dir(path);

    if(!dir.exists())
        return QModelIndex();

    m_rootDir = dir;


    if(!m_rootNode)
        delete m_rootNode;

    m_rootNode = new FileSystemNode(this, Q_NULLPTR, path, true);

    return index(path);
}

QString DFileSystemModel::rootPath() const
{
    return m_rootDir.absolutePath();
}
