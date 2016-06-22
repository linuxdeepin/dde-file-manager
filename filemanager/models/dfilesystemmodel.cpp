#include "dfilesystemmodel.h"
#include "desktopfileinfo.h"
#include "abstractfileinfo.h"

#include "../app/global.h"
#include "../views/dfileview.h"

#include "../app/filemanagerapp.h"
#include "../app/fmevent.h"

#include "../controllers/appcontroller.h"
#include "../controllers/fileservices.h"

#include "filemonitor/filemonitor.h"
#include "../shutil/mimetypedisplaymanager.h"
#include "../shutil/fileutils.h"

#include <QDebug>
#include <QFileIconProvider>
#include <QDateTime>
#include <QMimeData>

#define fileService FileServices::instance()
#define DEFAULT_COLUMN_COUNT 5

class FileSystemNode : public QSharedData
{
public:
    AbstractFileInfoPointer fileInfo;
    FileSystemNodePointer parent;
    QHash<DUrl, FileSystemNodePointer> children;
    QList<DUrl> visibleChildren;
    bool populatedChildren = false;

    FileSystemNode(const FileSystemNodePointer &parent,
                   const AbstractFileInfoPointer &info) :
        fileInfo(info),
        parent(parent)
    {

    }
};

DFileSystemModel::DFileSystemModel(DFileView *parent)
    : QAbstractItemModel(parent)
{
    connect(fileService, &FileServices::childrenAdded,
            this, &DFileSystemModel::onFileCreated,
            Qt::DirectConnection);
    connect(fileService, &FileServices::childrenRemoved,
            this, &DFileSystemModel::onFileDeleted,
            Qt::QueuedConnection);
    connect(fileService, &FileServices::childrenUpdated,
            this, &DFileSystemModel::onFileUpdated);
    connect(fileService, &FileServices::updateChildren,
            this, &DFileSystemModel::updateChildren,
            Qt::DirectConnection);
}

DFileSystemModel::~DFileSystemModel()
{
    for(const FileSystemNodePointer &node : m_urlToNode) {
        if(node->fileInfo->isDir())
            fileService->removeUrlMonitor(node->fileInfo->fileUrl());
    }
}

DFileView *DFileSystemModel::parent() const
{
    return qobject_cast<DFileView*>(QAbstractItemModel::parent());
}

QModelIndex DFileSystemModel::index(const DUrl &fileUrl, int column)
{
    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);

    if (!node)
        return QModelIndex();

    QModelIndex idx = createIndex(node, column);

    return idx;
}

QModelIndex DFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    const FileSystemNodePointer &parentNode = parent.isValid()
                                                ? FileSystemNodePointer(getNodeByIndex(parent))
                                                : m_rootNode;

    if(!parentNode)
        return QModelIndex();

    const DUrl &childFileUrl = parentNode->visibleChildren.value(row);
    const FileSystemNodePointer &childNode = parentNode->children.value(childFileUrl);

    if(!childNode)
        return QModelIndex();

    return createIndex(row, column, childNode.data());
}

QModelIndex DFileSystemModel::parent(const QModelIndex &child) const
{
    const FileSystemNodePointer &indexNode = getNodeByIndex(child);

    if(!indexNode || !indexNode->parent)
        return QModelIndex();

    return createIndex(indexNode->parent, 0);
}

int DFileSystemModel::rowCount(const QModelIndex &parent) const
{
    const FileSystemNodePointer &parentNode = parent.isValid()
                                 ? FileSystemNodePointer(getNodeByIndex(parent))
                                 : m_rootNode;

    if(!parentNode)
        return 0;

    return parentNode->visibleChildren.count();
}

int DFileSystemModel::columnCount(const QModelIndex &parent) const
{
    int columnCount = parent.column() > 0 ? 0 : DEFAULT_COLUMN_COUNT;

    const AbstractFileInfoPointer &currentFileInfo = fileInfo(m_activeIndex);

    if(currentFileInfo) {
        columnCount += currentFileInfo->userColumnCount();
    }

    return columnCount;
}

int DFileSystemModel::columnWidthByRole(int role) const
{
    switch (role) {
    case FileNameRole:
    case FileDisplayNameRole:
        return -1;
    case FileSizeRole:
        return 100;
    case FileMimeTypeRole:
        return 100;
    default:
        if (role >= FileUserRole) {
            const AbstractFileInfoPointer &currentFileInfo = fileInfo(m_activeIndex);

            if (currentFileInfo)
                return currentFileInfo->userColumnWidth(role - FileUserRole);
        }

        return 140;
    }
}

bool DFileSystemModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) // drives
        return false;

    const FileSystemNodePointer &indexNode = getNodeByIndex(parent);
    Q_ASSERT(indexNode);

    return isDir(indexNode);
}

QVariant DFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    const FileSystemNodePointer &indexNode = getNodeByIndex(index);

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
        switch(columnToRole(index.column())) {
        case FileCreatedRole:
        case FileLastModifiedRole:
        case FileLastReadRole:
            return Qt::AlignCenter;
        default:
            return Qt::AlignVCenter;
        }
    case FileLastModifiedRole:
        return indexNode->fileInfo->lastModifiedDisplayName();
    case FileSizeRole:
        return indexNode->fileInfo->sizeDisplayName();
    case FileMimeTypeRole:{
        return indexNode->fileInfo->mimeTypeDisplayName();
    }
    case FileCreatedRole:
        return indexNode->fileInfo->createdDisplayName();
    default: {
        const AbstractFileInfoPointer &fileInfo = indexNode->fileInfo;

        return fileInfo->userColumnData(AbstractFileInfo::UserType + role - FileUserRole);
    }
    }

    return QVariant();
}

QVariant DFileSystemModel::headerData(int section, Qt::Orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        switch(section)
        {
        case 0: return tr("Name");
        case 1: return tr("Time modified");
        case 2: return tr("Size");
        case 3: return tr("Type");
        case 4: return tr("Time created");
        default: {
            const AbstractFileInfoPointer &fileInfo = this->fileInfo(m_activeIndex);

            if(fileInfo)
                return fileInfo->userColumnDisplayName(AbstractFileInfo::UserType + section - DEFAULT_COLUMN_COUNT);

            return QVariant();
        }
        }
    } else if(role == Qt::BackgroundRole) {
        return QBrush(Qt::white);
    } else if(role == Qt::ForegroundRole) {
        return QBrush(Qt::black);
    }

    return QVariant();
}

int DFileSystemModel::columnToRole(int column) const
{
    switch (column) {
    case 0:
        return FileDisplayNameRole;
    case 1:
        return FileLastModifiedRole;
    case 2:
        return FileSizeRole;
    case 3:
        return FileMimeTypeRole;
    case 4:
        return FileCreatedRole;
    default:
        return FileUserRole + column - DEFAULT_COLUMN_COUNT;
    }
}

int DFileSystemModel::roleToColumn(int role) const
{
    switch (role) {
    case FileDisplayNameRole:
        return 0;
    case FileLastModifiedRole:
        return 1;
    case FileSizeRole:
        return 2;
    case FileMimeTypeRole:
        return 3;
    case FileCreatedRole:
        return 4;
    default:
        return role - FileUserRole + DEFAULT_COLUMN_COUNT;
    }
}

void DFileSystemModel::fetchMore(const QModelIndex &parent)
{
    if(!m_rootNode)
        return;

    const FileSystemNodePointer &parentNode = getNodeByIndex(parent);

    if(!parentNode || parentNode->populatedChildren)
        return;

    fileService->addUrlMonitor(parentNode->fileInfo->fileUrl());

    parentNode->populatedChildren = true;

    FMEvent event;

    event = this->parent()->windowId();
    event = parentNode->fileInfo->fileUrl();

    fileService->getChildren(event, m_filters);
}

Qt::ItemFlags DFileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    const FileSystemNodePointer &indexNode = getNodeByIndex(index);

    flags |= Qt::ItemIsDragEnabled;

    if(indexNode->fileInfo->isCanRename())
        flags |= Qt::ItemIsEditable;

    if ((index.column() == 0)) {
        if(indexNode->fileInfo->isWritable()) {
            if (isDir(indexNode))
                flags |= Qt::ItemIsDropEnabled;
            else
                flags |= Qt::ItemNeverHasChildren;
        }
    } else {
        flags = flags & ~Qt::ItemIsSelectable;
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
    DUrl toUrl = getUrlByIndex(parent);

    DUrlList urlList = DUrl::fromQUrlList(data->urls());

    FMEvent event;

    event = this->parent()->windowId();
    event = toUrl;

    switch (action) {
    case Qt::CopyAction:
        fileService->pasteFile(AbstractFileController::CopyType, urlList, event);
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        fileService->pasteFile(AbstractFileController::CutType, urlList, event);
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
    const FileSystemNodePointer &parentNode = getNodeByIndex(parent);

    if(!parentNode)
        return false;

    return isDir(parentNode) && !parentNode->populatedChildren;
}

QModelIndex DFileSystemModel::setRootUrl(const DUrl &fileUrl)
{
    if(m_rootNode) {
        const DUrl m_rootFileUrl = m_rootNode->fileInfo->fileUrl();

        if(fileUrl == m_rootFileUrl)
            return createIndex(m_rootNode, 0);

        fileService->removeUrlMonitor(m_rootNode->fileInfo->fileUrl());

        deleteNode((m_rootNode));
    }

    m_rootNode = m_urlToNode.value(fileUrl);

    if(!m_rootNode) {
        m_rootNode = createNode(FileSystemNodePointer(), fileService->createFileInfo(fileUrl));
    }

    return index(fileUrl);
}

DUrl DFileSystemModel::rootUrl() const
{
    return m_rootNode ? m_rootNode->fileInfo->fileUrl() : DUrl();
}

DUrl DFileSystemModel::getUrlByIndex(const QModelIndex &index) const
{
    const FileSystemNodePointer &node = getNodeByIndex(index);

    if(!node)
        return DUrl();

    return node->fileInfo->fileUrl();
}

void DFileSystemModel::setSortColumn(int column, Qt::SortOrder order)
{
    m_sortColumn = column;

    switch (m_sortColumn) {
    case 0:
        setSortRole(DFileSystemModel::FileDisplayNameRole, order);
        break;
    case 1:
        setSortRole(DFileSystemModel::FileLastModifiedRole, order);
        break;
    case 2:
        setSortRole(DFileSystemModel::FileSizeRole, order);
        break;
    case 3:
        setSortRole(DFileSystemModel::FileMimeTypeRole, order);
        break;
    case 4:
        setSortRole(DFileSystemModel::FileCreatedRole, order);
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
    int old_column_count = columnCount(m_activeIndex);

    m_activeIndex = index;

    int new_column_count = columnCount(index);

    if (old_column_count < new_column_count) {
        beginInsertColumns(index, old_column_count, new_column_count - 1);
        endInsertColumns();
    } else if (old_column_count > new_column_count) {
        beginRemoveColumns(index, new_column_count, old_column_count - 1);
        endRemoveColumns();
    }

    const FileSystemNodePointer &node = getNodeByIndex(index);

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

    sort();
}

void DFileSystemModel::sort()
{
    const FileSystemNodePointer &node = getNodeByIndex(m_activeIndex);

    if(!node)
        return;

    const DUrl &node_absoluteFileUrl = node->fileInfo->fileUrl();

    for(const FileSystemNodePointer &url_node : m_urlToNode) {
        if(node == url_node.constData())
            continue;

        url_node->populatedChildren = false;

        if(node_absoluteFileUrl.toString().startsWith(url_node->fileInfo->fileUrl().toString()))
            continue;

        url_node->visibleChildren.clear();
    }

    QList<AbstractFileInfoPointer> list;

    list.reserve(node->visibleChildren.size());

    for(const DUrl &fileUrl : node->visibleChildren) {
        list << node->children.value(fileUrl)->fileInfo;
    }

    sort(node->fileInfo, list);

    for(int i = 0; i < node->visibleChildren.count(); ++i) {
        node->visibleChildren[i] = list[i]->fileUrl();
    }

    QModelIndex parentIndex = createIndex(node, 0);
    QModelIndex topLeftIndex = index(0, 0, parentIndex);
    QModelIndex rightBottomIndex = index(node->visibleChildren.count(), columnCount(parentIndex), parentIndex);

    emit dataChanged(topLeftIndex, rightBottomIndex);
}

const AbstractFileInfoPointer DFileSystemModel::fileInfo(const QModelIndex &index) const
{
    const FileSystemNodePointer &node = getNodeByIndex(index);

    return node ? node->fileInfo : AbstractFileInfoPointer();
}

const AbstractFileInfoPointer DFileSystemModel::fileInfo(const DUrl &fileUrl) const
{
    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);

    return node ? node->fileInfo : AbstractFileInfoPointer();
}

const AbstractFileInfoPointer DFileSystemModel::parentFileInfo(const QModelIndex &index) const
{
    const FileSystemNodePointer &node = getNodeByIndex(index);

    return node ? node->parent->fileInfo : AbstractFileInfoPointer();
}

const AbstractFileInfoPointer DFileSystemModel::parentFileInfo(const DUrl &fileUrl) const
{
    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);

    return node ? node->parent->fileInfo : AbstractFileInfoPointer();
}

void DFileSystemModel::updateChildren(const FMEvent &event, QList<AbstractFileInfoPointer> list)
{
    if(event.windowId() != parent()->windowId())
        return;

    const FileSystemNodePointer &node = getNodeByIndex(index(event.fileUrl()));

    if(!node) {
        return;
    }

    for (const DUrl url : node->visibleChildren) {
        deleteNodeByUrl(url);
    }

    node->children.clear();
    node->visibleChildren.clear();

    sort(node->fileInfo, list);

    beginInsertRows(createIndex(node, 0), 0, list.count() - 1);

    for(const AbstractFileInfoPointer &fileInfo : list) {
        const FileSystemNodePointer &chileNode = createNode(node, fileInfo);

        node->children[fileInfo->fileUrl()] = chileNode;
        node->visibleChildren << fileInfo->fileUrl();
    }

    endInsertRows();
}

void DFileSystemModel::refresh(const DUrl &fileUrl, QDir::Filters filters)
{
    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);

    if(!node)
        return;

    if(!isDir(node))
        return;

    node->populatedChildren = true;

    FMEvent event;

    event = this->parent()->windowId();
    event = fileUrl;

    fileService->getChildren(event, filters);
}


void DFileSystemModel::toggleHiddenFiles(const DUrl &fileUrl)
{
    if (m_filters == (QDir::AllEntries | QDir::NoDotAndDotDot)){
        m_filters = QDir::AllEntries | QDir::Hidden |QDir::NoDotAndDotDot;
        refresh(fileUrl, m_filters);
    }else if (m_filters == (QDir::AllEntries | QDir::Hidden |QDir::NoDotAndDotDot)){
        m_filters = QDir::AllEntries |QDir::NoDotAndDotDot;
        refresh(fileUrl, m_filters);
    }
}


void DFileSystemModel::onFileCreated(const DUrl &fileUrl)
{
    qDebug() << "file creatored" << fileUrl;

    const AbstractFileInfoPointer &info = fileService->createFileInfo(fileUrl);

    if(!info)
        return;

    const FileSystemNodePointer &parentNode = m_urlToNode.value(info->parentUrl());

    if(parentNode && parentNode->populatedChildren && !parentNode->visibleChildren.contains(fileUrl)) {
        auto getFileInfoFun =   [&parentNode] (int index)->const AbstractFileInfoPointer {
                                    if(index >= parentNode->visibleChildren.count())
                                        return AbstractFileInfoPointer();

                                    return parentNode->children.value(parentNode->visibleChildren.value(index))->fileInfo;
                                };

        int row = parentNode->fileInfo->getIndexByFileInfo(getFileInfoFun, info, m_sortColumn, m_srotOrder);

        if(row == -1)
            row = parentNode->visibleChildren.count();

        beginInsertRows(createIndex(parentNode, 0), row, row);

        FileSystemNodePointer node = m_urlToNode.value(fileUrl);

        if(!node) {
            node = createNode(parentNode, info);

            m_urlToNode[fileUrl] = node;
        }

        parentNode->children[fileUrl] = node;
        parentNode->visibleChildren.insert(row, fileUrl);

        endInsertRows();
    }
}

void DFileSystemModel::onFileDeleted(const DUrl &fileUrl)
{
    qDebug() << "file deleted:" << fileUrl;

    const AbstractFileInfoPointer &info = fileService->createFileInfo(fileUrl);

    if(!info)
        return;

    if(info->isDir())
        fileService->removeUrlMonitor(fileUrl);

    const FileSystemNodePointer &parentNode = m_urlToNode.value(info->parentUrl());

    if(parentNode && parentNode->populatedChildren) {
        int index = parentNode->visibleChildren.indexOf(fileUrl);

        beginRemoveRows(createIndex(parentNode, 0), index, index);

        parentNode->visibleChildren.removeAt(index);
        parentNode->children.remove(fileUrl);

        endRemoveRows();

        const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);

        if(!node)
            return;

        if(hasChildren(createIndex(node, 0))) {
            for(const DUrl &url : m_urlToNode.keys()) {
                if(fileUrl.toString().startsWith(url.toString())) {
                    deleteNodeByUrl(url);
                }
            }
        }

        deleteNode(node);
    }
}

void DFileSystemModel::onFileUpdated(const DUrl &fileUrl)
{
    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);

    if(!node)
        return;

    const QModelIndex &index = this->index(fileUrl);

    if(!index.isValid())
        return;

    node->fileInfo = fileService->createFileInfo(fileUrl);

    emit dataChanged(index, index);
}

const FileSystemNodePointer DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return FileSystemNodePointer();

    FileSystemNode *indexNode = static_cast<FileSystemNode*>(index.internalPointer());
    Q_ASSERT(indexNode);

    return FileSystemNodePointer(indexNode);
}

QModelIndex DFileSystemModel::createIndex(const FileSystemNodePointer &node, int column) const
{
    int row = (node->parent && !node->parent->visibleChildren.isEmpty())
            ? node->parent->visibleChildren.indexOf(node->fileInfo->fileUrl())
            : 0;

    return createIndex(row, column, const_cast<FileSystemNode*>(node.data()));
}

bool DFileSystemModel::isDir(const FileSystemNodePointer &node) const
{
    return node->fileInfo->isDir();
}

void DFileSystemModel::sort(const AbstractFileInfoPointer &parentInfo, QList<AbstractFileInfoPointer> &list) const
{
    switch (m_sortRole) {
    case FileDisplayNameRole:
    case FileNameRole:
        parentInfo->sortByColumn(list, AbstractFileInfo::DisplayNameType, m_srotOrder);
        break;
    case FileSizeRole:
        parentInfo->sortByColumn(list, AbstractFileInfo::SizeType, m_srotOrder);
        break;
    case FileLastModifiedRole:
        parentInfo->sortByColumn(list, AbstractFileInfo::LastModifiedDateType, m_srotOrder);
        break;
    case FileCreatedRole:
        parentInfo->sortByColumn(list, AbstractFileInfo::CreatedDateType, m_srotOrder);
        break;
    case FileMimeTypeRole:
        parentInfo->sortByColumn(list, AbstractFileInfo::FileMimeType, m_srotOrder);
        break;
    default:
        break;
    }
}

const FileSystemNodePointer DFileSystemModel::createNode(const FileSystemNodePointer &parent, const AbstractFileInfoPointer &info)
{
    Q_ASSERT(info);

    const FileSystemNodePointer &node = m_urlToNode.value(info->fileUrl());

    if(node) {
        if(node->fileInfo != info) {
            node->fileInfo = info;
        }

        node->parent = parent;

        return node;
    } else {
        FileSystemNodePointer node(new FileSystemNode(parent, info));

        m_urlToNode[info->fileUrl()] = node;

        return node;
    }
}

void DFileSystemModel::deleteNode(const FileSystemNodePointer &node)
{
    m_urlToNode.remove(m_urlToNode.key(node));

    for(const FileSystemNodePointer &children : node->children) {
        if(children->parent == node) {
            deleteNode(children);
        }
    }
}

void DFileSystemModel::deleteNodeByUrl(const DUrl &url)
{
    m_urlToNode.take(url);
}
