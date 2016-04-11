#include "dfilesystemmodel.h"
#include "desktopfileinfo.h"

#include "../views/dfileview.h"

#include "../app/filemanagerapp.h"
#include "../app/fmevent.h"

#include "../controllers/appcontroller.h"
#include "../controllers/fileservices.h"

#include "filemonitor/filemonitor.h"

#include <QDebug>
#include <QFileIconProvider>
#include <QDateTime>
#include <QMimeData>

#define fileService FileServices::instance()

class FileSystemNode
{
public:
    AbstractFileInfo *fileInfo = Q_NULLPTR;
    FileSystemNode *parent = Q_NULLPTR;
    QHash<QString, FileSystemNode*> children;
    QList<QString> visibleChildren;
    bool populatedChildren = false;

    FileSystemNode(FileSystemNode *parent,
                   AbstractFileInfo *info) :
        fileInfo(info),
        parent(parent)
    {

    }

    ~FileSystemNode()
    {
        delete fileInfo;
    }
};

DFileSystemModel::DFileSystemModel(DFileView *parent) :
    QAbstractItemModel(parent)
{
    connect(fileService, &FileServices::childrenAdded,
            this, &DFileSystemModel::onFileCreated,
            Qt::QueuedConnection);
    connect(fileService, &FileServices::childrenRemoved,
            this, &DFileSystemModel::onFileDeleted,
            Qt::QueuedConnection);
    connect(fileService, &FileServices::childrenUpdated,
            this, &DFileSystemModel::updateChildren,
            Qt::QueuedConnection);
}

DFileSystemModel::~DFileSystemModel()
{
    for(FileSystemNode *node : m_urlToNode) {
        if(node->fileInfo->isDir())
            fileService->removeUrlMonitor(node->fileInfo->fileUrl());

        delete node;
    }
}

DFileView *DFileSystemModel::parent() const
{
    return qobject_cast<DFileView*>(QAbstractItemModel::parent());
}

QModelIndex DFileSystemModel::index(const QString &fileUrl, int column)
{
    FileSystemNode *node = m_urlToNode.value(fileUrl);

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

    const QString &childFileUrl = parentNode->visibleChildren.value(row);
    FileSystemNode *childNode = parentNode->children.value(childFileUrl);

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
        case 0: return indexNode->fileInfo->displayName();
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
    case FileDisplayNameRole:
        return indexNode->fileInfo->displayName();
    case FileNameRole:
        return indexNode->fileInfo->fileName();
    case FileIconRole:
        if (index.column() == 0) {
            return indexNode->fileInfo->fileIcon();
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

    fileService->addUrlMonitor(parentNode->fileInfo->fileUrl());

    parentNode->populatedChildren = true;

    FMEvent event;

    event = this->parent()->windowId();
    event = parentNode->fileInfo->fileUrl();

    fileService->getChildren(event);
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

QModelIndex DFileSystemModel::setRootPath(const QString &fileUrl)
{
    if(m_rootNode) {
        const QString m_rootFileUrl = m_rootNode->fileInfo->fileUrl();

        if(fileUrl == m_rootFileUrl)
            return createIndex(m_rootNode, 0);

        fileService->removeUrlMonitor(m_rootNode->fileInfo->fileUrl());

        deleteNode((m_rootNode));
    }

    m_rootNode = m_urlToNode.value(fileUrl);

    if(!m_rootNode) {
        m_rootNode = createNode(Q_NULLPTR, fileService->createFileInfo(fileUrl));
    }

    return index(fileUrl);
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

    return node->fileInfo->fileUrl();
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

    const QString &node_absoluteFileUrl = node->fileInfo->fileUrl();

    for(FileSystemNode *url_node : m_urlToNode) {
        if(node == url_node)
            continue;

        url_node->populatedChildren = false;

        if(node_absoluteFileUrl.startsWith(url_node->fileInfo->fileUrl()))
            continue;

        url_node->visibleChildren.clear();
    }

    QList<AbstractFileInfo*> list;

    list.reserve(node->visibleChildren.size());

    for(const QString &fileUrl : node->visibleChildren) {
        list << node->children.value(fileUrl)->fileInfo;
    }

    sort(list);

    for(int i = 0; i < node->visibleChildren.count(); ++i) {
        node->visibleChildren[i] = list[i]->fileUrl();
    }

    QModelIndex parentIndex = createIndex(node, 0);
    QModelIndex topLeftIndex = index(0, 0, parentIndex);
    QModelIndex rightBottomIndex = index(node->visibleChildren.count(), columnCount(parentIndex), parentIndex);

    emit dataChanged(topLeftIndex, rightBottomIndex);
}

AbstractFileInfo *DFileSystemModel::fileInfo(const QModelIndex &index) const
{
    FileSystemNode *node = getNodeByIndex(index);

    return node ? node->fileInfo : Q_NULLPTR;
}

AbstractFileInfo *DFileSystemModel::fileInfo(const QString &fileUrl) const
{
    FileSystemNode *node = m_urlToNode.value(fileUrl);

    return node ? node->fileInfo : Q_NULLPTR;
}

AbstractFileInfo *DFileSystemModel::parentFileInfo(const QModelIndex &index) const
{
    FileSystemNode *node = getNodeByIndex(index);

    return node ? node->parent->fileInfo : Q_NULLPTR;
}

AbstractFileInfo *DFileSystemModel::parentFileInfo(const QString &fileUrl) const
{
    FileSystemNode *node = m_urlToNode.value(fileUrl);

    return node ? node->parent->fileInfo : Q_NULLPTR;
}

void DFileSystemModel::updateChildren(const FMEvent &event, QList<AbstractFileInfo*> list)
{
    if(event.windowId() != parent()->windowId())
        return;

    FileSystemNode *node = getNodeByIndex(index(event.fileUrl()));

    if(!node) {
        qDeleteAll(list);
        return;
    }

    node->children.clear();
    node->visibleChildren.clear();

    sort(list);

    beginInsertRows(createIndex(node, 0), 0, list.count() - 1);

    for(AbstractFileInfo * const fileInfo : list) {
        FileSystemNode *chileNode = createNode(node, fileInfo);

        node->children[fileInfo->fileUrl()] = chileNode;
        node->visibleChildren << fileInfo->fileUrl();
    }

    endInsertRows();
}

void DFileSystemModel::refresh(const QString &fileUrl)
{
    FileSystemNode *node = m_urlToNode.value(fileUrl);

    if(!node)
        return;

    if(!isDir(node))
        return;

    node->populatedChildren = true;

    FMEvent event;

    event = this->parent()->windowId();
    event = fileUrl;

    fileService->getChildren(event);
}

void DFileSystemModel::onFileCreated(const QString &fileUrl)
{
    qDebug() << "file creatored" << fileUrl;

    AbstractFileInfo *info = fileService->createFileInfo(fileUrl);

    if(!info)
        return;

    FileSystemNode *parentNode = m_urlToNode.value(info->parentUrl());

    delete info;

    if(parentNode && parentNode->populatedChildren && !parentNode->visibleChildren.contains(fileUrl)) {
        beginInsertRows(createIndex(parentNode, 0), 0, 0);

        FileSystemNode *node = m_urlToNode.value(fileUrl);

        if(!node) {
            node = createNode(parentNode, fileService->createFileInfo(fileUrl));

            m_urlToNode[fileUrl] = node;
        }

        parentNode->children[fileUrl] = node;
        parentNode->visibleChildren.insert(0, fileUrl);

        endInsertRows();
    }
}

void DFileSystemModel::onFileDeleted(const QString &fileUrl)
{
    qDebug() << "file deleted:" << fileUrl;

    AbstractFileInfo *info = fileService->createFileInfo(fileUrl);

    if(!info)
        return;

    if(info->isDir())
        fileService->removeUrlMonitor(fileUrl);

    FileSystemNode *parentNode = m_urlToNode.value(info->parentUrl());

    delete info;

    if(parentNode && parentNode->populatedChildren) {
        int index = parentNode->visibleChildren.indexOf(fileUrl);

        beginRemoveRows(createIndex(parentNode, 0), index, index);

        parentNode->visibleChildren.removeAt(index);
        parentNode->children.remove(fileUrl);

        endRemoveRows();

        FileSystemNode *node = m_urlToNode.value(fileUrl);

        if(!node)
            return;

        if(hasChildren(createIndex(node, 0))) {
            for(const QString &url : m_urlToNode.keys()) {
                if(fileUrl.startsWith(url)) {
                    deleteNodeByUrl(url);
                }
            }
        }

        deleteNode(node);
    }
}

FileSystemNode *DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return Q_NULLPTR;

    FileSystemNode *indexNode = static_cast<FileSystemNode*>(index.internalPointer());
    Q_ASSERT(indexNode);

    return indexNode;
}

QModelIndex DFileSystemModel::createIndex(const FileSystemNode *node, int column) const
{
    int row = (node->parent && !node->parent->visibleChildren.isEmpty())
            ? node->parent->visibleChildren.indexOf(node->fileInfo->fileUrl())
            : 0;

    return createIndex(row, column, const_cast<FileSystemNode*>(node));
}

bool DFileSystemModel::isDir(const FileSystemNode *node) const
{
    return node->fileInfo->isDir();
}

Qt::SortOrder sortOrder_global;

bool sortFileListByName(const AbstractFileInfo *info1, const AbstractFileInfo *info2)
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

bool sortFileListBySize(const AbstractFileInfo *info1, const AbstractFileInfo *info2)
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

bool sortFileListByModified(const AbstractFileInfo *info1, const AbstractFileInfo *info2)
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

bool sortFileListByMime(const AbstractFileInfo *info1, const AbstractFileInfo *info2)
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

bool sortFileListByCreated(const AbstractFileInfo *info1, const AbstractFileInfo *info2)
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

void DFileSystemModel::sort(QList<AbstractFileInfo*> &list) const
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

FileSystemNode *DFileSystemModel::createNode(FileSystemNode *parent, AbstractFileInfo *info)
{
    Q_ASSERT(info);

    FileSystemNode *node = m_urlToNode.value(info->fileUrl());

    if(node) {
        if(node->fileInfo != info) {
            delete node->fileInfo;
            node->fileInfo = info;
        }

        node->parent = parent;
    } else {
        node = new FileSystemNode(parent, info);

        m_urlToNode[info->fileUrl()] = node;
    }

    return node;
}

void DFileSystemModel::deleteNode(FileSystemNode *node)
{
    m_urlToNode.remove(m_urlToNode.key(node));

    for(FileSystemNode *children : node->children) {
        if(children->parent == node) {
            children->parent = m_urlToNode.value(children->fileInfo->scheme() + "://"
                                                 + children->fileInfo->absolutePath());
        }
    }

    delete node;
}

void DFileSystemModel::deleteNodeByUrl(const QString &url)
{
    delete m_urlToNode.take(url);
}
