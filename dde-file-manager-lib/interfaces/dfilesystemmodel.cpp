#include "dfilesystemmodel.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "dabstractfilewatcher.h"
#include "dstyleditemdelegate.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "singleton.h"

#include "controllers/jobcontroller.h"
#include "controllers/appcontroller.h"
#include "shutil/desktopfile.h"

#include "interfaces/dfileviewhelper.h"
#include "interfaces/dfmsetting.h"
#include "shutil/fileutils.h"

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
    enum EventType {
        AddFile,
        RmFile
    };

    DFileSystemModelPrivate(DFileSystemModel *qq)
        : q_ptr(qq) {
        if (globalSetting->isShowedHiddenOnView())
            filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
        else
            filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    }

    bool passNameFilters(const FileSystemNodePointer &node) const;
    bool passFileFilters(const DAbstractFileInfoPointer &info) const;

    void _q_onFileCreated(const DUrl &fileUrl);
    void _q_onFileDeleted(const DUrl &fileUrl);
    void _q_onFileUpdated(const DUrl &fileUrl);
    void _q_onFileRename(const DUrl &from, const DUrl &to);

    /// add/rm file event
    void _q_processFileEvent();

    DFileSystemModel *q_ptr;

    FileSystemNodePointer rootNode;

//    QHash<DUrl, FileSystemNodePointer> d->urlToNode;

    int sortRole = DFileSystemModel::FileDisplayNameRole;
    QStringList nameFilters;
    QDir::Filters filters;
    Qt::SortOrder srotOrder = Qt::AscendingOrder;
//    QModelIndex d->activeIndex;

    QPointer<JobController> jobController;
    QEventLoop *eventLoop = Q_NULLPTR;
    QFuture<void> updateChildrenFuture;
    DAbstractFileWatcher *watcher = Q_NULLPTR;

    DFileSystemModel::State state = DFileSystemModel::Idle;

    bool childrenUpdated = false;
    bool readOnly = false;

    /// add/rm file event
    bool _q_processFileEvent_runing = false;
    QQueue<QPair<EventType, DUrl>> fileEventQueue;

    bool enabledSort = true;

    Q_DECLARE_PUBLIC(DFileSystemModel)
};

bool DFileSystemModelPrivate::passNameFilters(const FileSystemNodePointer &node) const
{
    if (nameFilters.isEmpty())
        return true;

    // Check the name regularexpression filters
    if (!(node->fileInfo->isDir() && (filters & (QDir::AllDirs | QDir::Dirs)))) {
        const Qt::CaseSensitivity caseSensitive = (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;

        for (int i = 0; i < nameFilters.size(); ++i) {
            QRegExp re(nameFilters.at(i), caseSensitive, QRegExp::Wildcard);

            if (re.exactMatch(node->fileInfo->fileDisplayName()))
                return true;
        }

        return false;
    }

    return true;
}

bool DFileSystemModelPrivate::passFileFilters(const DAbstractFileInfoPointer &info) const
{
    if (!(filters & QDir::Dirs) && info->isDir())
        return false;

    if (!(filters & QDir::Files) && info->isFile())
        return false;

    if ((filters & QDir::NoSymLinks) && info->isSymLink())
        return false;

    if (!(filters & QDir::Hidden) && info->isHidden())
        return false;

    if ((filters & QDir::Readable) && !info->isReadable())
        return false;

    if ((filters & QDir::Writable) && !info->isWritable())
        return false;

    if ((filters & QDir::Executable) && !info->isExecutable())
        return false;

    return true;
}

void DFileSystemModelPrivate::_q_onFileCreated(const DUrl &fileUrl)
{
    Q_Q(DFileSystemModel);

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(q, fileUrl);

    if (!info || !passFileFilters(info))
        return;

    fileEventQueue.enqueue(qMakePair(AddFile, fileUrl));
    q->metaObject()->invokeMethod(q, QT_STRINGIFY(_q_processFileEvent), Qt::QueuedConnection);
}

void DFileSystemModelPrivate::_q_onFileDeleted(const DUrl &fileUrl)
{
    Q_Q(DFileSystemModel);

    fileEventQueue.enqueue(qMakePair(RmFile, fileUrl));
    q->metaObject()->invokeMethod(q, QT_STRINGIFY(_q_processFileEvent), Qt::QueuedConnection);
}

void DFileSystemModelPrivate::_q_onFileUpdated(const DUrl &fileUrl)
{
    Q_Q(DFileSystemModel);

    const FileSystemNodePointer &node = rootNode;

    if (!node)
        return;

    const QModelIndex &index = q->index(fileUrl);

    if (!index.isValid())
        return;

    if (const DAbstractFileInfoPointer &fileInfo = q->fileInfo(index)) {
        fileInfo->refresh();
    }

    q->parent()->parent()->update(index);
//    emit q->dataChanged(index, index);
}

void DFileSystemModelPrivate::_q_onFileRename(const DUrl &from, const DUrl &to)
{
    _q_onFileDeleted(from);
    _q_onFileCreated(to);
}

void DFileSystemModelPrivate::_q_processFileEvent()
{
    if (_q_processFileEvent_runing)
        return;

    _q_processFileEvent_runing = true;

    Q_Q(DFileSystemModel);

    while (!fileEventQueue.isEmpty()) {
        const QPair<EventType, DUrl> &event = fileEventQueue.dequeue();
        const DUrl &fileUrl = event.second;

        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(q, fileUrl);

        if (!info)
            continue;

        const DUrl &rootUrl = q->rootUrl();

        if (fileUrl == rootUrl) {
            if (event.first == RmFile) {
                emit q->rootUrlDeleted(rootUrl);
            }
            // It must be refreshed when the root url itself is deleted or newly created
            q->refresh();
            continue;
        }

        if (info->parentUrl() != rootUrl)
            continue;

        // Will refreshing the file info meta data
        info->refresh();

        if (event.first == AddFile) {
            q->addFile(info);
            q->selectAndRenameFile(fileUrl);
        } else {// rm file event
            qDebug() << "file deleted:" << fileUrl;

            const FileSystemNodePointer &parentNode = rootNode;

            if (parentNode && parentNode->populatedChildren) {
                int index = parentNode->visibleChildren.indexOf(fileUrl);

                if (index < 0)
                    continue;

                q->beginRemoveRows(q->createIndex(parentNode, 0), index, index);
                parentNode->visibleChildren.removeAt(index);
                parentNode->children.remove(fileUrl);
                q->endRemoveRows();
            }
        }
    }

    _q_processFileEvent_runing = false;
}

DFileSystemModel::DFileSystemModel(DFileViewHelper *parent)
    : QAbstractItemModel(parent)
    , d_ptr(new DFileSystemModelPrivate(this))
{
    qRegisterMetaType<State>(QT_STRINGIFY(State));
    qRegisterMetaType<DAbstractFileInfoPointer>(QT_STRINGIFY(DAbstractFileInfoPointer));
}

DFileSystemModel::~DFileSystemModel()
{
    Q_D(DFileSystemModel);

    if (d->jobController) {
        d->jobController->stopAndDeleteLater();
    }

    if (d->updateChildrenFuture.isRunning()) {
        d->updateChildrenFuture.cancel();
        d->updateChildrenFuture.waitForFinished();
    }

    if (d->watcher) {
        d->watcher->deleteLater();
    }
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

    return indexNode && isDir(indexNode);
}

QVariant DFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    const FileSystemNodePointer &indexNode = getNodeByIndex(index);

    if (!indexNode)
        return QVariant();

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
    case Qt::ToolTipRole: {
        const QList<int> column_role_list = parent()->columnRoleList();

        if (column_role_list.length() < 2)
            break;

        const QPoint &cursor_pos = parent()->parent()->mapFromGlobal(QCursor::pos());
        QStyleOptionViewItem option;

        option.init(parent()->parent());
        parent()->initStyleOption(&option, index);
        option.rect = parent()->parent()->visualRect(index);
        const QList<QRect> &geometrys = parent()->itemDelegate()->paintGeomertys(option, index);

        for (int i = 1; i < geometrys.length() && i <= column_role_list.length(); ++i) {
            const QRect &rect = geometrys.at(i);

            if (rect.left() <= cursor_pos.x() && rect.right() >= cursor_pos.x()) {
                const QString &tooltip = data(index, column_role_list.at(i - 1)).toString();

                if (option.fontMetrics.width(tooltip, -1, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt())) > rect.width())
                    return tooltip;
                else
                    break;
            }
        }

        return QString();
    }
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
        disconnect(d->jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildrenOnNewThread);

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

    d->jobController = fileService->getChildrenJob(this, parentNode->fileInfo->fileUrl(), QStringList(), d->filters);

    if (!d->jobController)
        return;

    connect(d->jobController, &JobController::addChildren, this, &DFileSystemModel::onJobAddChildren, Qt::DirectConnection);
    connect(d->jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished, Qt::QueuedConnection);
    connect(d->jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildrenOnNewThread, Qt::DirectConnection);

    /// make root file to active
    d->rootNode->fileInfo->makeToActive();

    /// start file watcher
    if (d->watcher)
        d->watcher->startWatcher();

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

    if (!indexNode)
        return flags;

    if (!d->passNameFilters(indexNode)) {
        flags &= ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        // ### TODO you shouldn't be able to set this as the current item, task 119433
        return flags & ~ indexNode->fileInfo->fileItemDisableFlags();
    }

    flags |= Qt::ItemIsDragEnabled;

    if ((index.column() == 0)) {
        if (d->readOnly)
            return flags;

        if (indexNode->fileInfo->canRename())
            flags |= Qt::ItemIsEditable;

        if (indexNode->fileInfo->isWritable()) {
            if (indexNode->fileInfo->canDrop())
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

    const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, toUrl);

    if (info->isSymLink()) {
        toUrl = info->rootSymLinkTarget();
    }

    if (DFMGlobal::isTrashDesktopFile(toUrl)) {
         toUrl = DUrl::fromTrashFile("/");
         fileService->moveToTrash(this, urlList);
         return true;
    } else if(DFMGlobal::isComputerDesktopFile(toUrl)) {
         return true;
    } else if (DFMGlobal::isDesktopFile(toUrl)) {
         return FileUtils::openDesktopFileWithParams(toUrl.path(), urlList);
    }

    switch (action) {
    case Qt::CopyAction:
        fileService->pasteFile(this, DFMGlobal::CopyAction, toUrl, urlList);
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        fileService->pasteFile(this, DFMGlobal::CutAction, toUrl, urlList);
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

    return (parentNode->fileInfo->canFetch() || !parentNode->fileInfo->exists()) && !parentNode->populatedChildren;
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

    if (d->watcher) {
        disconnect(d->watcher, 0, this, 0);
        d->watcher->deleteLater();
    }

//    d->rootNode = d->urlToNode.value(fileUrl);

    d->rootNode = createNode(Q_NULLPTR, fileService->createFileInfo(this, fileUrl));
    d->watcher = DFileService::instance()->createFileWatcher(this, fileUrl);

    if (d->watcher) {
        connect(d->watcher, SIGNAL(fileAttributeChanged(DUrl)),
                this, SLOT(_q_onFileUpdated(DUrl)));
        connect(d->watcher, SIGNAL(fileDeleted(DUrl)),
                this, SLOT(_q_onFileDeleted(DUrl)));
        connect(d->watcher, SIGNAL(subfileCreated(DUrl)),
                this, SLOT(_q_onFileCreated(DUrl)));
        connect(d->watcher, SIGNAL(fileMoved(DUrl, DUrl)),
                this, SLOT(_q_onFileRename(DUrl, DUrl)));
        connect(d->watcher, SIGNAL(fileModified(DUrl)),
                this, SLOT(_q_onFileUpdated(DUrl)));
    }

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

    emitAllDataChanged();
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

void DFileSystemModel::setSortOrder(const Qt::SortOrder &order)
{
    Q_D(DFileSystemModel);
    d->srotOrder = order;
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

    if (!enabledSort())
        return;

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

    emitAllDataChanged();
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

    return fileService->createFileInfo(this, fileUrl.parentUrl(fileUrl));
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

DAbstractFileWatcher *DFileSystemModel::fileWatcher() const
{
    Q_D(const DFileSystemModel);

    return d->watcher;
}

bool DFileSystemModel::enabledSort() const
{
    Q_D(const DFileSystemModel);

    return d->enabledSort;
}

void DFileSystemModel::updateChildren(QList<DAbstractFileInfoPointer> list)
{
    Q_D(DFileSystemModel);

    const FileSystemNodePointer &node = d->rootNode;

    if(!node) {
        return;
    }

    QPointer<JobController> job = d->jobController;

    if (job)
        job->pause();

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

void DFileSystemModel::updateChildrenOnNewThread(QList<DAbstractFileInfoPointer> list)
{
    Q_D(DFileSystemModel);

    if (d->jobController)
        d->jobController->pause();

    if (QThreadPool::globalInstance()->activeThreadCount() >= QThreadPool::globalInstance()->maxThreadCount())
        QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() + 10);

    d->updateChildrenFuture = QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileSystemModel::updateChildren, list);
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

    for (const FileSystemNodePointer &node : d->rootNode->children)
        node->fileInfo->refresh();

    emit dataChanged(rootIndex.child(0, 0), rootIndex.child(rootIndex.row() - 1, 0));
}

void DFileSystemModel::toggleHiddenFiles(const DUrl &fileUrl)
{
    Q_D(DFileSystemModel);

    d->filters = ~(d->filters ^ QDir::Filter(~QDir::Hidden));

    refresh(fileUrl);
}

void DFileSystemModel::setEnabledSort(bool enabledSort)
{
    Q_D(DFileSystemModel);

    if (d->enabledSort == enabledSort)
        return;

    d->enabledSort = enabledSort;
    emit enabledSortChanged(enabledSort);
}

const FileSystemNodePointer DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode)
        return FileSystemNodePointer();

    FileSystemNode *indexNode = static_cast<FileSystemNode*>(index.internalPointer());

    if (indexNode == d->rootNode.constData()) {
        return d->rootNode;
    }

    if (enabledSort()) {
        if (d->rootNode->children.value(d->rootNode->visibleChildren.value(index.row())).constData() != indexNode) {
            return FileSystemNodePointer();
        }
    } else if (!d->rootNode->children.key(FileSystemNodePointer(indexNode)).isValid()) {
        return FileSystemNodePointer();
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

    DAbstractFileInfo::CompareFunction sortFun = parentInfo->compareFunByColumn(d->sortRole);

    if (!sortFun)
        return;

    qSort(list.begin(), list.end(), [sortFun, d] (const DAbstractFileInfoPointer &info1, const DAbstractFileInfoPointer &info2) {
        return sortFun(info1, info2, d->srotOrder);
    });
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
    node->fileInfo->makeToInactive();
//    deleteNodeByUrl(node->fileInfo->fileUrl());
}

void DFileSystemModel::clear()
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode)
        return;

    const QModelIndex &index = createIndex(d->rootNode, 0);

    beginRemoveRows(index, 0, d->rootNode->visibleChildren.count() - 1);

    deleteNode(d->rootNode);

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

    QTimer *volatile timer = new QTimer;
    timer->setSingleShot(true);
    timer->moveToThread(qApp->thread());
    timer->setParent(this);
    connect(timer, &QTimer::timeout, this, [this, fileInfo, &timer] {
        timer->deleteLater();
        addFile(fileInfo);
        timer = Q_NULLPTR;
        condition.wakeAll();
    });
    mutex.lock();
    timer->metaObject()->invokeMethod(timer, "start", Q_ARG(int, 0));

    if (timer)
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

    const FileSystemNodePointer parentNode = d->rootNode;
    const DUrl &fileUrl = fileInfo->fileUrl();

    if (parentNode && parentNode->populatedChildren && !parentNode->children.contains(fileUrl)) {
        QPointer<DFileSystemModel> me = this;

        int row = -1;

        if (enabledSort()) {
            row = 0;

            QFuture<void> result;

            if (fileInfo->hasOrderly()) {
                DAbstractFileInfo::CompareFunction compareFun = fileInfo->compareFunByColumn(d->sortRole);

                if (compareFun) {
                    result = QtConcurrent::run(QThreadPool::globalInstance(), [&] {
                        forever {
                            if (!me || row >= parentNode->visibleChildren.count())
                                break;

                            const DUrl &url = parentNode->visibleChildren.value(row);
                            const FileSystemNodePointer &node = parentNode->children.value(url);

                            Q_ASSERT_X(node, "DFileSystemModel::addFile", url.toString().toUtf8().constData());

                            if (compareFun(fileInfo, node->fileInfo, d->srotOrder)) {
                                break;
                            }

                            ++row;
                        }
                    });
                } else {
                    row = -1;
                }
            } else if (fileInfo->isFile()) {
                row = -1;
            } else {
                result = QtConcurrent::run(QThreadPool::globalInstance(), [&] {
                    forever {
                        if (!me || row >= parentNode->visibleChildren.count())
                            break;

                        const DUrl &url = parentNode->visibleChildren.value(row);
                        const FileSystemNodePointer &node = parentNode->children.value(url);

                        Q_ASSERT_X(node, "DFileSystemModel::addFile", url.toString().toUtf8().constData());

                        if (node->fileInfo->isFile()) {
                            break;
                        }

                        ++row;
                    }
                });
            }

            while (!result.isFinished()) {
                qApp->processEvents();
            }
        }

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

void DFileSystemModel::emitAllDataChanged()
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode)
        return;

    QModelIndex parentIndex = createIndex(d->rootNode, 0);
    QModelIndex topLeftIndex = index(0, 0, parentIndex);
    QModelIndex rightBottomIndex = index(d->rootNode->visibleChildren.count(), columnCount(parentIndex), parentIndex);

    QMetaObject::invokeMethod(this, "dataChanged", Qt::QueuedConnection,
                              Q_ARG(QModelIndex, topLeftIndex), Q_ARG(QModelIndex, rightBottomIndex));
}

void DFileSystemModel::selectAndRenameFile(const DUrl &fileUrl)
{
    /// TODO: 暂时放在此处实现，后面将移动到DFileService中实现。
    if (AppController::selectionAndRenameFile.first == fileUrl) {
        quint64 windowId = AppController::selectionAndRenameFile.second;

        if (windowId != parent()->windowId())
            return;

        AppController::selectionAndRenameFile = qMakePair(DUrl(), 0);
        DFMUrlBaseEvent event(this, fileUrl);
        event.setWindowId(windowId);

        TIMER_SINGLESHOT_OBJECT(const_cast<DFileSystemModel*>(this), 100, {
                                    emit fileSignalManager->requestSelectRenameFile(event);
                                }, event)

        emit newFileByInternal(fileUrl);
    }
}

#include "moc_dfilesystemmodel.cpp"
