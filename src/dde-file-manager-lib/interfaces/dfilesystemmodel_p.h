#ifndef DFILESYSTEMMODEL_P_H
#define DFILESYSTEMMODEL_P_H

#include "dfilesystemmodel.h"
#include "interfaces/durl.h"
#include "interfaces/dfileviewhelper.h"
#include "shutil/fileutils.h"
#include "deviceinfo/udisklistener.h"
#include "shutil/dfmfilelistfile.h"

#include <QReadWriteLock>
#include <QQueue>

class FileSystemNode : public QSharedData
{
public:
    FileSystemNode(FileSystemNode *parent,
                   const DAbstractFileInfoPointer &info,
                   DFileSystemModel *dFileSystemModel,
                   QReadWriteLock *lock = nullptr);

    ~FileSystemNode();
    QVariant dataByRole(int role);
    void setNodeVisible(const FileSystemNodePointer &node, bool visible);
    void applyFileFilter(std::shared_ptr<FileFilter> filter);
    bool shouldHideByFilterRule(std::shared_ptr<FileFilter> filter);
    void noLockInsertChildren(int index, const DUrl &url, const FileSystemNodePointer &node);
    void insertChildren(int index, const DUrl &url, const FileSystemNodePointer &node);
    void noLockAppendChildren(const DUrl &url, const FileSystemNodePointer &node);
    void appendChildren(const DUrl &url, const FileSystemNodePointer &node);
    FileSystemNodePointer getNodeByUrl(const DUrl &url);
    FileSystemNodePointer getNodeByIndex(int index);
    FileSystemNodePointer takeNodeByUrl(const DUrl &url);
    FileSystemNodePointer takeNodeByIndex(const int index);
    int indexOfChild(const FileSystemNodePointer &node);
    int indexOfChild(const DUrl &url);
    int childrenCount();
    QList<FileSystemNodePointer> getChildrenList() const;
    DUrlList getChildrenUrlList();
    void setChildrenList(const QList<FileSystemNodePointer> &list);
    void setChildrenMap(const QHash<DUrl, FileSystemNodePointer> &map);
    void clearChildren();
    bool childContains(const DUrl &url);
    void addFileSystemNode(const FileSystemNodePointer &node);
    void removeFileSystemNode(const FileSystemNodePointer &node);
    const FileSystemNodePointer getFileSystemNode(FileSystemNode *parent);
public:
    DAbstractFileInfoPointer fileInfo;
    FileSystemNode *parent = Q_NULLPTR;
    bool populatedChildren = false;
private:
    QHash<DUrl, FileSystemNodePointer> children;
    //fix bug 31225,if children clear,another thread useing visibleChildren will crush,so use FileSystemNodePointer
    QList<FileSystemNodePointer> visibleChildren;
    DFileSystemModel *m_dFileSystemModel = nullptr;
    QReadWriteLock *rwLock = nullptr;
};

template<typename T>
class LockFreeQueue
{
public:
    struct Node {
        T data;
        QAtomicPointer<Node> next;
    };

    LockFreeQueue()
    {
        m_head.store(new Node());
        m_head.load()->next.store(nullptr);
        m_tail.store(m_head.load());
    }

    ~LockFreeQueue()
    {
        clear();

        delete m_head.load();
    }

    void clear()
    {
        while (!isEmpty()) {
            dequeue();
        }
    }

    bool isEmpty() const
    {
        Node *_head = m_head.load();

        return _head->next == nullptr;
    }

    T dequeue()
    {
        Node *_head;

        do {
            _head = m_head.load();

            if (_head->next == nullptr) {
                std::abort();
            }
        } while (!m_head.testAndSetAcquire(_head, _head->next.load()));

        const T &data = _head->next.load()->data;
        delete _head;

        return data;
    }

    void enqueue(const T &t)
    {
        Node *node = new Node();

        node->data = t;
        node->next = nullptr;

        Node *_tail = nullptr;

        do {
            _tail = m_tail.load();
        } while (!_tail->next.testAndSetAcquire(nullptr, node));

        m_tail = node;
    }

    T &head()
    {
        return m_head.load();
    }

    const T &head() const
    {
        return m_head.load();
    }

    Node *headNode()
    {
        return m_head.load()->next.load();
    }

    void removeNode(Node *node)
    {
        auto _head = &m_head;

        do {
            if (!_head->load()->next.load()) {
                std::abort();
            }

            _head = &_head->load()->next;
        } while (!_head->testAndSetAcquire(node, node->next.load()));

        delete node;
    }

private:
    QAtomicPointer<Node> m_head;
    QAtomicPointer<Node> m_tail;
};

class FileNodeManagerThread : public QThread
{
public:
    enum EventType {
        AddFile, // 在需要排序时会插入到对应位置
        AppendFile, // 直接放到列表末尾，不管当前列表的顺序，不过文件和文件夹还是分开的
        RmFile
    };
    explicit FileNodeManagerThread(DFileSystemModel *parent);
    ~FileNodeManagerThread() override;
    void start();
    inline DFileSystemModel *model() const
    {
        return static_cast<DFileSystemModel *>(parent());
    }
    void addFile(const DAbstractFileInfoPointer &info, bool append = false);
    void removeFile(const DAbstractFileInfoPointer &info);
    void setRootNode(const FileSystemNodePointer &node);
    void setEnable(bool enable);
    void stop();
private:
    void run() override;

private:
    QTimer *waitTimer;
    LockFreeQueue<QPair<EventType, DAbstractFileInfoPointer>> fileQueue;
    FileSystemNodePointer rootNode;
    QAtomicInteger<bool> enable;
    QSemaphore semaphore;
};

class DFileSystemModelPrivate
{
public:
    enum EventType {
        AddFile,
        RmFile
    };
    explicit DFileSystemModelPrivate(DFileSystemModel *qq);
    ~DFileSystemModelPrivate();

    DFileSystemModelPrivate(DFileSystemModelPrivate &) = delete;
    DFileSystemModelPrivate &operator=(DFileSystemModelPrivate &) = delete;

    bool passNameFilters(const FileSystemNodePointer &node) const;
    bool passFileFilters(const DAbstractFileInfoPointer &info) const;

    void _q_onFileCreated(const DUrl &fileUrl, bool isPickUpQueue = false);
    void _q_onFileDeleted(const DUrl &fileUrl);
    void _q_onFileUpdated(const DUrl &fileUrl);
    void _q_onFileUpdated(const DUrl &fileUrl, const int &isExternalSource);
    void _q_onFileRename(const DUrl &from, const DUrl &to);

    /// add/rm file event
    void _q_processFileEvent();
    bool checkFileEventQueue();

    DFileSystemModel *q_ptr;

    FileSystemNodePointer rootNode;
    QReadWriteLock rootNodeRWLock;
    QReadWriteLock queueWLock;
    FileNodeManagerThread *rootNodeManager;
    // 是否支持一列中包含多个元素
    bool columnCompact = false;

//    QHash<DUrl, FileSystemNodePointer> d->urlToNode;

    int sortRole = DFileSystemModel::FileDisplayNameRole;
    QStringList nameFilters;
    QDir::Filters filters;
    Qt::SortOrder srotOrder = Qt::AscendingOrder;
//    QModelIndex d->activeIndex;

    QPointer<JobController> jobController;
    QEventLoop *eventLoop = Q_NULLPTR;
    QFuture<void> updateChildrenFuture;
    QAtomicInteger<bool> needQuitUpdateChildren;
    DAbstractFileWatcher *watcher = Q_NULLPTR;
    std::shared_ptr<FileFilter> advanceSearchFilter;

    DFileSystemModel::State state = DFileSystemModel::Idle;

    bool childrenUpdated = false;
    bool readOnly = false;

    /// add/rm file event
    std::atomic<bool> _q_processFileEvent_runing;
    QQueue<QPair<EventType, DUrl>> fileEventQueue;
    QQueue<QPair<EventType, DUrl>> laterFileEventQueue;

    bool enabledSort = true;

    bool beginRemoveRowsFlag = false;
    QMutex mutex;
    //防止析构后，添加flags崩溃锁
    QMutex mutexFlags;
    QAtomicInteger<bool> currentRemove = false;

    // 每列包含多个role时，存储此列活跃的role
    QMap<int, int> columnActiveRole;
    mutable QMap<DUrl, bool> nameFiltersMatchResultMap;
    Q_DECLARE_PUBLIC(DFileSystemModel)
};

#endif // DFILESYSTEMMODEL_P_H
