#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "dbusinterface/dbustype.h"
#include "dbusinterface/listjob_interface.h"
#include "dbusinterface/fileInfo_interface.h"

#include <QDebug>
#include <QDBusPendingCallWatcher>
#include <QFileIconProvider>

class FileSystemNode
{
public:
    FileItemInfo fileInfo;
    FileSystemNode *parent = Q_NULLPTR;
    QHash<QString, FileSystemNode*> children;
    QList<QString> visibleChildren;
    bool populatedChildren = false;

    FileSystemNode(DFileSystemModel *model, FileSystemNode *parent,
                   const QString &path, bool initChildren = false) :
        parent(parent),
        populatedChildren(initChildren),
        m_model(model)
    {
        model->m_pathToNode[path] = this;

        if(initChildren)
            initListJob(path);
    }

    ~FileSystemNode()
    {
        m_model->m_pathToNode.remove(fileInfo.URI);
        qDeleteAll(children.values());
    }

    void initListJob(const QString &path)
    {
        FileOperationsInterface *dbusInterface = dbusController->getFileOperationsInterface();

        ASYN_CALL(dbusInterface->NewListJob(path, 0), {
                      if(watcher->isError()) {
                          qDebug() << watcher->error().message();
                          return;
                      }

                      getChildren(args[0].toString(),
                                  qvariant_cast<QDBusObjectPath>(args[1]),
                                  args[2].toString());
                  }, this);
    }

    void getChildren(const QString &service, const QDBusObjectPath &path, const QString &interface)
    {
        ListJobInterface *listJob = new ListJobInterface(service, path.path(), interface);

        ASYN_CALL(listJob->Execute(), {
                      FileItemInfoList fileInfoList = (QDBusPendingReply<FileItemInfoList>(*watcher)).value();
                      children.reserve(fileInfoList.count());
                      visibleChildren.reserve(fileInfoList.count());

                      m_model->beginInsertRows(m_model->createIndex(this), 0, fileInfoList.count() - 1);

                      for(FileItemInfo &fileInfo : fileInfoList) {
                          FileSystemNode *chileNode = new FileSystemNode(m_model, this, fileInfo.URI);

                          chileNode->fileInfo = std::move(fileInfo);
                          children[fileInfo.BaseName] = chileNode;
                          visibleChildren << fileInfo.BaseName;
                      }

                      m_model->endInsertRows();
                  }, this, listJob);
    }

    void clearChildren()
    {
        visibleChildren.clear();
        qDeleteAll(children.values());
    }

    void refreshChildren()
    {
        clearChildren();
        initListJob(fileInfo.URI);
    }

private:
    DFileSystemModel *m_model;
};

DFileSystemModel::DFileSystemModel(QObject *parent) :
    QAbstractItemModel(parent)
{

}

QModelIndex DFileSystemModel::index(const QUrl &url, int /*column*/)
{
    FileSystemNode *node = m_pathToNode.value(url.toString());

    if (!node)
        return QModelIndex();

    QModelIndex idx = createIndex(node);

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

    return createIndex(indexNode->parent);
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

                QFileIconProvider tmp_icon;

QVariant DFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    FileSystemNode *indexNode = getNodeByIndex(index);

    Q_ASSERT(indexNode);

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return indexNode->fileInfo.DisplayName;
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
        break;
    case FileNameRole:
        return indexNode->fileInfo.BaseName;
        break;
    case FileIconRole:
        if (index.column() == 0) {
            QIcon icon = m_typeToIcon.value(indexNode->fileInfo.MIME);

            if(icon.isNull()) {
                QString iconPath = dbusController->getFileInfoInterface()->GetThemeIcon(indexNode->fileInfo.URI, 30);

                if(iconPath.isEmpty()) {
                    const QFileInfo &fileInfo = QFileInfo(QUrl(indexNode->fileInfo.URI).toLocalFile());
                    QFileIconProvider prrovider;
                    icon = prrovider.icon(fileInfo);
                } else {
                    icon = QIcon(iconPath);
                }

                m_typeToIcon[indexNode->fileInfo.MIME] = icon;
            }

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

void DFileSystemModel::fetchMore(const QModelIndex &parent)
{
    if(!m_rootNode)
        return;

    FileSystemNode *parentNode = getNodeByIndex(parent);

    if(!parentNode || parentNode->populatedChildren)
        return;

    parentNode->populatedChildren = true;
    parentNode->refreshChildren();
}

bool DFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    FileSystemNode *parentNode = getNodeByIndex(parent);

    return parentNode && !parentNode->populatedChildren;
}

QModelIndex DFileSystemModel::setRootPath(const QUrl &url)
{
    if(url.isLocalFile()) {
        QDir dir(url.toLocalFile());

        if(!dir.exists())
            return QModelIndex();
    }

    if(!m_rootNode)
        delete m_rootNode;

    QString path = url.toString();

    m_rootNode = new FileSystemNode(this, Q_NULLPTR, path, true);
    m_rootNode->fileInfo.URI = path;
    m_rootNode->fileInfo.BaseName = url.isLocalFile() ? url.toLocalFile() : path;
    m_rootNode->fileInfo.DisplayName = m_rootNode->fileInfo.BaseName;

    return index(path);
}

QString DFileSystemModel::rootPath() const
{
    return m_rootNode ? QUrl(m_rootNode->fileInfo.URI).toLocalFile() : "";
}

FileSystemNode *DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_rootNode;

    FileSystemNode *indexNode = static_cast<FileSystemNode*>(index.internalPointer());
    Q_ASSERT(indexNode);

    return indexNode;
}

QModelIndex DFileSystemModel::createIndex(const FileSystemNode *node) const
{
    int row = node->parent
            ? node->parent->visibleChildren.indexOf(node->fileInfo.BaseName)
            : 0;

    return createIndex(row, 0, const_cast<FileSystemNode*>(node));
}
