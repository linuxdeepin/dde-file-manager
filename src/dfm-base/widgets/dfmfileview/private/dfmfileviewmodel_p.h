/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DFMFileViewModel_P_H
#define DFMFileViewModel_P_H

#include "widgets/dfmfileview/dfmfileviewmodel.h"
#include "shutil/fileutils.h"
#include "shutil/dfmfilelistfile.h"
#include "base/private/dfmthreadcontainer.hpp"
#include "widgets/dfmfileview/private/dfmtraversaldirthread.h"

#include <QReadWriteLock>
#include <QQueue>

//class DFMFileSystemNode : public QSharedData
//{
//public:
//    DFMFileSystemNode(DFMFileSystemNode *parent,
//                   const DAbstractFileInfoPointer &info,
//                   DFMFileViewModel *DFMFileViewModel,
//                   QReadWriteLock *lock = nullptr);

//    ~DFMFileSystemNode();
//    QVariant dataByRole(int role);
////    void setNodeVisible(const DFMFileSystemNodePointer &node, bool visible);
////    void applyFileFilter(std::shared_ptr<FileFilter> filter);
////    bool shouldHideByFilterRule(std::shared_ptr<FileFilter> filter);
////    void noLockInsertChildren(int index, const QUrl &url, const DFMFileSystemNodePointer &node);
////    void insertChildren(int index, const QUrl &url, const DFMFileSystemNodePointer &node);
////    void noLockAppendChildren(const QUrl &url, const DFMFileSystemNodePointer &node);
////    void appendChildren(const QUrl &url, const DFMFileSystemNodePointer &node);
////    DFMFileSystemNodePointer getNodeByUrl(const QUrl &url);
////    DFMFileSystemNodePointer getNodeByIndex(int index);
////    DFMFileSystemNodePointer takeNodeByUrl(const QUrl &url);
////    DFMFileSystemNodePointer takeNodeByIndex(const int index);
////    int indexOfChild(const DFMFileSystemNodePointer &node);
////    int indexOfChild(const QUrl &url);
////    int childrenCount();
////    QList<DFMFileSystemNodePointer> getChildrenList() const;
////    QList<QUrl> getChildrenUrlList();
////    void setChildrenList(const QList<DFMFileSystemNodePointer> &list);
////    void setChildrenMap(const QHash<QUrl, DFMFileSystemNodePointer> &map);
////    void clearChildren();
////    bool childContains(const QUrl &url);
////    void addDFMFileSystemNode(const DFMFileSystemNodePointer &node);
////    void removeDFMFileSystemNode(const DFMFileSystemNodePointer &node);
////    const DFMFileSystemNodePointer getDFMFileSystemNode(DFMFileSystemNode *parent);
//public:
//    DFMLocalFileInfoPointer fileInfo;
//    DFMFileSystemNode *parent = Q_NULLPTR;
//    bool populatedChildren = false;
//private:
////    QHash<QUrl, DFMFileSystemNodePointer> children;
////    //fix bug 31225,if children clear,another thread useing visibleChildren will crush,so use DFMFileSystemNodePointer
////    QList<DFMFileSystemNodePointer> visibleChildren;
//    DFMFileViewModel *m_DFMFileViewModel = nullptr;
//    QReadWriteLock *rwLock = nullptr;
//};

//template<typename T>
//class LockFreeQueue
//{
//public:
//    struct Node {
//        T data;
//        QAtomicPointer<Node> next;
//    };

//    LockFreeQueue()
//    {
//        m_head.store(new Node());
//        m_head.load()->next.store(nullptr);
//        m_tail.store(m_head.load());
//    }

//    ~LockFreeQueue()
//    {
//        clear();

//        delete m_head.load();
//    }

//    void clear()
//    {
//        while (!isEmpty()) {
//            dequeue();
//        }
//    }

//    bool isEmpty() const
//    {
//        Node *_head = m_head.load();

//        return _head->next == nullptr;
//    }

//    T dequeue()
//    {
//        Node *_head;

//        do {
//            _head = m_head.load();

//            if (_head->next == nullptr) {
//                std::abort();
//            }
//        } while (!m_head.testAndSetAcquire(_head, _head->next.load()));

//        const T &data = _head->next.load()->data;
//        delete _head;

//        return data;
//    }

//    void enqueue(const T &t)
//    {
//        Node *node = new Node();

//        node->data = t;
//        node->next = nullptr;

//        Node *_tail = nullptr;

//        do {
//            _tail = m_tail.load();
//        } while (!_tail->next.testAndSetAcquire(nullptr, node));

//        m_tail = node;
//    }

//    T &head()
//    {
//        return m_head.load();
//    }

//    const T &head() const
//    {
//        return m_head.load();
//    }

//    Node *headNode()
//    {
//        return m_head.load()->next.load();
//    }

//    void removeNode(Node *node)
//    {
//        auto _head = &m_head;

//        do {
//            if (!_head->load()->next.load()) {
//                std::abort();
//            }

//            _head = &_head->load()->next;
//        } while (!_head->testAndSetAcquire(node, node->next.load()));

//        delete node;
//    }

//private:
//    QAtomicPointer<Node> m_head;
//    QAtomicPointer<Node> m_tail;
//};

//class FileNodeManagerThread : public QThread
//{
//public:
//    enum EventType {
//        AddFile, // 在需要排序时会插入到对应位置
//        AppendFile, // 直接放到列表末尾，不管当前列表的顺序，不过文件和文件夹还是分开的
//        RmFile
//    };
//    explicit FileNodeManagerThread(DFMFileViewModel *parent);
//    ~FileNodeManagerThread() override;
//    void start();
//    inline DFMFileViewModel *model() const
//    {
//        return static_cast<DFMFileViewModel *>(parent());
//    }
//    void addFile(const DAbstractFileInfoPointer &info, bool append = false);
//    void removeFile(const DAbstractFileInfoPointer &info);
//    void setRootNode(const DFMFileSystemNodePointer &node);
//    void setEnable(bool enable);
//    void stop();
//private:
//    void run() override;

//private:
//    QTimer *waitTimer;
//    LockFreeQueue<QPair<EventType, DAbstractFileInfoPointer>> fileQueue;
//    DFMFileSystemNodePointer rootNode;
//    QAtomicInteger<bool> enable;
//    QSemaphore semaphore;
//};

class DFMFileViewModelPrivate
{
public:
    enum EventType {
        AddFile,
        RmFile
    };
    explicit DFMFileViewModelPrivate(DFMFileViewModel *qq);
    ~DFMFileViewModelPrivate();

//    DFMFileViewModelPrivate(DFMFileViewModelPrivate &) = delete;
//    DFMFileViewModelPrivate &operator=(DFMFileViewModelPrivate &) = delete;

//    bool passNameFilters(const DFMFileSystemNodePointer &node) const;
//    bool passFileFilters(const DFMLocalFileInfo &info) const;

//    void _q_onFileCreated(const QUrl &fileUrl, bool isPickUpQueue = false);
//    void _q_onFileDeleted(const QUrl &fileUrl);
//    void _q_onFileUpdated(const QUrl &fileUrl);
//    void _q_onFileUpdated(const QUrl &fileUrl, const int &isExternalSource);
//    void _q_onFileRename(const QUrl &from, const QUrl &to);

//    /// add/rm file event
//    void _q_processFileEvent();
//    bool checkFileEventQueue();

    DFMFileViewModel * q_ptr;

////    DFMFileSystemNodePointer rootNode;
//    QReadWriteLock rootNodeRWLock;
//    QReadWriteLock queueWLock;
////    FileNodeManagerThread *rootNodeManager;
//    // 是否支持一列中包含多个元素
//    bool columnCompact = false;

////    QHash<QUrl, DFMFileSystemNodePointer> d->urlToNode;

////    int sortRole = DFMFileViewModel::FileDisplayNameRole;
//    QStringList nameFilters;
//    QDir::Filters filters;
//    Qt::SortOrder srotOrder = Qt::AscendingOrder;
////    QModelIndex d->activeIndex;

////    QPointer<JobController> jobController;
//    QEventLoop *eventLoop = Q_NULLPTR;
//    QFuture<void> updateChildrenFuture;
//    QAtomicInteger<bool> needQuitUpdateChildren;
//    DAbstractFileWatcher *watcher = Q_NULLPTR;
////    std::shared_ptr<FileFilter> advanceSearchFilter;

////    DFMFileViewModel::State state = DFMFileViewModel::Idle;

//    bool childrenUpdated = false;
//    bool readOnly = false;

//    /// add/rm file event
//    std::atomic<bool> _q_processFileEvent_runing;
//    QQueue<QPair<EventType, QUrl>> fileEventQueue;
//    QQueue<QPair<EventType, QUrl>> laterFileEventQueue;

//    bool enabledSort = true;

//    bool beginRemoveRowsFlag = false;
//    QMutex mutex;
//    //防止析构后，添加flags崩溃锁
//    QMutex mutexFlags;
//    QAtomicInteger<bool> currentRemove = false;

//    // 每列包含多个role时，存储此列活跃的role
//    QMap<int, int> columnActiveRole;
//    mutable QMap<QUrl, bool> nameFiltersMatchResultMap;

private:
    DThreadList<QSharedPointer<DFMFileViewItem>> m_childers;
    QSharedPointer<DFMFileViewItem> m_root;
    int m_column = 0;

    DAbstractFileWatcherPointer m_watcher;
    QSharedPointer<DFMTraversalDirThread> m_traversalThread;

    Q_DECLARE_PUBLIC(DFMFileViewModel)
};

#endif // DFMFileViewModel_P_H
