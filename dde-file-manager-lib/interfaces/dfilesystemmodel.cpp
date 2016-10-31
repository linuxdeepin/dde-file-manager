#include "dfilesystemmodel.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmevent.h"

#include "app/define.h"

#include "controllers/jobcontroller.h"

#include "filemonitor/filemonitor.h"
#include "interfaces/dfileviewhelper.h"

#include <QDebug>
#include <QMimeData>
#include <QtConcurrent/QtConcurrent>
#include <QAbstractItemView>

#define fileService DFileService::instance()
#define DEFAULT_COLUMN_COUNT 1

class FileSystemNode : public QSharedData
{
public:
    DAbstractFileInfoPointer fileInfo;
    FileSystemNode *parent = Q_NULLPTR;
    QHash<DUrl, FileSystemNodePointer> children;
    QList<DUrl> visibleChildren;
    bool populatedChildren = false;

    FileSystemNode(FileSystemNode *parent,
                   const DAbstractFileInfoPointer &info) :
        fileInfo(info),
        parent(parent)
    {

    }
};

class DFileSystemModelPrivate
{
public:
    DFileSystemModelPrivate(DFileSystemModel *qq)
        : q_ptr(qq) {}

    bool passNameFilters(const FileSystemNodePointer &node) const;

    DFileSystemModel *q_ptr;

    FileSystemNodePointer rootNode;

//    QHash<DUrl, FileSystemNodePointer> d->urlToNode;

    int sortRole = DFileSystemModel::FileDisplayNameRole;
    QStringList nameFilters;
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    Qt::SortOrder srotOrder = Qt::AscendingOrder;
//    QModelIndex d->activeIndex;

    QPointer<JobController> jobController;
    QEventLoop *eventLoop = Q_NULLPTR;
    QFuture<void> updateChildrenFuture;

    DFileSystemModel::State state = DFileSystemModel::Idle;

    bool childrenUpdated = false;
    bool readOnly = false;

    Q_DECLARE_PUBLIC(DFileSystemModel)
};

bool DFileSystemModelPrivate::passNameFilters(const FileSystemNodePointer &node) const
{
    if (nameFilters.isEmpty())
        return true;

    // Check the name regularexpression filters
    if (!(node->fileInfo->isDir() && (filters & QDir::AllDirs))) {
        for (int i = 0; i < nameFilters.size(); ++i) {
            const Qt::CaseSensitivity caseSensitive = (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
            QRegExp re(nameFilters.at(i), caseSensitive, QRegExp::Wildcard);
            if (re.exactMatch(node->fileInfo->fileDisplayName()))
                return true;
        }

        return false;
    }

    return true;
}

DFileSystemModel::DFileSystemModel(DFileViewHelper *parent)
    : QAbstractItemModel(parent)
    , d_ptr(new DFileSystemModelPrivate(this))
{
    connect(fileService, &DFileService::childrenAdded,
            this, &DFileSystemModel::onFileCreated,
            Qt::DirectConnection);
    connect(fileService, &DFileService::childrenRemoved,
            this, &DFileSystemModel::onFileDeleted,
            Qt::DirectConnection);
    connect(fileService, &DFileService::childrenUpdated,
            this, &DFileSystemModel::onFileUpdated);

    qRegisterMetaType<State>("State");
    qRegisterMetaType<DAbstractFileInfoPointer>("DAbstractFileInfoPointer");
}

DFileSystemModel::~DFileSystemModel()
{
    Q_D(DFileSystemModel);
//    for(const FileSystemNodePointer &node : d->urlToNode) {
//        if(node->fileInfo->isDir())
//            fileService->removeUrlMonitor(node->fileInfo->fileUrl());
//    }

//    setRootUrl(DUrl());

    if (d->jobController) {
        d->jobController->stopAndDeleteLater();
    }

    if (d->updateChildrenFuture.isRunning()) {
        d->updateChildrenFuture.cancel();
        d->updateChildrenFuture.waitForFinished();
    }

    clear();
}

DFileViewHelper *DFileSystemModel::parent() const
{
    return static_cast<DFileViewHelper*>(QAbstractItemModel::parent());
}

QModelIndex DFileSystemModel::index(const DUrl &fileUrl, int column)
{
    Q_D(DFileSystemModel);

    if (fileUrl == rootUrl())
        return createIndex(d->rootNode, column);

    if (!d->rootNode)
        return QModelIndex();

//    const FileSystemNodePointer &node = d->urlToNode.value(fileUrl);
    const FileSystemNodePointer &node = d->rootNode->children.value(fileUrl);

    if (!node)
        return QModelIndex();

    QModelIndex idx = createIndex(node, column);

    return idx;
}

QModelIndex DFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const DFileSystemModel);

    if (row < 0 || column < 0/* || row >= rowCount(parent) || column >= columnCount(parent)*/)
        return QModelIndex();

    const FileSystemNodePointer &parentNode = parent.isValid()
                                                ? FileSystemNodePointer(getNodeByIndex(parent))
                                                : d->rootNode;

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
    Q_D(const DFileSystemModel);

    const FileSystemNodePointer &parentNode = parent.isValid()
                                 ? FileSystemNodePointer(getNodeByIndex(parent))
                                 : d->rootNode;

    if(!parentNode)
        return 0;

    return parentNode->visibleChildren.count();
}

int DFileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const DFileSystemModel);

    int columnCount = parent.column() > 0 ? 0 : DEFAULT_COLUMN_COUNT;

//    const AbstractFileInfoPointer &currentFileInfo = fileInfo(d->activeIndex);

    if (!d->rootNode)
        return columnCount;

    const DAbstractFileInfoPointer &currentFileInfo = d->rootNode->fileInfo;

    if(currentFileInfo) {
        columnCount += currentFileInfo->userColumnRoles().count();
    }

    return columnCount;
}

QVariant DFileSystemModel::columnNameByRole(int role, const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);

//    const AbstractFileInfoPointer &fileInfo = this->fileInfo(index.isValid() ? index : d->activeIndex);
    const DAbstractFileInfoPointer &fileInfo = index.isValid() ? this->fileInfo(index) : d->rootNode->fileInfo;

    if (fileInfo)
        return fileInfo->userColumnDisplayName(role);

    return QVariant();
}

int DFileSystemModel::columnWidthByRole(int role) const
{
    Q_D(const DFileSystemModel);

    switch (role) {
    case FileNameRole:
    case FileDisplayNameRole:
        return -1;
    default:
//        const AbstractFileInfoPointer &currentFileInfo = fileInfo(d->activeIndex);
        const DAbstractFileInfoPointer &currentFileInfo = d->rootNode->fileInfo;

        if (currentFileInfo)
            return currentFileInfo->userColumnWidth(role, parent()->parent()->fontMetrics());

        return 140;
    }
}

bool DFileSystemModel::columnDefaultVisibleForRole(int role, const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);

    if (role == FileDisplayNameRole || role == FileNameRole)
        return true;

//    const AbstractFileInfoPointer &fileInfo = this->fileInfo(index.isValid() ? index : d->activeIndex);
    const DAbstractFileInfoPointer &fileInfo = index.isValid() ? this->fileInfo(index) : d->rootNode->fileInfo;

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
        return indexNode->fileInfo->fileDisplayName();
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
        return indexNode->fileInfo->fileDisplayPinyinName();
    default: {
        const DAbstractFileInfoPointer &fileInfo = indexNode->fileInfo;

        return fileInfo->userColumnData(role);
    }
    }

    return QVariant();
}

QVariant DFileSystemModel::headerData(int column, Qt::Orientation, int role) const
{
    Q_D(const DFileSystemModel);

    if(role == Qt::DisplayRole) {
        int column_role = columnToRole(column);

        if (column_role == FileDisplayNameRole) {
            return roleName(FileDisplayNameRole);
        } else {
//            const AbstractFileInfoPointer &fileInfo = this->fileInfo(d->activeIndex);
            const DAbstractFileInfoPointer &fileInfo = d->rootNode->fileInfo;

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
    Q_D(const DFileSystemModel);

    if (column == 0) {
        return FileDisplayNameRole;
    } else {
//        const AbstractFileInfoPointer &fileInfo = this->fileInfo(d->activeIndex);
        const DAbstractFileInfoPointer &fileInfo = d->rootNode->fileInfo;

        if(fileInfo)
            return fileInfo->userColumnRoles().value(column - 1, UnknowRole);
    }

    return UnknowRole;
}

int DFileSystemModel::roleToColumn(int role) const
{
    Q_D(const DFileSystemModel);

    if (role == FileDisplayNameRole) {
        return 0;
    } else {
        if (!d->rootNode)
            return -1;

//        const AbstractFileInfoPointer &fileInfo = this->fileInfo(d->activeIndex);
        const DAbstractFileInfoPointer &fileInfo = d->rootNode->fileInfo;

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
    Q_D(DFileSystemModel);

    if (d->eventLoop || !d->rootNode)
        return;

    const FileSystemNodePointer &parentNode = getNodeByIndex(parent);

    if (!parentNode || parentNode->populatedChildren)
        return;

    if (d->jobController) {
        disconnect(d->jobController, &JobController::addChildren, this, &DFileSystemModel::onJobAddChildren);
        disconnect(d->jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished);
        disconnect(d->jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildren);

        if (d->jobController->isFinished()) {
            d->jobController->deleteLater();
        } else {
            QEventLoop eventLoop;
            QPointer<DFileSystemModel> me = this;
            d->eventLoop = &eventLoop;

            connect(d->jobController, &JobController::destroyed, &eventLoop, &QEventLoop::quit);

            d->jobController->stopAndDeleteLater();

            int code = eventLoop.exec();

            d->eventLoop = Q_NULLPTR;

            if (code != 0) {
                d->jobController->terminate();
                d->jobController->quit();
                d->jobController.clear();

                return;
            }

            if (!me)
                return;
        }
    }

    d->jobController = fileService->getChildrenJob(parentNode->fileInfo->fileUrl(), QStringList(), d->filters);

    if (!d->jobController)
        return;

    connect(d->jobController, &JobController::addChildren, this, &DFileSystemModel::onJobAddChildren, Qt::DirectConnection);
    connect(d->jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished, Qt::QueuedConnection);
    connect(d->jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildren, Qt::QueuedConnection);

    fileService->addUrlMonitor(parentNode->fileInfo->fileUrl());

    parentNode->populatedChildren = true;

    setState(Busy);

    d->childrenUpdated = false;
    d->jobController->start();
}

Qt::ItemFlags DFileSystemModel::flags(const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    const FileSystemNodePointer &indexNode = getNodeByIndex(index);

    if (!d->passNameFilters(indexNode)) {
        flags &= ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        // ### TODO you shouldn't be able to set this as the current item, task 119433
        return flags & ~ indexNode->fileInfo->fileItemDisableFlags();
    }

    flags |= Qt::ItemIsDragEnabled;

    if ((index.column() == 0)) {
        if (d->readOnly)
            return flags;

        if(indexNode->fileInfo->isWritable()) {
            flags |= Qt::ItemIsEditable;

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
    Q_D(const DFileSystemModel);

    if (d->rootNode) {
        return d->rootNode->fileInfo->supportedDragActions();
    }

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions DFileSystemModel::supportedDropActions() const
{
    Q_D(const DFileSystemModel);

    if (d->rootNode) {
        return d->rootNode->fileInfo->supportedDropActions();
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

    DFMEvent event;

    event << this->parent()->windowId();
    event << toUrl;
    event << urlList;

    switch (action) {
    case Qt::CopyAction:
        fileService->pasteFile(DAbstractFileController::CopyType, toUrl, event);
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        fileService->pasteFile(DAbstractFileController::CutType, toUrl, event);
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
            const DAbstractFileInfoPointer &fileInfo = this->fileInfo(*it);

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
    Q_D(DFileSystemModel);

    // Restore state
    setState(Idle);

    if (d->eventLoop)
        d->eventLoop->exit(1);

    if (d->updateChildrenFuture.isRunning()) {
        d->updateChildrenFuture.cancel();
        d->updateChildrenFuture.waitForFinished();
    }

    if (d->rootNode) {
        const DUrl rootFileUrl = d->rootNode->fileInfo->fileUrl();

        if(fileUrl == rootFileUrl)
            return createIndex(d->rootNode, 0);

        clear();
    }

//    d->rootNode = d->urlToNode.value(fileUrl);

    d->rootNode = createNode(Q_NULLPTR, fileService->createFileInfo(fileUrl));

    return index(fileUrl);
}

DUrl DFileSystemModel::rootUrl() const
{
    Q_D(const DFileSystemModel);

    return d->rootNode ? d->rootNode->fileInfo->fileUrl() : DUrl();
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
    Q_D(DFileSystemModel);

    d->sortRole = role;
    d->srotOrder = order;
}

void DFileSystemModel::setNameFilters(const QStringList &nameFilters)
{
    Q_D(DFileSystemModel);

    if (d->nameFilters == nameFilters)
        return;

    d->nameFilters = nameFilters;

    emitAllDateChanged();
}

void DFileSystemModel::setFilters(QDir::Filters filters)
{
    Q_D(DFileSystemModel);

    if (d->filters == filters)
        return;

    d->filters = filters;

    refresh();
}

//void DFileSystemModel::setActiveIndex(const QModelIndex &index)
//{
//    int old_column_count = columnCount(d->activeIndex);

//    d->activeIndex = index;

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
    Q_D(const DFileSystemModel);

    return d->srotOrder;
}

int DFileSystemModel::sortColumn() const
{
    Q_D(const DFileSystemModel);

    return roleToColumn(d->sortRole);
}

int DFileSystemModel::sortRole() const
{
    Q_D(const DFileSystemModel);

    return d->sortRole;
}

QStringList DFileSystemModel::nameFilters() const
{
    Q_D(const DFileSystemModel);

    return d->nameFilters;
}

QDir::Filters DFileSystemModel::filters() const
{
    Q_D(const DFileSystemModel);

    return d->filters;
}

void DFileSystemModel::sort(int column, Qt::SortOrder order)
{
    Q_D(DFileSystemModel);

    int old_sortRole = d->sortRole;
    int old_sortOrder = d->srotOrder;

    setSortColumn(column, order);

    if(old_sortRole == d->sortRole && old_sortOrder == d->srotOrder) {
        return;
    }

    sort();
}

void DFileSystemModel::sort()
{
    Q_D(const DFileSystemModel);

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

//    const FileSystemNodePointer &node = getNodeByIndex(d->activeIndex);
    const FileSystemNodePointer &node = d->rootNode;

    if(!node)
        return;

//    const DUrl &node_absoluteFileUrl = node->fileInfo->fileUrl();

//    for(const FileSystemNodePointer &url_node : d->urlToNode) {
//        if(node == url_node.constData())
//            continue;

//        url_node->populatedChildren = false;

//        if(node_absoluteFileUrl.toString().startsWith(url_node->fileInfo->fileUrl().toString()))
//            continue;

//        url_node->visibleChildren.clear();
//    }

    QList<DAbstractFileInfoPointer> list;

    list.reserve(node->visibleChildren.size());

    for(const DUrl &fileUrl : node->visibleChildren) {
        list << node->children.value(fileUrl)->fileInfo;
    }

    sort(node->fileInfo, list);

    for(int i = 0; i < node->visibleChildren.count(); ++i) {
        node->visibleChildren[i] = list[i]->fileUrl();
    }

    emitAllDateChanged();
}

const DAbstractFileInfoPointer DFileSystemModel::fileInfo(const QModelIndex &index) const
{
    const FileSystemNodePointer &node = getNodeByIndex(index);
//    if (node && node->fileInfo){
//        node->fileInfo->updateFileInfo();
//    }

    return node ? node->fileInfo : DAbstractFileInfoPointer();
}

const DAbstractFileInfoPointer DFileSystemModel::fileInfo(const DUrl &fileUrl) const
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode)
        return DAbstractFileInfoPointer();

    if (fileUrl == d->rootNode->fileInfo->fileUrl())
        return d->rootNode->fileInfo;

//    const FileSystemNodePointer &node = d->urlToNode.value(fileUrl);
    const FileSystemNodePointer &node = d->rootNode->children.value(fileUrl);

    return node ? node->fileInfo : DAbstractFileInfoPointer();
}

const DAbstractFileInfoPointer DFileSystemModel::parentFileInfo(const QModelIndex &index) const
{
    const FileSystemNodePointer &node = getNodeByIndex(index);

    return node ? node->parent->fileInfo : DAbstractFileInfoPointer();
}

const DAbstractFileInfoPointer DFileSystemModel::parentFileInfo(const DUrl &fileUrl) const
{
    Q_D(const DFileSystemModel);
//    const FileSystemNodePointer &node = d->urlToNode.value(fileUrl);
//    const FileSystemNodePointer &node = d->rootNode->children.value(fileUrl);

//    return node ? node->parent->fileInfo : AbstractFileInfoPointer();
    if (fileUrl == rootUrl())
        return d->rootNode->fileInfo;

    return fileService->createFileInfo(fileUrl.parentUrl(fileUrl));
}

DFileSystemModel::State DFileSystemModel::state() const
{
    Q_D(const DFileSystemModel);

    return d->state;
}

void DFileSystemModel::setReadOnly(bool readOnly)
{
    Q_D(DFileSystemModel);

    d->readOnly = readOnly;
}

bool DFileSystemModel::isReadOnly() const
{
    Q_D(const DFileSystemModel);

    return d->readOnly;
}

void DFileSystemModel::updateChildren(QList<DAbstractFileInfoPointer> list)
{
    Q_D(DFileSystemModel);

    if (qApp->thread() == QThread::currentThread()) {
        if (QThreadPool::globalInstance()->activeThreadCount() >= QThreadPool::globalInstance()->maxThreadCount())
            QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() + 10);

        d->updateChildrenFuture = QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileSystemModel::updateChildren, list);

        return;
    };

    const FileSystemNodePointer &node = d->rootNode;

    if(!node) {
        return;
    }

    QPointer<JobController> job = d->jobController;

    if (job)
        job->pause();

    for (const DUrl url : node->visibleChildren) {
        deleteNodeByUrl(url);
    }

    node->children.clear();
    node->visibleChildren.clear();

    sort(node->fileInfo, list);

    beginInsertRows(createIndex(node, 0), 0, list.count() - 1);

    for(const DAbstractFileInfoPointer &fileInfo : list) {
        if (node->children.contains(fileInfo->fileUrl()))
            continue;

        const FileSystemNodePointer &chileNode = createNode(node.data(), fileInfo);

        node->children[fileInfo->fileUrl()] = chileNode;
        node->visibleChildren << fileInfo->fileUrl();
    }

    endInsertRows();

    if (!d->jobController || d->jobController->isFinished()) {
        setState(Idle);
    } else {
        d->childrenUpdated = true;
    }

    if (job && job->state() == JobController::Paused)
        job->start();
}

void DFileSystemModel::refresh(const DUrl &fileUrl)
{
    Q_D(const DFileSystemModel);
//    const FileSystemNodePointer &node = d->urlToNode.value(fileUrl);
    const FileSystemNodePointer &node = d->rootNode;

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
    Q_D(const DFileSystemModel);

    const QModelIndex &rootIndex = createIndex(d->rootNode, 0);

    emit dataChanged(rootIndex.child(0, 0), rootIndex.child(rootIndex.row() - 1, 0));
}

void DFileSystemModel::toggleHiddenFiles(const DUrl &fileUrl)
{
    Q_D(DFileSystemModel);

    d->filters = ~(d->filters ^ QDir::Filter(~QDir::Hidden));

    refresh(fileUrl);
}


void DFileSystemModel::onFileCreated(const DUrl &fileUrl)
{
    qDebug() << "file creatored" << fileUrl;

    const DAbstractFileInfoPointer &info = fileService->createFileInfo(fileUrl);

    if (!info)
        return;

//    const FileSystemNodePointer &parentNode = d->urlToNode.value(info->parentUrl());
    if (fileUrl == rootUrl()) {
        return refresh();
    }

    if (info->parentUrl() != rootUrl())
        return;

    addFile(info);
}

void DFileSystemModel::onFileDeleted(const DUrl &fileUrl)
{
    Q_D(const DFileSystemModel);
    qDebug() << "file deleted:" << fileUrl;

    const DAbstractFileInfoPointer &info = fileService->createFileInfo(fileUrl);
    if(!info)
        return;

//    const FileSystemNodePointer &parentNode = d->urlToNode.value(info->parentUrl());
    const DUrl &rootUrl = this->rootUrl();

    if (fileUrl == rootUrl) {
        return refresh();
    }

    if (info->parentUrl() != rootUrl)
        return;

    const FileSystemNodePointer &parentNode = d->rootNode;
    if(parentNode && parentNode->populatedChildren) {
        int index = parentNode->visibleChildren.indexOf(fileUrl);
        beginRemoveRows(createIndex(parentNode, 0), index, index);
        parentNode->visibleChildren.removeAt(index);
        parentNode->children.remove(fileUrl);
        endRemoveRows();

//        const FileSystemNodePointer &node = d->urlToNode.value(fileUrl);

//        if(!node)
//            return;

//        if(hasChildren(createIndex(node, 0))) {
//            for(const DUrl &url : d->urlToNode.keys()) {
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
    Q_D(const DFileSystemModel);
//    const FileSystemNodePointer &node = d->urlToNode.value(fileUrl);

    const FileSystemNodePointer &node = d->rootNode;

    if(!node)
        return;

    const QModelIndex &index = this->index(fileUrl);

    if(!index.isValid())
        return;

    emit dataChanged(index, index);
}

const FileSystemNodePointer DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode)
        return FileSystemNodePointer();

    FileSystemNode *indexNode = static_cast<FileSystemNode*>(index.internalPointer());

    if (indexNode == d->rootNode.constData()
            || d->rootNode->children.value(d->rootNode->visibleChildren.value(index.row())).constData() != indexNode) {
        return d->rootNode;
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

void DFileSystemModel::sort(const DAbstractFileInfoPointer &parentInfo, QList<DAbstractFileInfoPointer> &list) const
{
    Q_D(const DFileSystemModel);

    if (!parentInfo)
        return;

    parentInfo->sortByColumnRole(list, d->sortRole, d->srotOrder);
}

const FileSystemNodePointer DFileSystemModel::createNode(FileSystemNode *parent, const DAbstractFileInfoPointer &info)
{
    Q_ASSERT(info);

//    const FileSystemNodePointer &node = d->urlToNode.value(info->fileUrl());

//    if(node) {
//        if(node->fileInfo != info) {
//            node->fileInfo = info;
//        }

//        node->parent = parent;

//        return node;
//    } else {
        FileSystemNodePointer node(new FileSystemNode(parent, info));

//        d->urlToNode[info->fileUrl()] = node;

        return node;
//    }
}

void DFileSystemModel::deleteNode(const FileSystemNodePointer &node)
{
//    d->urlToNode.remove(d->urlToNode.key(node));

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
    //    d->urlToNode.take(url);
}

void DFileSystemModel::clear()
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode)
        return;

    const QModelIndex &index = createIndex(d->rootNode, 0);

    beginRemoveRows(index, 0, d->rootNode->visibleChildren.count() - 1);

    deleteNode((d->rootNode));

    endRemoveRows();
}

void DFileSystemModel::setState(DFileSystemModel::State state)
{
    Q_D(DFileSystemModel);

    if (d->state == state)
        return;

    d->state = state;

    emit stateChanged(state);
}

void DFileSystemModel::onJobAddChildren(const DAbstractFileInfoPointer &fileInfo)
{
    static QMutex mutex;
    static QWaitCondition condition;

    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    timer->moveToThread(qApp->thread());
    timer->setParent(this);
    connect(timer, &QTimer::timeout, this, [this, fileInfo, timer] {
        timer->deleteLater();
        addFile(fileInfo);
        condition.wakeAll();
    });
    timer->metaObject()->invokeMethod(timer, "start", Q_ARG(int, 0));
    mutex.lock();
    condition.wait(&mutex);
    mutex.unlock();
}

void DFileSystemModel::onJobFinished()
{
    Q_D(const DFileSystemModel);

    if (d->childrenUpdated)
        setState(Idle);

    JobController *job = qobject_cast<JobController*>(sender());

    if (job)
        job->deleteLater();
}

void DFileSystemModel::addFile(const DAbstractFileInfoPointer &fileInfo)
{
    Q_D(const DFileSystemModel);

    const FileSystemNodePointer &parentNode = d->rootNode;
    const DUrl &fileUrl = fileInfo->fileUrl();

    if (parentNode && parentNode->populatedChildren && !parentNode->visibleChildren.contains(fileUrl)) {
        QPointer<DFileSystemModel> me = this;

        auto getFileInfoFun =   [&parentNode, &me] (int index)->const DAbstractFileInfoPointer {
                                    qApp->processEvents();

                                    if (!me || index >= parentNode->visibleChildren.count())
                                        return DAbstractFileInfoPointer();

                                    return parentNode->children.value(parentNode->visibleChildren.value(index))->fileInfo;
                                };

        int row = parentNode->fileInfo->getIndexByFileInfo(getFileInfoFun, fileInfo, d->sortRole, d->srotOrder);

        if (!me)
            return;

        if (row == -1)
            row = parentNode->visibleChildren.count();

        beginInsertRows(createIndex(parentNode, 0), row, row);

//        FileSystemNodePointer node = d->urlToNode.value(fileUrl);

//        if(!node) {
            FileSystemNodePointer node = createNode(parentNode.data(), fileInfo);

//            d->urlToNode[fileUrl] = node;
//        }

        parentNode->children[fileUrl] = node;
        parentNode->visibleChildren.insert(row, fileUrl);

        endInsertRows();
    }
}

void DFileSystemModel::emitAllDateChanged()
{
    Q_D(const DFileSystemModel);

    QModelIndex parentIndex = createIndex(d->rootNode, 0);
    QModelIndex topLeftIndex = index(0, 0, parentIndex);
    QModelIndex rightBottomIndex = index(d->rootNode->visibleChildren.count(), columnCount(parentIndex), parentIndex);

    QMetaObject::invokeMethod(this, "dataChanged", Qt::QueuedConnection,
                              Q_ARG(QModelIndex, topLeftIndex), Q_ARG(QModelIndex, rightBottomIndex));
}
