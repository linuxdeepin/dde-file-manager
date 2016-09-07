#include "dfilesystemmodel.h"
#include "desktopfileinfo.h"
#include "abstractfileinfo.h"

#include "../app/global.h"
#include "../app/filemanagerapp.h"
#include "../app/fmevent.h"

#include "../views/dfileview.h"

#include "../controllers/appcontroller.h"
#include "../controllers/fileservices.h"
#include "../controllers/jobcontroller.h"

#include "filemonitor/filemonitor.h"

#include "../shutil/mimetypedisplaymanager.h"
#include "../shutil/fileutils.h"

#include <QDebug>
#include <QFileIconProvider>
#include <QDateTime>
#include <QMimeData>
#include <QtConcurrent/QtConcurrent>

#define fileService FileServices::instance()
#define DEFAULT_COLUMN_COUNT 1

class FileSystemNode : public QSharedData
{
public:
    AbstractFileInfoPointer fileInfo;
    FileSystemNode *parent = Q_NULLPTR;
    QHash<DUrl, FileSystemNodePointer> children;
    QList<DUrl> visibleChildren;
    bool populatedChildren = false;

    FileSystemNode(FileSystemNode *parent,
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
            Qt::DirectConnection);
    connect(fileService, &FileServices::childrenUpdated,
            this, &DFileSystemModel::onFileUpdated);

    qRegisterMetaType<State>("State");
    qRegisterMetaType<AbstractFileInfoPointer>("AbstractFileInfoPointer");
}

DFileSystemModel::~DFileSystemModel()
{
//    for(const FileSystemNodePointer &node : m_urlToNode) {
//        if(node->fileInfo->isDir())
//            fileService->removeUrlMonitor(node->fileInfo->fileUrl());
//    }

//    setRootUrl(DUrl());

    if (jobController) {
        jobController->stopAndDeleteLater();
    }

    clear();
}

DFileView *DFileSystemModel::parent() const
{
    return qobject_cast<DFileView*>(QAbstractItemModel::parent());
}

QModelIndex DFileSystemModel::index(const DUrl &fileUrl, int column)
{
    if (fileUrl == rootUrl())
        return createIndex(m_rootNode, column);

    if (!m_rootNode)
        return QModelIndex();

//    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);
    const FileSystemNodePointer &node = m_rootNode->children.value(fileUrl);

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

    FileSystemNodePointer parentNode(indexNode->parent);

    return createIndex(parentNode, 0);
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

//    const AbstractFileInfoPointer &currentFileInfo = fileInfo(m_activeIndex);

    if (!m_rootNode)
        return columnCount;

    const AbstractFileInfoPointer &currentFileInfo = m_rootNode->fileInfo;

    if(currentFileInfo) {
        columnCount += currentFileInfo->userColumnRoles().count();
    }

    return columnCount;
}

QVariant DFileSystemModel::columnNameByRole(int role, const QModelIndex &index) const
{
//    const AbstractFileInfoPointer &fileInfo = this->fileInfo(index.isValid() ? index : m_activeIndex);
    const AbstractFileInfoPointer &fileInfo = index.isValid() ? this->fileInfo(index) : m_rootNode->fileInfo;

    if (fileInfo)
        return fileInfo->userColumnDisplayName(role);

    return QVariant();
}

int DFileSystemModel::columnWidthByRole(int role) const
{
    switch (role) {
    case FileNameRole:
    case FileDisplayNameRole:
        return -1;
    default:
//        const AbstractFileInfoPointer &currentFileInfo = fileInfo(m_activeIndex);
        const AbstractFileInfoPointer &currentFileInfo = m_rootNode->fileInfo;

        if (currentFileInfo)
            return currentFileInfo->userColumnWidth(role);

        return 140;
    }
}

bool DFileSystemModel::columnDefaultVisibleForRole(int role, const QModelIndex &index) const
{
    if (role == FileDisplayNameRole || role == FileNameRole)
        return true;

//    const AbstractFileInfoPointer &fileInfo = this->fileInfo(index.isValid() ? index : m_activeIndex);
    const AbstractFileInfoPointer &fileInfo = index.isValid() ? this->fileInfo(index) : m_rootNode->fileInfo;

    if (fileInfo)
        return fileInfo->columnDefaultVisibleForRole(role);

    return true;
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
    case Qt::DisplayRole: {
        int column_role = columnToRole(index.column());

        return data(index.sibling(index.row(), 0), column_role);
    }
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
    case FileLastModifiedRole:
        return indexNode->fileInfo->lastModifiedDisplayName();
    case FileSizeRole:
        return indexNode->fileInfo->sizeDisplayName();
    case FileMimeTypeRole:{
        return indexNode->fileInfo->mimeTypeDisplayName();
    }
    case FileCreatedRole:
        return indexNode->fileInfo->createdDisplayName();
    case FilePinyinName:
        return indexNode->fileInfo->pinyinName();
    default: {
        const AbstractFileInfoPointer &fileInfo = indexNode->fileInfo;

        return fileInfo->userColumnData(role);
    }
    }

    return QVariant();
}

QVariant DFileSystemModel::headerData(int column, Qt::Orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        int column_role = columnToRole(column);

        if (column_role == FileDisplayNameRole) {
            return roleName(FileDisplayNameRole);
        } else {
//            const AbstractFileInfoPointer &fileInfo = this->fileInfo(m_activeIndex);
            const AbstractFileInfoPointer &fileInfo = m_rootNode->fileInfo;

            if(fileInfo)
                return fileInfo->userColumnDisplayName(column_role);

            return QVariant();
        }
    } else if(role == Qt::BackgroundRole) {
        return QBrush(Qt::white);
    } else if(role == Qt::ForegroundRole) {
        return QBrush(Qt::black);
    }

    return QVariant();
}

QString DFileSystemModel::roleName(int role)
{
    switch(role)
    {
    case FileDisplayNameRole:
    case FileNameRole:
        return tr("Name");
    case FileLastModifiedRole:
        return tr("Time modified");
    case FileSizeRole:
        return tr("Size");
    case FileMimeTypeRole:
        return tr("Type");
    case FileCreatedRole:
        return tr("Time created");
    default: return QString();
    }
}

int DFileSystemModel::columnToRole(int column) const
{
    if (column == 0) {
        return FileDisplayNameRole;
    } else {
//        const AbstractFileInfoPointer &fileInfo = this->fileInfo(m_activeIndex);
        const AbstractFileInfoPointer &fileInfo = m_rootNode->fileInfo;

        if(fileInfo)
            return fileInfo->userColumnRoles().value(column - 1, UnknowRole);
    }

    return UnknowRole;
}

int DFileSystemModel::roleToColumn(int role) const
{
    if (role == FileDisplayNameRole) {
        return 0;
    } else {
        if (!m_rootNode)
            return -1;

//        const AbstractFileInfoPointer &fileInfo = this->fileInfo(m_activeIndex);
        const AbstractFileInfoPointer &fileInfo = m_rootNode->fileInfo;

        if(fileInfo) {
            int column = fileInfo->userColumnRoles().indexOf(role);

            if (column < 0)
                return -1;

            return column + 1;
        }
    }

    return -1;
}

void DFileSystemModel::fetchMore(const QModelIndex &parent)
{
    if (eventLoop)
        eventLoop->exit(1);

    if (!m_rootNode)
        return;

    const FileSystemNodePointer &parentNode = getNodeByIndex(parent);

    if (!parentNode || parentNode->populatedChildren)
        return;

    if (jobController) {
        disconnect(jobController, &JobController::addChildren, this, &DFileSystemModel::addFile);
        disconnect(jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished);
        disconnect(jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildren);

        if (jobController->isFinished()) {
            jobController->deleteLater();
        } else {
            QEventLoop eventLoop(this);

            this->eventLoop = &eventLoop;

            connect(jobController, &JobController::destroyed, &eventLoop, &QEventLoop::quit);

            jobController->stopAndDeleteLater();

            int code = eventLoop.exec();

            this->eventLoop = Q_NULLPTR;

            if (code != 0)
                return;
        }
    }

    jobController = fileService->getChildrenJob(parentNode->fileInfo->fileUrl(), m_filters);

    if (!jobController)
        return;

    connect(jobController, &JobController::addChildren, this, &DFileSystemModel::addFile, Qt::QueuedConnection);
    connect(jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished, Qt::QueuedConnection);
    connect(jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildren, Qt::QueuedConnection);

    fileService->addUrlMonitor(parentNode->fileInfo->fileUrl());

    parentNode->populatedChildren = true;

    setState(Busy);

    childrenUpdated = false;
    jobController->start();
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

    return flags & ~ indexNode->fileInfo->fileItemDisableFlags();
}

Qt::DropActions DFileSystemModel::supportedDragActions() const
{
    if (m_rootNode) {
        return m_rootNode->fileInfo->supportedDragActions();
    }

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions DFileSystemModel::supportedDropActions() const
{
    if (m_rootNode) {
        return m_rootNode->fileInfo->supportedDropActions();
    }

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
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

    for (; it != indexes.end(); ++it) {
        if ((*it).column() == 0) {
            const AbstractFileInfoPointer &fileInfo = this->fileInfo(*it);

            urls << fileInfo->mimeDataUrl();
        }
    }

    QMimeData *data = new QMimeData();
    data->setUrls(urls);

    return data;
}

bool DFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    const FileSystemNodePointer &parentNode = getNodeByIndex(parent);

    if(!parentNode)
        return false;

    return (isDir(parentNode) || !parentNode->fileInfo->exists()) && !parentNode->populatedChildren;
}

QModelIndex DFileSystemModel::setRootUrl(const DUrl &fileUrl)
{
    if (eventLoop)
        eventLoop->exit(1);

    if (m_rootNode) {
        const DUrl m_rootFileUrl = m_rootNode->fileInfo->fileUrl();

        if(fileUrl == m_rootFileUrl)
            return createIndex(m_rootNode, 0);

        clear();
    }

//    m_rootNode = m_urlToNode.value(fileUrl);

    m_rootNode = createNode(Q_NULLPTR, fileService->createFileInfo(fileUrl));

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
    setSortRole(columnToRole(column), order);
}

void DFileSystemModel::setSortRole(int role, Qt::SortOrder order)
{
    m_sortRole = role;
    m_srotOrder = order;
}

//void DFileSystemModel::setActiveIndex(const QModelIndex &index)
//{
//    int old_column_count = columnCount(m_activeIndex);

//    m_activeIndex = index;

//    int new_column_count = columnCount(index);

//    if (old_column_count < new_column_count) {
//        beginInsertColumns(index, old_column_count, new_column_count - 1);
//        endInsertColumns();
//    } else if (old_column_count > new_column_count) {
//        beginRemoveColumns(index, new_column_count, old_column_count - 1);
//        endRemoveColumns();
//    }

//    const FileSystemNodePointer &node = getNodeByIndex(index);

//    if(!node || node->populatedChildren)
//        return;

//    node->visibleChildren.clear();
//}

Qt::SortOrder DFileSystemModel::sortOrder() const
{
    return m_srotOrder;
}

int DFileSystemModel::sortColumn() const
{
    return roleToColumn(m_sortRole);
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
    if (state() == Busy) {
        qWarning() << "I'm busying";

        return;
    }

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if (QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileSystemModel::sort);

        return;
    }

//    const FileSystemNodePointer &node = getNodeByIndex(m_activeIndex);
    const FileSystemNodePointer &node = m_rootNode;

    if(!node)
        return;

    const DUrl &node_absoluteFileUrl = node->fileInfo->fileUrl();

//    for(const FileSystemNodePointer &url_node : m_urlToNode) {
//        if(node == url_node.constData())
//            continue;

//        url_node->populatedChildren = false;

//        if(node_absoluteFileUrl.toString().startsWith(url_node->fileInfo->fileUrl().toString()))
//            continue;

//        url_node->visibleChildren.clear();
//    }

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

    QMetaObject::invokeMethod(this, "dataChanged", Qt::QueuedConnection,
                              Q_ARG(QModelIndex, topLeftIndex), Q_ARG(QModelIndex, rightBottomIndex));
}

const AbstractFileInfoPointer DFileSystemModel::fileInfo(const QModelIndex &index) const
{
    const FileSystemNodePointer &node = getNodeByIndex(index);

    return node ? node->fileInfo : AbstractFileInfoPointer();
}

const AbstractFileInfoPointer DFileSystemModel::fileInfo(const DUrl &fileUrl) const
{
    if (!m_rootNode)
        return AbstractFileInfoPointer();

    if (fileUrl == m_rootNode->fileInfo->fileUrl())
        return m_rootNode->fileInfo;

//    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);
    const FileSystemNodePointer &node = m_rootNode->children.value(fileUrl);

    return node ? node->fileInfo : AbstractFileInfoPointer();
}

const AbstractFileInfoPointer DFileSystemModel::parentFileInfo(const QModelIndex &index) const
{
    const FileSystemNodePointer &node = getNodeByIndex(index);

    return node ? node->parent->fileInfo : AbstractFileInfoPointer();
}

const AbstractFileInfoPointer DFileSystemModel::parentFileInfo(const DUrl &fileUrl) const
{
//    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);
//    const FileSystemNodePointer &node = m_rootNode->children.value(fileUrl);

//    return node ? node->parent->fileInfo : AbstractFileInfoPointer();
    if (fileUrl == rootUrl())
        return m_rootNode->fileInfo;

    return fileService->createFileInfo(fileUrl.parentUrl(fileUrl));
}

DFileSystemModel::State DFileSystemModel::state() const
{
    return m_state;
}

void DFileSystemModel::updateChildren(QList<AbstractFileInfoPointer> list)
{
    if (qApp->thread() == QThread::currentThread()) {
        if (QThreadPool::globalInstance()->activeThreadCount() >= QThreadPool::globalInstance()->maxThreadCount())
            QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() + 10);

        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileSystemModel::updateChildren, list);

        return;
    };

    const FileSystemNodePointer &node = m_rootNode;

    if(!node) {
        return;
    }

    QPointer<JobController> job = jobController;

    if (job)
        job->pause();

    for (const DUrl url : node->visibleChildren) {
        deleteNodeByUrl(url);
    }

    node->children.clear();
    node->visibleChildren.clear();

    sort(node->fileInfo, list);

    beginInsertRows(createIndex(node, 0), 0, list.count() - 1);

    for(const AbstractFileInfoPointer &fileInfo : list) {
        if (node->children.contains(fileInfo->fileUrl()))
            continue;

        const FileSystemNodePointer &chileNode = createNode(node.data(), fileInfo);

        node->children[fileInfo->fileUrl()] = chileNode;
        node->visibleChildren << fileInfo->fileUrl();
    }

    endInsertRows();

    if (!jobController || jobController->isFinished()) {
        setState(Idle);
    } else {
        childrenUpdated = true;
    }

    if (job && job->state() == JobController::Paused)
        job->start();
}

void DFileSystemModel::refresh(const DUrl &fileUrl)
{
//    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);
    const FileSystemNodePointer &node = m_rootNode;

    if (!node)
        return;

    if (!fileUrl.isEmpty() && fileUrl != node->fileInfo->fileUrl())
        return;

//    if(!isDir(node))
//        return;

    node->populatedChildren = false;

    const QModelIndex &index = createIndex(node, 0);

    beginRemoveRows(index, 0, rowCount(index) - 1);

    node->children.clear();
    node->visibleChildren.clear();

    endRemoveRows();

    fetchMore(index);
}

void DFileSystemModel::update()
{
    const QModelIndex &rootIndex = createIndex(m_rootNode, 0);

    emit dataChanged(rootIndex.child(0, 0), rootIndex.child(rootIndex.row() - 1, 0));
}

void DFileSystemModel::toggleHiddenFiles(const DUrl &fileUrl)
{
    m_filters = ~(m_filters ^ QDir::Filter(~QDir::Hidden));

    refresh(fileUrl);
}


void DFileSystemModel::onFileCreated(const DUrl &fileUrl)
{
    qDebug() << "file creatored" << fileUrl;

    const AbstractFileInfoPointer &info = fileService->createFileInfo(fileUrl);

    if (!info)
        return;

//    const FileSystemNodePointer &parentNode = m_urlToNode.value(info->parentUrl());
    if (fileUrl == rootUrl()) {
        return refresh();
    }

    if (info->parentUrl() != rootUrl())
        return;

    addFile(info);
}

void DFileSystemModel::onFileDeleted(const DUrl &fileUrl)
{
    qDebug() << "file deleted:" << fileUrl;

    const AbstractFileInfoPointer &info = fileService->createFileInfo(fileUrl);
    if(!info)
        return;

//    const FileSystemNodePointer &parentNode = m_urlToNode.value(info->parentUrl());
    const DUrl &rootUrl = this->rootUrl();

    if (fileUrl == rootUrl) {
        return refresh();
    }

    if (info->parentUrl() != rootUrl)
        return;

    const FileSystemNodePointer &parentNode = m_rootNode;
    if(parentNode && parentNode->populatedChildren) {
        int index = parentNode->visibleChildren.indexOf(fileUrl);
        beginRemoveRows(createIndex(parentNode, 0), index, index);
        parentNode->visibleChildren.removeAt(index);
        parentNode->children.remove(fileUrl);
        endRemoveRows();

//        const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);

//        if(!node)
//            return;

//        if(hasChildren(createIndex(node, 0))) {
//            for(const DUrl &url : m_urlToNode.keys()) {
//                if(fileUrl.toString().startsWith(url.toString())) {
//                    deleteNodeByUrl(url);
//                }
//            }
//        }

//        deleteNode(node);
    }
}

void DFileSystemModel::onFileUpdated(const DUrl &fileUrl)
{
//    const FileSystemNodePointer &node = m_urlToNode.value(fileUrl);

    const FileSystemNodePointer &node = m_rootNode;

    if(!node)
        return;

    const QModelIndex &index = this->index(fileUrl);

    if(!index.isValid())
        return;

    const FileSystemNodePointer &child = node->children.value(fileUrl);

    if (child && child->fileInfo)
        child->fileInfo->updateFileInfo();

    emit dataChanged(index, index);
}

const FileSystemNodePointer DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    if (!m_rootNode)
        return FileSystemNodePointer();

    FileSystemNode *indexNode = static_cast<FileSystemNode*>(index.internalPointer());

    if (indexNode == m_rootNode.constData()
            || m_rootNode->children.value(m_rootNode->visibleChildren.value(index.row())).constData() != indexNode) {
        return m_rootNode;
    }

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
    if (!parentInfo)
        return;

    parentInfo->sortByColumnRole(list, m_sortRole, m_srotOrder);
}

const FileSystemNodePointer DFileSystemModel::createNode(FileSystemNode *parent, const AbstractFileInfoPointer &info)
{
    Q_ASSERT(info);

//    const FileSystemNodePointer &node = m_urlToNode.value(info->fileUrl());

//    if(node) {
//        if(node->fileInfo != info) {
//            node->fileInfo = info;
//        }

//        node->parent = parent;

//        return node;
//    } else {
        FileSystemNodePointer node(new FileSystemNode(parent, info));

//        m_urlToNode[info->fileUrl()] = node;

        return node;
//    }
}

void DFileSystemModel::deleteNode(const FileSystemNodePointer &node)
{
//    m_urlToNode.remove(m_urlToNode.key(node));

//    for(const FileSystemNodePointer &children : node->children) {
//        if(children->parent == node) {
//            deleteNode(children);
//        }
//    }
    deleteNodeByUrl(node->fileInfo->fileUrl());
}

void DFileSystemModel::deleteNodeByUrl(const DUrl &url)
{
    fileService->removeUrlMonitor(url);
    //    m_urlToNode.take(url);
}

void DFileSystemModel::clear()
{
    if (!m_rootNode)
        return;

    const QModelIndex &index = createIndex(m_rootNode, 0);

    beginRemoveRows(index, 0, rowCount(index) - 1);

    deleteNode((m_rootNode));

    endRemoveRows();
}

void DFileSystemModel::setState(DFileSystemModel::State state)
{
    if (m_state == state)
        return;

    m_state = state;

    emit stateChanged(state);
}

void DFileSystemModel::onJobFinished()
{
    if (childrenUpdated)
        setState(Idle);
}

void DFileSystemModel::addFile(const AbstractFileInfoPointer &fileInfo)
{
    const FileSystemNodePointer &parentNode = m_rootNode;
    const DUrl &fileUrl = fileInfo->fileUrl();

    if (parentNode && parentNode->populatedChildren && !parentNode->visibleChildren.contains(fileUrl)) {
        QPointer<DFileSystemModel> me = this;

        auto getFileInfoFun =   [&parentNode, &me] (int index)->const AbstractFileInfoPointer {
                                    qApp->processEvents();

                                    if (!me || index >= parentNode->visibleChildren.count())
                                        return AbstractFileInfoPointer();

                                    return parentNode->children.value(parentNode->visibleChildren.value(index))->fileInfo;
                                };

        int row = parentNode->fileInfo->getIndexByFileInfo(getFileInfoFun, fileInfo, m_sortRole, m_srotOrder);

        if (!me)
            return;

        if (row == -1)
            row = parentNode->visibleChildren.count();

        beginInsertRows(createIndex(parentNode, 0), row, row);

//        FileSystemNodePointer node = m_urlToNode.value(fileUrl);

//        if(!node) {
            FileSystemNodePointer node = createNode(parentNode.data(), fileInfo);

//            m_urlToNode[fileUrl] = node;
//        }

        parentNode->children[fileUrl] = node;
        parentNode->visibleChildren.insert(row, fileUrl);

        endInsertRows();
    }
}
