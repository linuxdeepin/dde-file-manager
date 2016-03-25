#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "desktopfileinfo.h"
#include "../app/filemanagerapp.h"
#include "../controllers/appcontroller.h"
#include "filemonitor/filemonitor.h"

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

    DFileSystemModel *m_model;
};

DFileSystemModel::DFileSystemModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    connect(fileMonitor, &FileMonitor::fileCreated,
            this, &DFileSystemModel::onFileCreated);

    connect(fileMonitor, &FileMonitor::fileDeleted,
            this, &DFileSystemModel::onFileDeleted);

    connect(fileMonitor, &FileMonitor::fileRenamed,
            this, &DFileSystemModel::onFileRenamed);
}

DFileSystemModel::~DFileSystemModel()
{
    for(FileSystemNode *node : m_urlToNode) {
        fileMonitor->removeMonitorPath(node->fileInfo->absoluteFilePath());

        delete node;
    }
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

    fileMonitor->addMonitorPath(parentNode->fileInfo->absoluteFilePath());

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
            success = QFile::copy(path, to + QFileInfo(path).fileName());
        }
        break;
    case Qt::LinkAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::link(path, to + QFileInfo(path).fileName());
        }
        break;
    case Qt::MoveAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::rename(path, to + QFileInfo(path).fileName());
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
    if(m_rootNode) {
        const QString m_rootPath = m_rootNode->fileInfo->absoluteFilePath();

        if(urlStr == m_rootPath)
            return createIndex(m_rootNode, 0);

        fileMonitor->removeMonitorPath(m_rootPath);

        m_rootNode->populatedChildren = false;
        m_rootNode->visibleChildren.clear();
    }

    m_rootNode = m_urlToNode.value(urlStr);

    if(!m_rootNode)
        m_rootNode = new FileSystemNode(this, Q_NULLPTR, new FileInfo(urlStr));

    return index(urlStr);
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

void DFileSystemModel::setSortColumn(int column, Qt::SortOrder order)
{
    m_sortColumn = column;

    switch (m_sortColumn) {
    case 0:
        setSortRole(DFileSystemModel::FileNameRole, order);
        break;
    case 1:
        setSortRole(DFileSystemModel::FileLastModified, order);
        break;
    case 2:
        setSortRole(DFileSystemModel::FileSizeRole, order);
        break;
    case 3:
        setSortRole(DFileSystemModel::FileMimeTypeRole, order);
        break;
    case 4:
        setSortRole(DFileSystemModel::FileCreated, order);
        break;
    default:
        break;
    }
}

void DFileSystemModel::setSortRole(int role, Qt::SortOrder order)
{
    m_sortRole = role;
    m_srotOrder = order;
}

void DFileSystemModel::setActiveIndex(const QModelIndex &index)
{
    m_activeIndex = index;

    FileSystemNode *node = getNodeByIndex(index);

    if(!node || node->populatedChildren)
        return;

    node->visibleChildren.clear();
}

Qt::SortOrder DFileSystemModel::sortOrder() const
{
    return m_srotOrder;
}

int DFileSystemModel::sortColumn() const
{
    return m_sortColumn;
}

int DFileSystemModel::sortRole() const
{
    return m_sortRole;
}

void DFileSystemModel::sort(int column, Qt::SortOrder order)
{
    int old_sortRole = m_sortRole;
    int old_sortOrder = m_srotOrder;

    setSortColumn(column, order);

    if(old_sortRole == m_sortRole && old_sortOrder == m_srotOrder) {
        return;
    }

    FileSystemNode *node = getNodeByIndex(m_activeIndex);

    if(!node)
        return;

    const QString &node_absoluteFilePath = node->fileInfo->absoluteFilePath();

    for(FileSystemNode *url_node : m_urlToNode) {
        if(node == url_node)
            continue;

        url_node->populatedChildren = false;

        if(node_absoluteFilePath.startsWith(url_node->fileInfo->absoluteFilePath()))
            continue;

        url_node->visibleChildren.clear();
    }

    QList<FileInfo*> list;

    list.reserve(node->visibleChildren.size());

    for(const QString &fileName : node->visibleChildren) {
        list << node->children.value(fileName)->fileInfo;
    }

    sort(list);

    for(int i = 0; i < node->visibleChildren.count(); ++i) {
        node->visibleChildren[i] = list[i]->fileName();
    }

    QModelIndex parentIndex = createIndex(node, 0);
    QModelIndex topLeftIndex = index(0, 0, parentIndex);
    QModelIndex rightBottomIndex = index(node->visibleChildren.count(), columnCount(parentIndex), parentIndex);

    emit dataChanged(topLeftIndex, rightBottomIndex);
}

void DFileSystemModel::updateChildren(const QString &url, QList<FileInfo*> list)
{
    FileSystemNode *node = getNodeByIndex(index(url));

    if(!node)
        return;

    node->children.clear();
    node->visibleChildren.clear();

    sort(list);

    beginInsertRows(createIndex(node, 0), 0, list.count() - 1);

    for(FileInfo * const fileInfo : list) {
        FileSystemNode *chileNode = m_urlToNode.value(fileInfo->absoluteFilePath());

        if(chileNode) {
            if(chileNode->fileInfo)
                delete chileNode->fileInfo;

            chileNode->fileInfo = fileInfo;
            chileNode->parent = node;
        } else {
            chileNode = new FileSystemNode(this, node, fileInfo);
        }

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

void DFileSystemModel::onFileCreated(const QString &path)
{
    qDebug() << "file creatored" << path;

    QFileInfo info(path);

    FileSystemNode *parentNode = m_urlToNode.value(info.absolutePath());

    if(parentNode && parentNode->populatedChildren && !parentNode->visibleChildren.contains(info.fileName())) {
        beginInsertRows(createIndex(parentNode, 0), 0, 0);

        FileSystemNode *node = m_urlToNode.value(path);

        if(!node) {
            node = new FileSystemNode(this, parentNode, new FileInfo(info));

            m_urlToNode[path] = node;
        }

        parentNode->children[info.fileName()] = node;
        parentNode->visibleChildren.insert(0, info.fileName());

        endInsertRows();
    }
}

void DFileSystemModel::onFileDeleted(const QString &path)
{
    qDebug() << "file deleted:" << path;

    fileMonitor->removeMonitorPath(path);

    QFileInfo info(path);

    FileSystemNode *parentNode = m_urlToNode.value(info.absolutePath());

    if(parentNode && parentNode->populatedChildren) {
        int index = parentNode->visibleChildren.indexOf(info.fileName());

        beginRemoveRows(createIndex(parentNode, 0), index, index);

        parentNode->visibleChildren.removeAt(index);
        parentNode->children.remove(info.fileName());

        endRemoveRows();

        FileSystemNode *node = m_urlToNode.value(path);

        if(!node)
            return;

        if(hasChildren(createIndex(node, 0))) {
            for(const QString &url : m_urlToNode.keys()) {
                if(path.startsWith(url)) {
                    delete m_urlToNode[url];

                    m_urlToNode.remove(url);
                }
            }
        }

        delete node;

        m_urlToNode.remove(path);
    }
}

void DFileSystemModel::onFileRenamed(const QString &oldPath, const QString &newPath)
{
    qDebug() << "file renamed, old path:" << oldPath << "new path:" << newPath;

    onFileDeleted(oldPath);
    onFileCreated(newPath);
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

Qt::SortOrder sortOrder_global;

bool sortFileListByName(const FileInfo *info1, const FileInfo *info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    return (sortOrder_global == Qt::DescendingOrder) ^ (info1->fileName().toLower() < info2->fileName().toLower());
}

bool sortFileListBySize(const FileInfo *info1, const FileInfo *info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    return (sortOrder_global == Qt::DescendingOrder) ^ (info1->size() < info2->size());
}

bool sortFileListByModified(const FileInfo *info1, const FileInfo *info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    return (sortOrder_global == Qt::DescendingOrder) ^ (info1->lastModified() < info2->lastModified());
}

bool sortFileListByMime(const FileInfo *info1, const FileInfo *info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    return (sortOrder_global == Qt::DescendingOrder) ^ (info1->mimeTypeName() < info2->mimeTypeName());
}

bool sortFileListByCreated(const FileInfo *info1, const FileInfo *info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    return (sortOrder_global == Qt::DescendingOrder) ^ (info1->created() < info2->created());
}

void DFileSystemModel::sort(QList<FileInfo *> &list) const
{
    sortOrder_global = m_srotOrder;

    switch (m_sortRole) {
    case DFileSystemModel::FileNameRole:
    case Qt::DisplayRole:
        qSort(list.begin(), list.end(), sortFileListByName);
        break;
    case DFileSystemModel::FileLastModified:
        qSort(list.begin(), list.end(), sortFileListByModified);
        break;
    case DFileSystemModel::FileSizeRole:
        qSort(list.begin(), list.end(), sortFileListBySize);
        break;
    case DFileSystemModel::FileMimeTypeRole:
        qSort(list.begin(), list.end(), sortFileListByMime);
        break;
    case DFileSystemModel::FileCreated:
        qSort(list.begin(), list.end(), sortFileListByCreated);
        break;
    default:
        break;
    }
}
