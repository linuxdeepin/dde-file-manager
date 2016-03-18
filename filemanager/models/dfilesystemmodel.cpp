#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "desktopfileinfo.h"

#include <QDebug>
#include <QFileIconProvider>

class FileSystemNode
{
public:
    FileInfo *fileInfo;
    FileSystemNode *parent = Q_NULLPTR;
    QHash<QString, FileSystemNode*> children;
    QList<QString> visibleChildren;
    bool populatedChildren = false;

    FileSystemNode(DFileSystemModel *model, FileSystemNode *parent,
                   FileInfo *info) :
        fileInfo(info),
        parent(parent),
        m_model(model)
    {
        model->m_urlToNode[fileInfo->absoluteFilePath()] = this;
    }

    ~FileSystemNode()
    {
        m_model->m_urlToNode.remove(fileInfo->absoluteFilePath());
        qDeleteAll(children.values());

        delete fileInfo;
    }

    void clearChildren()
    {
        visibleChildren.clear();
        qDeleteAll(children.values());
        children.clear();
    }

    DFileSystemModel *m_model;
};

DFileSystemModel::DFileSystemModel(QObject *parent) :
    QAbstractItemModel(parent)
{

}

QModelIndex DFileSystemModel::index(const QString &url, int column)
{
    FileSystemNode *node = m_urlToNode.value(url);

    if (!node)
        return QModelIndex();

    QModelIndex idx = createIndex(node, column);

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

    return createIndex(indexNode->parent, 0);
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

int DFileSystemModel::columnCount(const QModelIndex &parent) const
{
    return parent.column() > 0 ? 0 : 5;
}

bool DFileSystemModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) // drives
        return false;

    const FileSystemNode *indexNode = getNodeByIndex(parent);
    Q_ASSERT(indexNode);

    return isDir(indexNode);
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
        switch (index.column()) {
        case 0: return indexNode->fileInfo->fileName();
        case 1: return indexNode->fileInfo->lastModified().toString();
        case 2: return indexNode->fileInfo->size();
        case 3: return indexNode->fileInfo->mimeTypeName();
        case 4: return indexNode->fileInfo->created().toString();
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case FilePathRole:
        return indexNode->fileInfo->absoluteFilePath();
        break;
    case FileNameRole:
        return indexNode->fileInfo->fileName();
        break;
    case FileIconRole:
        if (index.column() == 0) {
            if(indexNode->fileInfo->isDesktopFile()) {
                DesktopFileInfo *info = dynamic_cast<DesktopFileInfo*>(indexNode->fileInfo);

                if(info)
                    return m_iconProvider.getDesktopIcon(info->getIcon(), 256);
            }

            return m_iconProvider.getFileIcon(indexNode->fileInfo->absoluteFilePath());
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter;
    case FileLastModified:
        return indexNode->fileInfo->lastModified().toString();
    case FileSizeRole:
        return indexNode->fileInfo->size();
    case FileMimeTypeRole:
        return indexNode->fileInfo->mimeTypeName();
    case FileCreated:
        return indexNode->fileInfo->created().toString();
    }

    return QVariant();
}

QVariant DFileSystemModel::headerData(int section, Qt::Orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        switch(section)
        {
        case 0: return tr("Name");
        case 1: return tr("Date Modified");
        case 2: return tr("Size");
        case 3: return tr("Type");
        case 4: return tr("Date Created");
        default: return QVariant();
        }
    } else if(role == Qt::BackgroundRole) {
        return QBrush(Qt::white);
    } else if(role == Qt::ForegroundRole) {
        return QBrush(Qt::black);
    }

    return QVariant();
}

int DFileSystemModel::headerDataToRole(QVariant data) const
{
    if(!data.isValid())
        return -1;

    if(data == tr("Name")) {
        return FileNameRole;
    } else if(data == tr("Date Modified")) {
        return FileLastModified;
    } else if(data == tr("Size")) {
        return FileSizeRole;
    } else if(data == tr("Type")) {
        return FileMimeTypeRole;
    } else if(data == tr("Date Created")) {
        return FileCreated;
    }

    return -1;
}

void DFileSystemModel::fetchMore(const QModelIndex &parent)
{
    if(!m_rootNode)
        return;

    FileSystemNode *parentNode = getNodeByIndex(parent);

    if(!parentNode || parentNode->populatedChildren)
        return;

    parentNode->populatedChildren = true;

    emit fileSignalManager->requestChildren(parentNode->fileInfo->absoluteFilePath());
}

Qt::ItemFlags DFileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    FileSystemNode *indexNode = getNodeByIndex(index);

    flags |= Qt::ItemIsDragEnabled;

    if(indexNode->fileInfo->isCanRename())
        flags |= Qt::ItemIsEditable;

    if ((index.column() == 0) && indexNode->fileInfo->isWritable()) {
        if (isDir(indexNode))
            flags |= Qt::ItemIsDropEnabled;
        else
            flags |= Qt::ItemNeverHasChildren;
    }

    return flags;
}

Qt::DropActions DFileSystemModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions DFileSystemModel::supportedDropActions() const
{
    return supportedDragActions();
}

QStringList DFileSystemModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

bool DFileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug() << "drop mime data";

    Q_UNUSED(row);
    Q_UNUSED(column);
    if (!parent.isValid())
        return false;

    bool success = true;
    QString to = QUrl(getUrlByIndex(parent)).toLocalFile() + QDir::separator();

    QList<QUrl> urls = data->urls();
    QList<QUrl>::const_iterator it = urls.constBegin();

    switch (action) {
    case Qt::CopyAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::LinkAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::link(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::MoveAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::rename(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    default:
        return false;
    }

    return success;
}

QMimeData *DFileSystemModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QList<QModelIndex>::const_iterator it = indexes.begin();
    for (; it != indexes.end(); ++it)
        if ((*it).column() == 0)
            urls << QUrl(getUrlByIndex(*it));
    QMimeData *data = new QMimeData();
    data->setUrls(urls);

    return data;
}

bool DFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    FileSystemNode *parentNode = getNodeByIndex(parent);

    if(!parentNode)
        return false;

    return isDir(parentNode) && !parentNode->populatedChildren;
}

QModelIndex DFileSystemModel::setRootPath(const QString &urlStr)
{
    if(!m_rootNode)
        delete m_rootNode;

    m_rootNode = new FileSystemNode(this, Q_NULLPTR, new FileInfo(urlStr));

    return index(urlStr);

    return QModelIndex();
}

QString DFileSystemModel::rootPath() const
{
    return m_rootNode ? m_rootNode->fileInfo->absolutePath() : "";
}

QString DFileSystemModel::getUrlByIndex(const QModelIndex &index) const
{
    FileSystemNode *node = getNodeByIndex(index);

    if(!node)
        return "";

    return node->fileInfo->absoluteFilePath();
}

void DFileSystemModel::updateChildren(const QString &url, const QList<FileInfo*> &list)
{
    FileSystemNode *node = getNodeByIndex(index(url));

    if(!node)
        return;

    node->clearChildren();

    beginInsertRows(createIndex(node, 0), 0, list.count() - 1);

    for(FileInfo * const fileInfo : list) {
        FileSystemNode *chileNode = new FileSystemNode(this, node, fileInfo);

        node->children[fileInfo->fileName()] = chileNode;
        node->visibleChildren << fileInfo->fileName();
    }

    endInsertRows();
}

void DFileSystemModel::refresh(const QString &url)
{
    FileSystemNode *node = m_urlToNode.value(url);

    if(!node)
        return;

    if(!isDir(node))
        return;

    node->populatedChildren = true;

    emit fileSignalManager->requestChildren(url);
}

FileSystemNode *DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_rootNode;

    FileSystemNode *indexNode = static_cast<FileSystemNode*>(index.internalPointer());
    Q_ASSERT(indexNode);

    return indexNode;
}

QModelIndex DFileSystemModel::createIndex(const FileSystemNode *node, int column) const
{
    int row = node->parent
            ? node->parent->visibleChildren.indexOf(node->fileInfo->fileName())
            : 0;

    return createIndex(row, column, const_cast<FileSystemNode*>(node));
}

bool DFileSystemModel::isDir(const FileSystemNode *node) const
{
    return node->fileInfo->isDir();
}
