/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfilesystemmodel.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dabstractfilewatcher.h"
#include "dfmstyleditemdelegate.h"
#include "dfmapplication.h"
#include "views/dfileview.h"
#include "dfmstandardpaths.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "singleton.h"

#include "controllers/vaultcontroller.h"
#include "controllers/jobcontroller.h"
#include "controllers/appcontroller.h"
#include "shutil/desktopfile.h"
//处理自动整理的路径问题
#include "controllers/mergeddesktopcontroller.h"
#include "shutil/dfmfilelistfile.h"
#include "dfilesystemmodel_p.h"
#include "dfmsettings.h"

#include <memory>
#include <QList>
#include <QMap>
#include <QDebug>
#include <QMimeData>
#include <QSharedPointer>
#include <QAbstractItemView>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>

#define fileService DFileService::instance()
#define DEFAULT_COLUMN_COUNT 0

FileSystemNode::FileSystemNode(FileSystemNode *parent,
                   const DAbstractFileInfoPointer &info,
                   DFileSystemModel *dFileSystemModel,
                   QReadWriteLock *lock)
        : fileInfo(info)
        , parent(parent)
        , m_dFileSystemModel(dFileSystemModel)
        , rwLock(lock)
{

}

FileSystemNode::~FileSystemNode()
{
    visibleChildren.clear();
    children.clear();
}

QVariant FileSystemNode::dataByRole(int role)
{
    using Role = DFileSystemModel::Roles;

    switch (role)
    {
    case Role::FilePathRole:
        return fileInfo->absoluteFilePath();
    case Role::FileDisplayNameRole:
        return fileInfo->fileDisplayName();
    case Role::FileNameRole:
        return fileInfo->fileName();
    case Role::FileNameOfRenameRole:
        return fileInfo->fileNameOfRename();
    case Role::FileBaseNameRole:
        return fileInfo->baseName();
    case Role::FileBaseNameOfRenameRole:
        return fileInfo->baseNameOfRename();
    case Role::FileSuffixRole:
        return fileInfo->suffix();
    case Role::FileSuffixOfRenameRole:
        return fileInfo->suffixOfRename();
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter;
    case Role::FileLastModifiedRole:
        return fileInfo->lastModifiedDisplayName();
    case Role::FileLastModifiedDateTimeRole:
        return fileInfo->lastModified();
    case Role::FileSizeRole:
        return fileInfo->sizeDisplayName();
    case Role::FileSizeInKiloByteRole:
        return fileInfo->size();
    case Role::FileMimeTypeRole:
        return fileInfo->mimeTypeDisplayName();
    case Role::FileCreatedRole:
        return fileInfo->createdDisplayName();
    case Role::FilePinyinName:
        return fileInfo->fileDisplayPinyinName();
    case Role::ExtraProperties:
        return fileInfo->extraProperties();
    case Role::FileLastReadDateTimeRole:
        return fileInfo->lastRead();
    case Role::FileCreatedDateTimeRole:
        return fileInfo->created();
    default: {
        return QVariant();
        }
    }
}

void FileSystemNode::setNodeVisible(const FileSystemNodePointer &node, bool visible)
{
    if (visible) {
        if (!visibleChildren.contains(node)) {
            visibleChildren.append(node);
        }
    } else {
        if (visibleChildren.contains(node)) {
            visibleChildren.removeOne(node);
        }
    }
}

void FileSystemNode::applyFileFilter(std::shared_ptr<FileFilter> filter)
{
    if (!filter) return;

    // fix bug 54887 【专业版1030】【文管5.2.0.82】搜索结果页多次筛选后，再重置筛选结果，重置搜索界面页面，出现重复搜索界面
    // 点击重置按钮，会触发所有combox的change信号，都会访问visibleChildren资源，因此加锁限制
    rwLock->lockForWrite();
    visibleChildren.clear();

    for (auto node : children) {
        if (!node->shouldHideByFilterRule(filter)) {
            visibleChildren.append(node);
        }
    }
    rwLock->unlock();
}

bool FileSystemNode::shouldHideByFilterRule(std::shared_ptr<FileFilter> filter)
{
    if (!filter) return false;

    if (filter->f_comboValid[SEARCH_RANGE] && !filter->f_includeSubDir) {
        DUrl parentUrl = fileInfo->parentUrl().isSearchFile() ? fileInfo->parentUrl().searchTargetUrl() : fileInfo->parentUrl();
        QString filePath = dataByRole(DFileSystemModel::FilePathRole).toString();
        // fix bug 44185 【专业版 sp3】【文件管理器】【5.2.0.28-1】多标签操作筛选搜索结果，回退路径时出现空白页面
        filePath.remove(parentUrl.toLocalFile().endsWith("/") ?  parentUrl.toLocalFile() : parentUrl.toLocalFile() + '/');
        if (filePath.contains('/')) return true;
    }

    if (filter->f_comboValid[FILE_TYPE]) {
        QString fileTypeStr = dataByRole(DFileSystemModel::FileMimeTypeRole).toString();
        if (!fileTypeStr.startsWith(filter->f_typeString)) return true;
    }

    if (filter->f_comboValid[SIZE_RANGE]) {
        // note: FileSizeInKiloByteRole is the size of Byte, not KB!
        quint64 fileSize = dataByRole(DFileSystemModel::FileSizeInKiloByteRole).toULongLong();
        quint32 blockSize = 1 << 10;
        quint64 lower = filter->f_sizeRange.first * blockSize;
        quint64 upper = filter->f_sizeRange.second * blockSize;
        // filter file size in Bytes, not Kilobytes
        if (fileSize < lower || fileSize > upper) return true;
    }

    if (filter->f_comboValid[DATE_RANGE]) {
        QDateTime filemtime = dataByRole(DFileSystemModel::FileLastModifiedDateTimeRole).toDateTime();
        if (filemtime < filter->f_dateRangeStart || filemtime > filter->f_dateRangeEnd) return true;
    }

    if (filter->f_comboValid[ACCESS_DATE_RANGE]) {
        QDateTime filemtime = dataByRole(DFileSystemModel::FileLastReadDateTimeRole).toDateTime();
        if (filemtime < filter->f_accessDateRangeStart || filemtime > filter->f_accessDateRangeEnd) return true;
    }

    if (filter->f_comboValid[CREATE_DATE_RANGE]) {
        QDateTime filemtime = dataByRole(DFileSystemModel::FileCreatedDateTimeRole).toDateTime();
        if (filemtime < filter->f_createDateRangeStart || filemtime > filter->f_createDateRangeEnd) return true;
    }

    return false;
}

void FileSystemNode::noLockInsertChildren(int index, const DUrl &url, const FileSystemNodePointer &node)
{
    // fix bug 105595
    if (!children.contains(url)) {
        children[url] = node;
        visibleChildren.insert(index, node);
    }
}

void FileSystemNode::insertChildren(int index, const DUrl &url, const FileSystemNodePointer &node)
{
    rwLock->lockForWrite();
    noLockInsertChildren(index, url, node);
    rwLock->unlock();
}

void FileSystemNode::noLockAppendChildren(const DUrl &url, const FileSystemNodePointer &node)
{
    // fix bug 105595
    if (!children.contains(url)) {
        children[url] = node;
        visibleChildren.append(node);
    }
}

void FileSystemNode::appendChildren(const DUrl &url, const FileSystemNodePointer &node)
{
    rwLock->lockForWrite();
    noLockAppendChildren(url, node);
    rwLock->unlock();
}

FileSystemNodePointer FileSystemNode::getNodeByUrl(const DUrl &url)
{
    rwLock->lockForRead();
    FileSystemNodePointer node = children.value(url);
    rwLock->unlock();

    return node;
}

FileSystemNodePointer FileSystemNode::getNodeByIndex(int index)
{
    rwLock->lockForRead();
    FileSystemNodePointer node = visibleChildren.value(index);
    rwLock->unlock();
    return node;
}

FileSystemNodePointer FileSystemNode::takeNodeByUrl(const DUrl &url)
{
    rwLock->lockForWrite();
    FileSystemNodePointer node = children.take(url);
    visibleChildren.removeOne(node);
    rwLock->unlock();

    return node;
}

FileSystemNodePointer FileSystemNode::takeNodeByIndex(const int index)
{
    rwLock->lockForWrite();
    FileSystemNodePointer node;
    if (index >= 0 && visibleChildren.size() > index) {
        node = visibleChildren.takeAt(index);
        children.remove(node->fileInfo->fileUrl());
    } else {
        qWarning() << "index [" << index << "] out of range [" << visibleChildren.size() << "]";
    }
    rwLock->unlock();

    return node;
}

int FileSystemNode::indexOfChild(const FileSystemNodePointer &node)
{
    rwLock->lockForRead();
    int index = visibleChildren.indexOf(node);
    rwLock->unlock();

    return index;
}

int FileSystemNode::indexOfChild(const DUrl &url)
{
    rwLock->lockForRead();
    const FileSystemNodePointer &node = children.value(url);
    int index = visibleChildren.indexOf(node);
    rwLock->unlock();

    return index;
}

int FileSystemNode::childrenCount()
{
    QReadLocker rl(rwLock);

    return visibleChildren.count();
}

QList<FileSystemNodePointer> FileSystemNode::getChildrenList() const
{
    return visibleChildren;
}

DUrlList FileSystemNode::getChildrenUrlList()
{
    DUrlList list;

    rwLock->lockForRead();

    list.reserve(visibleChildren.size());

    for (const FileSystemNodePointer &node : visibleChildren)
        list << node->fileInfo->fileUrl();

    rwLock->unlock();

    return list;
}

void FileSystemNode::setChildrenList(const QList<FileSystemNodePointer> &list)
{
    rwLock->lockForWrite();
    visibleChildren = list;

    rwLock->unlock();
}

void FileSystemNode::setChildrenMap(const QHash<DUrl, FileSystemNodePointer> &map)
{
    rwLock->lockForWrite();
    children = map;
    rwLock->unlock();
}

void FileSystemNode::clearChildren()
{
    rwLock->lockForWrite();
    visibleChildren.clear();
    children.clear();
    rwLock->unlock();
}

bool FileSystemNode::childContains(const DUrl &url)
{
    QReadLocker rl(rwLock);

    return children.contains(url);
}


void FileSystemNode::addFileSystemNode(const FileSystemNodePointer &node)
{
    if (nullptr != node->parent) {
        QString url = node->fileInfo->filePath();
        rwLock->lockForWrite();
        if (!m_dFileSystemModel->m_allFileSystemNodes.contains(url)) {
            m_dFileSystemModel->m_allFileSystemNodes[url] = node;
        }
        rwLock->unlock();
    }
}

void FileSystemNode::removeFileSystemNode(const FileSystemNodePointer &node)
{
    if (nullptr != node->parent) {
        QString url = node->fileInfo->filePath();
        rwLock->lockForWrite();
        qDebug() << m_dFileSystemModel->m_allFileSystemNodes[url];
        m_dFileSystemModel->m_allFileSystemNodes.remove(url);
        rwLock->unlock();
    }
}

const FileSystemNodePointer FileSystemNode::getFileSystemNode(FileSystemNode *parent)
{
    if (nullptr == parent) {
        return FileSystemNodePointer();
    }

//        return FileSystemNodePointer(parent);

//        qDebug() << "start check map!";
//        qDebug() << "Count = " << QString::number(m_dFileSystemModel->m_allFileSystemNodes.count());
//        for (FileSystemNodePointer pointer : m_dFileSystemModel->m_allFileSystemNodes)
//        {
//            auto key = m_dFileSystemModel->m_allFileSystemNodes.key(pointer);
//            qDebug() << "value = " << pointer.data();
//            qDebug() << "key = " << key;
//        }


//        qDebug() << "start check list!";
//        QList<FileSystemNode *> keyList = m_dFileSystemModel->m_allFileSystemNodes.uniqueKeys();
//        qDebug() << "Count = " << QString::number(keyList.count());
//        for (FileSystemNode *t_node : keyList)
//        {
//            qDebug() << "key = " << t_node;
//        }

    QString url = parent->fileInfo->filePath();

    rwLock->lockForWrite();
    if (!m_dFileSystemModel->m_allFileSystemNodes.contains(url)) {
        FileSystemNodePointer tmpNode(parent);
        m_dFileSystemModel->m_allFileSystemNodes[url] = tmpNode;
        rwLock->unlock();
        return tmpNode;
    }

    FileSystemNodePointer tmpNode1(m_dFileSystemModel->m_allFileSystemNodes[url]);
    rwLock->unlock();
    return tmpNode1;
}



FileNodeManagerThread::FileNodeManagerThread(DFileSystemModel *parent)
    : QThread(parent)
    , waitTimer(new QTimer(this))
    , enable(true)
{
    waitTimer->setSingleShot(true);
    waitTimer->setInterval(50);

    connect(waitTimer, &QTimer::timeout, this, &FileNodeManagerThread::start);
}

FileNodeManagerThread::~FileNodeManagerThread()
{
    stop();
}

void FileNodeManagerThread::start()
{
    if (fileQueue.isEmpty())
        return;

    QThread::start();
}

void FileNodeManagerThread::addFile(const DAbstractFileInfoPointer &info, bool append)
{
    if (!enable)
        return;

    fileQueue.enqueue(qMakePair(append ? AppendFile : AddFile, info));

    if (!isRunning()) {
        if (!waitTimer->isActive()) {
            // 确保从第一个文件操作开始，处理此文件操作做多不应超过1s
            QTimer::singleShot(1000, this, &FileNodeManagerThread::start);
        }

        waitTimer->start();
    }
}

void FileNodeManagerThread::removeFile(const DAbstractFileInfoPointer &info)
{
    if (!enable)
        return;

    fileQueue.enqueue(qMakePair(RmFile, info));

    if (!isRunning()) {
        if (!waitTimer->isActive()) {
            // 确保从第一个文件操作开始，处理此文件操作做多不应超过1s
            QTimer::singleShot(1000, this, &FileNodeManagerThread::start);
        }

        waitTimer->start();
    }
}

void FileNodeManagerThread::setRootNode(const FileSystemNodePointer &node)
{
    rootNode = node;
}

void FileNodeManagerThread::setEnable(bool enable)
{
    this->enable = enable;
}

void FileNodeManagerThread::stop()
{
    enable = false;
    // 确保在timer的所在线程停止它
    waitTimer->metaObject()->invokeMethod(waitTimer, "stop");
    // 取消工作线程的等待，防止产生死锁
    semaphore.release();
    wait();

    // 消除释放出的多余信号量
    if (semaphore.available() == 1)
        semaphore.acquire();

    fileQueue.clear();
}

void FileNodeManagerThread::run()
{
    // 缓存需要批量插入的文件信息列表
    QList<DAbstractFileInfoPointer> backlogFileInfoList;
    QList<DAbstractFileInfoPointer> backlogDirInfoList;
    // 使用计时器避免文件在批量插入列表中等待太久
    QTime timerOfFileList, timerOfDirList;

    auto insertInfoList = [&](int index, const QList<DAbstractFileInfoPointer> &list) {
        DThreadUtil::runInThread(&semaphore, model()->thread(), model(), &DFileSystemModel::beginInsertRows,
                                 model()->createIndex(rootNode, 0), index, index + list.count() - 1);

        if (!enable)
            return false;

        for (const DAbstractFileInfoPointer &fileInfo : list) {
            if (!enable) {
                return false;
            }

            FileSystemNodePointer node = model()->createNode(rootNode.data(), fileInfo);
            rootNode->insertChildren(index++, fileInfo->fileUrl(), node);
            if (node->shouldHideByFilterRule(model()->advanceSearchFilter())) {
                rootNode->setNodeVisible(node, false);
            }
        }

        DThreadUtil::runInThread(&semaphore, model()->thread(), model(), &DFileSystemModel::endInsertRows);

        return enable.load();
    };

    auto disposeBacklogFileList = [&] {
        if (backlogFileInfoList.isEmpty())
        {
            return true;
        }

        int row = rootNode->childrenCount();

        if (!insertInfoList(row, backlogFileInfoList))
            return false;

        backlogFileInfoList.clear();

        return true;
    };

    auto disposeBacklogDirList = [&] {
        if (backlogDirInfoList.isEmpty())
        {
            return true;
        }

        int row = 0;

        forever
        {
            if (!enable) {
                return false;
            }

            if (row >= rootNode->childrenCount()) {
                break;
            }

            const FileSystemNodePointer &node = rootNode->getNodeByIndex(row);

            //因在自动整理时，超时会在次判定，导致文件夹以及分类文件夹会出现在扩展分类之前，
            //所以加一个node->fileInfo->fileUrl().scheme() != DFMMD_SCHEME规避掉
            if (node->fileInfo->isFile() && node->fileInfo->fileUrl().scheme() != DFMMD_SCHEME) {
                break;
            }

            ++row;
        }

        if (!insertInfoList(row, backlogDirInfoList))
            return false;

        backlogDirInfoList.clear();

        return true;
    };

    auto removeInList = [&](QList<DAbstractFileInfoPointer> &list, const DUrl & url) {
        for (int i = 0; i < list.count(); ++i) {
            if (list.at(i)->fileUrl() == url) {
                list.removeAt(i);

                return true;
            }
        }

        return false;
    };

begin:

    while (!fileQueue.isEmpty()) {
        if (!enable) {
            return;
        }

        const QPair<EventType, DAbstractFileInfoPointer> &v = fileQueue.dequeue();
        const DAbstractFileInfoPointer &fileInfo = v.second;
        if (!fileInfo) {
            continue;
        }
        const DUrl &fileUrl = fileInfo->fileUrl();

        if (v.first == AddFile || v.first == AppendFile) {
            if (rootNode->childContains(fileUrl))
                continue;

            int row = -1;

            if (model()->enabledSort()) {
                row = 0;

                if (fileInfo->hasOrderly() && v.first == AddFile) {
                    DAbstractFileInfo::CompareFunction compareFun = fileInfo->compareFunByColumn(model()->sortRole());

                    if (compareFun) {
                        forever {
                            if (!enable) {
                                return;
                            }

                            if (row >= rootNode->childrenCount()) {
                                break;
                            }

                            const FileSystemNodePointer &node = rootNode->getNodeByIndex(row);
                            if (node) {
                                if (compareFun(fileInfo, node->fileInfo, model()->sortOrder())) {
                                    break;
                                }
                            }

                            ++row;
                        }
                    } else {
                        row = -1;
                    }
                } else {
                    row = -1;
                }
            }

            if (row < 0) {
                bool isFile = fileInfo->isFile();

                // 先加到待插入列表
                if (isFile) {
                    if (backlogFileInfoList.isEmpty()) {
                        timerOfFileList.start();
                    } else if (timerOfFileList.elapsed() > 1000) {
                        disposeBacklogFileList();
                        timerOfFileList.start();
                    }

                    backlogFileInfoList << fileInfo;
                } else {
                    if (backlogDirInfoList.isEmpty()) {
                        timerOfDirList.start();
                    } else if (timerOfDirList.elapsed() > 1000) {
                        disposeBacklogDirList();
                        timerOfDirList.start();
                    }

                    backlogDirInfoList << fileInfo;
                }
            } else {
                if (!enable) {
                    return;
                }

                DThreadUtil::runInThread(&semaphore, model()->thread(), model(), &DFileSystemModel::beginInsertRows,
                                         model()->createIndex(rootNode, 0), row, row);

                if (!enable) {
                    return;
                }

                FileSystemNodePointer node = model()->createNode(rootNode.data(), fileInfo);
                rootNode->insertChildren(row, fileUrl, node);

                DThreadUtil::runInThread(&semaphore, model()->thread(), model(), &DFileSystemModel::endInsertRows);
            }
        } else {
            // 先尝试从待插入列表中删除
            if (fileInfo->isFile()) {
                if (removeInList(backlogFileInfoList, fileUrl)) {
                    continue;
                }
            } else if (removeInList(backlogDirInfoList, fileUrl)) {
                continue;
            }

            int row = rootNode->indexOfChild(fileUrl);

            if (!enable) {
                return;
            }

            if (row < 0) {
                continue;
            }

            if (DThreadUtil::runInThread(&semaphore, model()->thread(), model(), &DFileSystemModel::beginRemoveRows,
                                         model()->createIndex(rootNode, 0), row, row)) {
                if (!enable) {
                    return;
                }

                Q_UNUSED(rootNode->takeNodeByIndex(row));
                DThreadUtil::runInThread(&semaphore, model()->thread(), model(), &DFileSystemModel::endRemoveRows);
            }
        }
    }

    // 退出前确保所有文件都被处理
    disposeBacklogFileList();
    disposeBacklogDirList();

    if (!enable) {
        return;
    }

    // 先等待一秒看是否还有数据
    QThread::msleep(300);

    if (!enable) {
        return;
    }

    if (!fileQueue.isEmpty()) {
        goto begin;
    }
}

DFileSystemModelPrivate::DFileSystemModelPrivate(DFileSystemModel *qq)
    : q_ptr(qq)
    , rootNodeManager(new FileNodeManagerThread(qq))
    , needQuitUpdateChildren(false)
{
    _q_processFileEvent_runing.store(false);
    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedHiddenFiles).toBool()) {
        filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
    } else {
        filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    }

    columnCompact = DFMApplication::instance()->appAttribute(DFMApplication::AA_ViewComppactMode).toBool();

    qq->connect(rootNodeManager, &FileNodeManagerThread::finished, qq, [this, qq] {
        // 在此线程结束时判断是否需要将model的状态设置为空闲
        if (!jobController || !jobController->isRunning())
        {
            qq->setState(DFileSystemModel::Idle);
        }

        //当遍历文件的耗时超过JobController::m_timeCeiling时，
        //onJobFinished函数中拿到的文件不足，因为rootNodeManager还要处理剩余文件
        //因此在这里rootNodeManager处理完后，再次发送信号 关联bug#24863
        emit qq->sigJobFinished();
    });
}

DFileSystemModelPrivate::~DFileSystemModelPrivate()
{
    if (_q_processFileEvent_runing.load()) {
        fileEventQueue.clear();
    }
}

bool DFileSystemModelPrivate::passNameFilters(const FileSystemNodePointer &node) const
{
    if (nameFilters.isEmpty())
        return true;
 
    if (!node || !node->fileInfo)
        return true;

    // 大量的过滤规则场景时，框选会出现卡顿，在性能较差的平台上较明显
    const DUrl fileUrl = node->fileInfo->fileUrl();
    if (nameFiltersMatchResultMap.contains(fileUrl))
        return nameFiltersMatchResultMap.value(fileUrl, false);

    // Check the name regularexpression filters
    if (!(node->fileInfo->isDir() && (filters & QDir::Dirs))) {
        const Qt::CaseSensitivity caseSensitive = (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
        const QString &fileDisplayName = node->fileInfo->fileDisplayName();
        QRegExp re("", caseSensitive, QRegExp::Wildcard);

        for (int i = 0; i < nameFilters.size(); ++i) {
            re.setPattern(nameFilters.at(i));
            if (re.exactMatch(fileDisplayName)) {
                nameFiltersMatchResultMap[fileUrl] = true;
                return true;
            }
        }

        nameFiltersMatchResultMap[fileUrl] = false;
        return false;
    }

    nameFiltersMatchResultMap[fileUrl] = true;
    return true;
}

bool DFileSystemModelPrivate::passFileFilters(const DAbstractFileInfoPointer &info) const
{
    if (!(filters & (QDir::Dirs | QDir::AllDirs)) && info->isDir()) {
        return false;
    }

    if (!(filters & QDir::Files) && info->isFile()) {
        return false;
    }

    if ((filters & QDir::NoSymLinks) && info->isSymLink()) {
        return false;
    }

    if (!(filters & QDir::Hidden) && info->isHidden()) {
        return false;
    }

    if ((filters & QDir::Readable) && !info->isReadable()) {
        return false;
    }

    if ((filters & QDir::Writable) && !info->isWritable()) {
        return false;
    }

    if ((filters & QDir::Executable) && !info->isExecutable()) {
        return false;
    }

    return !info->isPrivate();
}

void DFileSystemModelPrivate::_q_onFileCreated(const DUrl &fileUrl, bool isPickUpQueue)
{
    Q_Q(DFileSystemModel);

    DAbstractFileInfoPointer info = DAbstractFileInfo::getFileInfo(fileUrl);
    if (info) {
        info->refresh(info->isGvfsMountFile());
    } else {
        info = DFileService::instance()->createFileInfo(q, fileUrl);
    }

    if ((!info || !passFileFilters(info)) && !isPickUpQueue) {
        return;
    }
//    rootNodeManager->addFile(info);
    mutex.lock();
    fileEventQueue.enqueue(qMakePair(AddFile, fileUrl));
    mutex.unlock();
    q->metaObject()->invokeMethod(q, QT_STRINGIFY(_q_processFileEvent), Qt::QueuedConnection);

//    if (!_q_processFileEvent_runing.load()) {
//        queueWLock.lockForWrite();
//        while (!laterFileEventQueue.isEmpty()) {
//            fileEventQueue.enqueue(laterFileEventQueue.dequeue());
//        }
//        fileEventQueue.enqueue(qMakePair(AddFile, fileUrl));
//        queueWLock.unlock();
//        q->metaObject()->invokeMethod(q, QT_STRINGIFY(_q_processFileEvent), Qt::QueuedConnection);
//    } else {
//        queueWLock.lockForWrite();
//        laterFileEventQueue.enqueue(qMakePair(AddFile, fileUrl));
//        queueWLock.unlock();
//    }
}

void DFileSystemModelPrivate::_q_onFileDeleted(const DUrl &fileUrl)
{
    Q_Q(DFileSystemModel);

//    rootNodeManager->removeFile(DFileService::instance()->createFileInfo(q, fileUrl));
    //当文件删除时，删除隐藏文件集中的隐藏
    QString absort = fileUrl.path().left(fileUrl.path().length() - fileUrl.fileName().length());
    DFMFileListFile flf(absort);
    if (flf.contains(fileUrl.fileName())) {
        flf.remove(fileUrl.fileName());
        flf.save();
    }

    mutex.lock();
    fileEventQueue.enqueue(qMakePair(RmFile, fileUrl));
    mutex.unlock();
    q->metaObject()->invokeMethod(q, QT_STRINGIFY(_q_processFileEvent), Qt::QueuedConnection);
//    if (!_q_processFileEvent_runing.load()) {
//        while (!laterFileEventQueue.isEmpty()) {
//            fileEventQueue.enqueue(laterFileEventQueue.dequeue());
//        }
//        fileEventQueue.enqueue(qMakePair(RmFile, fileUrl));
//        q->metaObject()->invokeMethod(q, QT_STRINGIFY(_q_processFileEvent), Qt::QueuedConnection);
//    } else {
//        laterFileEventQueue.enqueue(qMakePair(RmFile, fileUrl));
//    }
}

void DFileSystemModelPrivate::_q_onFileUpdated(const DUrl &fileUrl)
{
    Q_Q(DFileSystemModel);

    //fix 31327， 监控./.hidden文件更改
    //bug#30019 task#40201 补充
    if ((fileUrl.fileName() == ".hidden") && !(q->filters() & QDir::Hidden)) {
        q->refresh();
    }

    const FileSystemNodePointer &node = rootNode;

    if (!node) {
        return;
    }

    const QModelIndex &index = q->index(fileUrl);

    if (!index.isValid()) {
        return;
    }

    if (const DAbstractFileInfoPointer &fileInfo = q->fileInfo(index)) {
        fileInfo->refresh(true);
    }

    q->parent()->parent()->update(index);
//    emit q->dataChanged(index, index);
    //recentfile变更需要调整排序
    if (fileUrl.isRecentFile()) {
        //fileinfo已在recentcontroller中更新，只需要重排序
        q->sort();
    }
}

void DFileSystemModelPrivate::_q_onFileUpdated(const DUrl &fileUrl, const int &isExternalSource)
{
    Q_Q(DFileSystemModel);

    const FileSystemNodePointer &node = rootNode;
    //fix 27828 文件属性改变刷新一次缓存数据,这里只是刷缓存数据
    if (!node) {
        return;
    }

    const QModelIndex &index = q->index(fileUrl);

    if (!index.isValid()) {
        return;
    }

    if (const DAbstractFileInfoPointer &fileInfo = q->fileInfo(index)) {
        if (isExternalSource) {
            fileInfo->refresh(fileInfo->isGvfsMountFile());
        }
    }

    q->parent()->parent()->update(index);
//    emit q->dataChanged(index, index);
}

void DFileSystemModelPrivate::_q_onFileRename(const DUrl &from, const DUrl &to)
{
    Q_Q(DFileSystemModel);

    //如果被重命名的目录是root目录，则不刷新该目录,而是直接退回到上层目录
    bool isLocalFile = from.isLocalFile() || (from.isVaultFile() || to.isVaultFile());
    if (isLocalFile && from.path() == rootNode->dataByRole(DFileSystemModel::Roles::FilePathRole).toString()) {
        QString trashPath = DFMStandardPaths::location(DFMStandardPaths::TrashPath);
        bool isMoveToTrash = to.toLocalFile().contains(trashPath);
        if (!isMoveToTrash && !to.toLocalFile().isEmpty()) {
            // re-enter directory if tab root directory renamed.
            DFileView *fileview = static_cast<DFileView *>(q->parent()->parent());
            fileSignalManager->requestRedirectTabUrl(fileview->rootUrl(), to);
        }

        return;
    }
    _q_onFileDeleted(from);
    _q_onFileCreated(to);
}

void DFileSystemModelPrivate::_q_processFileEvent()
{
    Q_Q(DFileSystemModel);
    //处理异步正在执行此函数，但是当前类释放了
    QPointer<DFileSystemModel> me = q;
    if (_q_processFileEvent_runing.load()) {
        return;
    }

    // CAS
    bool expect = false;
    _q_processFileEvent_runing.compare_exchange_strong(expect, true);

    while (checkFileEventQueue()) {
        if (!q->isDesktop)  //紧急修复，由于修复bug#33209添加了一次事件循环的处理，导致桌面的自动排列在删除，恢复文件时显示异常
            qApp->processEvents();

        if (me.isNull()) { // 当前窗口被关闭以后，me 指针指向的窗口会马上被析构，后面的流程不需要再走了
            return;
        }
        mutex.lock();
        const QPair<EventType, DUrl> &event = fileEventQueue.dequeue();
        mutex.unlock();
        const DUrl &fileUrl = event.second;
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(q, fileUrl);

        if (!info) {
            continue;
        }
        if (event.first != AddFile) {
            info->refresh(info->isGvfsMountFile());
        }
        const DUrl &rootUrl = q->rootUrl();
        const DAbstractFileInfoPointer rootinfo = fileService->createFileInfo(q, rootUrl);
        DUrl nparentUrl(info->parentUrl());
        DUrl nfileUrl(fileUrl);

        if (rootUrl.scheme() == BURN_SCHEME) {
            QRegularExpression burn_rxp("^(.*?)/(" BURN_SEG_ONDISC "|" BURN_SEG_STAGING ")(.*)$");
            QString rxp_after(QString("\\1/%1\\3").arg(rootUrl.burnIsOnDisc() ? BURN_SEG_ONDISC : BURN_SEG_STAGING));
            nfileUrl.setPath(nfileUrl.path().replace(burn_rxp, rxp_after));
            nparentUrl.setPath(nparentUrl.path().replace(burn_rxp, rxp_after));
            if (!nparentUrl.path().endsWith('/') && rootUrl.path().endsWith("/")) {
                nparentUrl.setPath(nparentUrl.path() + "/");
            }
        }

        if (nfileUrl == rootUrl) {
            if (event.first == RmFile) {
                //! close tab if root url deleted.
                emit fileSignalManager->requestCloseTab(nfileUrl);
                //! return to parent.
                emit q->rootUrlDeleted(rootUrl);
            }

            // It must be refreshed when the root url itself is deleted or newly created
            q->refresh();
            continue;
        }
        if (nparentUrl != rootUrl) {
            continue;
        }
        // Will refreshing the file info meta data
        info->refresh();
        if (event.first == AddFile) {
            q->addFile(info);
            q->selectAndRenameFile(fileUrl);
        } else {// rm file event
            // q->update();/*解决文管多窗口删除文件的时候，文官会崩溃的问题*/
            // todo: 此处引起效率变低，暂时注释
            q->remove(fileUrl);
        }
        if (me.isNull()) {
            return;
        }
    }
    _q_processFileEvent_runing.store(false);
}

bool DFileSystemModelPrivate::checkFileEventQueue()
{
    mutex.lock();
    bool isemptyqueue = fileEventQueue.isEmpty();
    mutex.unlock();
    return !isemptyqueue;
}

DFileSystemModel::DFileSystemModel(DFileViewHelper *parent)
    : QAbstractItemModel(parent)
    , d_ptr(new DFileSystemModelPrivate(this))
{
    qRegisterMetaType<State>(QT_STRINGIFY(State));
    qRegisterMetaType<DAbstractFileInfoPointer>(QT_STRINGIFY(DAbstractFileInfoPointer));

    m_smForDragEvent = new QSharedMemory();
}

DFileSystemModel::~DFileSystemModel()
{
    Q_D(DFileSystemModel);

    d->needQuitUpdateChildren = true;

    isNeedToBreakBusyCase = true; // 清场的时候，必须让其他资源线程跳出相关流程

    if (m_smForDragEvent) {
        delete m_smForDragEvent;
        m_smForDragEvent = nullptr;
    }

    //fix bug 33014
//    releaseJobController();

    if (d->jobController) {
        disconnect(d->jobController, &JobController::addChildren, this, &DFileSystemModel::onJobAddChildren);
        disconnect(d->jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished);
        disconnect(d->jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildrenOnNewThread);
        d->jobController->stopAndDeleteLater();
    }

    QMutexLocker locker(&m_mutex); // 必须等待其他 资源性线程结束，否则 要崩溃

    if (d->updateChildrenFuture.isRunning()) {
        d->updateChildrenFuture.cancel();
        d->updateChildrenFuture.waitForFinished();
    }

    if (d->watcher) {
        d->watcher->deleteLater();
    }

    if (d->rootNodeManager->isRunning()) {
        d->rootNodeManager->stop();
    }

    d->needQuitUpdateChildren = false;
    QMutexLocker lk(&d_ptr->mutexFlags);

    qDebug() << "DFileSystemModel is released soon!";
}

DFileViewHelper *DFileSystemModel::parent() const
{
    return static_cast<DFileViewHelper *>(QAbstractItemModel::parent());
}

QModelIndex DFileSystemModel::index(const DUrl &fileUrl, int column)
{
    Q_D(DFileSystemModel);

    if (!fileUrl.isValid())
        return QModelIndex();

    if (fileUrl == rootUrl()) {
        return createIndex(d->rootNode, column);
    }

    if (!d->rootNode) {
        return QModelIndex();
    }

//    const FileSystemNodePointer &node = d->urlToNode.value(fileUrl);
    const FileSystemNodePointer &node = d->rootNode->getNodeByUrl(fileUrl);

    if (!node) {
        return QModelIndex();
    }

    QModelIndex idx = createIndex(node, column);

    return idx;
}

QModelIndex DFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const DFileSystemModel);

    if (row < 0 || column < 0/* || row >= rowCount(parent) || column >= columnCount(parent)*/) {
        return QModelIndex();
    }

    const FileSystemNodePointer &parentNode = parent.isValid()
                                              ? FileSystemNodePointer(getNodeByIndex(parent))
                                              : d->rootNode;

    if (!parentNode) {
        return QModelIndex();
    }

    FileSystemNode *childNode = parentNode->getNodeByIndex(row).data();

    if (!childNode) {
        return QModelIndex();
    }

    return createIndex(row, column, childNode);
}

QModelIndex DFileSystemModel::parent(const QModelIndex &child) const
{
    const FileSystemNodePointer &indexNode = getNodeByIndex(child);

    if (!indexNode || !indexNode->parent) {
        return QModelIndex();
    }

    FileSystemNodePointer parentNode(indexNode->parent);

    return createIndex(parentNode, 0);
}

int DFileSystemModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const DFileSystemModel);

    const FileSystemNodePointer &parentNode = parent.isValid()
                                              ? FileSystemNodePointer(getNodeByIndex(parent))
                                              : d->rootNode;

    if (!parentNode) {
        return 0;
    }

    return parentNode->childrenCount();
}

int DFileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const DFileSystemModel);

    int columnCount = parent.column() > 0 ? 0 : DEFAULT_COLUMN_COUNT;
    if (!d->rootNode) {
        return columnCount;
    }

    const DAbstractFileInfoPointer &currentFileInfo = d->rootNode->fileInfo;

    if (currentFileInfo) {
        columnCount += currentFileInfo->userColumnRoles().count();
    }

    return columnCount;
}

QVariant DFileSystemModel::columnNameByRole(int role, const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);

//    const AbstractFileInfoPointer &fileInfo = this->fileInfo(index.isValid() ? index : d->activeIndex);
    const DAbstractFileInfoPointer &fileInfo = index.isValid() ? this->fileInfo(index) : d->rootNode->fileInfo;

    if (fileInfo) {
        return fileInfo->userColumnDisplayName(role);
    }

    return QVariant();
}

int DFileSystemModel::columnWidthByRole(int role) const
{
    Q_D(const DFileSystemModel);

    const DAbstractFileInfoPointer &currentFileInfo = d->rootNode->fileInfo;

    if (currentFileInfo) {
        return currentFileInfo->userColumnWidth(role, parent()->parent()->fontMetrics());
    }

    return 140;
}

bool DFileSystemModel::columnDefaultVisibleForRole(int role, const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);

//    const AbstractFileInfoPointer &fileInfo = this->fileInfo(index.isValid() ? index : d->activeIndex);
    const DAbstractFileInfoPointer &fileInfo = index.isValid() ? this->fileInfo(index) : d->rootNode->fileInfo;

    if (fileInfo) {
        return fileInfo->columnDefaultVisibleForRole(role);
    }

    return true;
}

bool DFileSystemModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) { // drives
        return false;
    }

    const FileSystemNodePointer &indexNode = getNodeByIndex(parent);

    return indexNode && isDir(indexNode);
}

QVariant DFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this) {
        return QVariant();
    }

    const FileSystemNodePointer &indexNode = getNodeByIndex(index);

    if (!indexNode) {
        return QVariant();
    }

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole: {
        int column_role = columnToRole(index.column());

        const QVariant &d = data(index.sibling(index.row(), 0), column_role);

        if (d.canConvert<QString>()) {
            return d;
        } else if (d.canConvert<QPair<QString, QString>>()) {
            return qvariant_cast<QPair<QString, QString>>(d).first;
        } else if (d.canConvert<QPair<QString, QPair<QString, QString>>>()) {
            return qvariant_cast<QPair<QString, QPair<QString, QString>>>(d).first;
        }

        return d;
    }
    case FilePathRole:
    case FileDisplayNameRole:
    case FileNameRole:
    case FileNameOfRenameRole:
    case FileBaseNameRole:
    case FileBaseNameOfRenameRole:
    case FileSuffixRole:
    case FileSuffixOfRenameRole:
        return indexNode->dataByRole(role);
    case FileIconRole:
        if (index.column() == 0) {
            return indexNode->fileInfo->fileIcon();
        }
        break;
    case Qt::TextAlignmentRole:
    case FileLastModifiedRole:
    case FileLastModifiedDateTimeRole:
    case FileSizeRole:
    case FileSizeInKiloByteRole:
    case FileMimeTypeRole:
    case FileCreatedRole:
    case FilePinyinName:
        return indexNode->dataByRole(role);
    case Qt::ToolTipRole: {
        const QList<int> column_role_list = parent()->columnRoleList();

        if (column_role_list.length() < 2) {
            break;
        }

        const QPoint &cursor_pos = parent()->parent()->mapFromGlobal(QCursor::pos());
        QStyleOptionViewItem option;

        option.init(parent()->parent());
        parent()->initStyleOption(&option, index);
        option.rect = parent()->parent()->visualRect(index);
        const QList<QRect> &geometries = parent()->itemDelegate()->paintGeomertys(option, index);

        // 从1开始是为了排除掉icon区域
        for (int i = 1; i < geometries.length() && i <= column_role_list.length(); ++i) {
            const QRect &rect = geometries.at(i);

            if (rect.left() <= cursor_pos.x() && rect.right() >= cursor_pos.x()) {
                const QString &tooltip = data(index, columnActiveRole(i - 1)).toString();

                if (option.fontMetrics.width(tooltip, -1, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt())) > rect.width()) {
                    return tooltip;
                } else {
                    break;
                }
            }
        }

        return QString();
    }
    case ExtraProperties:
        return indexNode->dataByRole(role);
    case FileIconModelToolTipRole: { // fix bug 202007010029 由于 list 视图处理 tooltip 的代码通用性太弱，所以这里新增一个 role 用来返回 tooltip
        /*fix bug 63708 tooltip应该显示的是显示的名字，不是实际的名字*/
        QString strToolTip = data(index, Qt::DisplayRole).toString();
        QStyleOptionViewItem option;

        option.init(parent()->parent());
        parent()->initStyleOption(&option, index);
        option.rect = parent()->parent()->visualRect(index);
        const QList<QRect> &geometries = parent()->itemDelegate()->paintGeomertys(option, index);
        if (geometries.count() < 3)
            return QString();
        if (option.fontMetrics.width(strToolTip) > geometries[1].width() * 2) {
            //主目录下的用户文件是做过特殊处理的（文档、音乐等），在这里也需要对长度达标的目录做特殊过滤
            //否则文档和下载目录会显示hoverTip
            const QString filePath = data(index, FilePathRole).toString();
            const QString stdDocPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);
            const QString stdDownPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DownloadLocation);
            if (filePath == stdDocPath || filePath == stdDownPath || filePath == "/data" + stdDocPath || filePath == "/data" + stdDownPath)
                return QString();

            return strToolTip;
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

    if (role == Qt::DisplayRole) {
        int column_role = columnToRole(column);

        if (column_role < FileUserRole) {
            return roleName(column_role);
        } else {
//            const AbstractFileInfoPointer &fileInfo = this->fileInfo(d->activeIndex);
            const DAbstractFileInfoPointer &fileInfo = d->rootNode->fileInfo;

            if (fileInfo) {
                if (fileInfo->columnIsCompact()) {
                    const QList<int> roles = fileInfo->userColumnChildRoles(column);

                    if (!roles.isEmpty()) {
                        column_role = d->columnActiveRole.value(column, roles.first());
                    }
                }

                return fileInfo->userColumnDisplayName(column_role);
            }

            return QVariant();
        }
    }

    return QVariant();
}

QString DFileSystemModel::roleName(int role)
{
    switch (role) {
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
    case FileLastReadRole:
        return tr("Last access");
    case FilePathRole:
        return tr("Path");
    default:
        return QString();
    }
}

int DFileSystemModel::columnToRole(int column) const
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode)
        return UnknowRole;

    const DAbstractFileInfoPointer &fileInfo = d->rootNode->fileInfo;

    if (fileInfo) {
        //获取修改过顺序后的列属性
        DUrl url = rootUrl();
        //搜索目录统一从"search:"项中取配置数据
        if (url.isSearchFile()) {
            url = DUrl();
            url.setScheme(SEARCH_SCHEME);
        }
        const QVariantMap &map = DFMApplication::appObtuselySetting()->value("FileViewState", url).toMap();
        if (map.contains("headerList")) {
            const QVariantList &indexList = map.value("headerList").toList();
            if (indexList.length() > column)
                return indexList.at(column).toInt();
        }

        return fileInfo->userColumnRoles().value(column, UnknowRole);
    }

    return UnknowRole;
}

int DFileSystemModel::roleToColumn(int role) const
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode) {
        return -1;
    }

//        const AbstractFileInfoPointer &fileInfo = this->fileInfo(d->activeIndex);
    const DAbstractFileInfoPointer &fileInfo = d->rootNode->fileInfo;

    if (fileInfo) {
        int column = fileInfo->userColumnRoles().indexOf(role);
        DUrl url = rootUrl();
        //搜索目录统一从"search:"项中取配置数据
        if (url.isSearchFile()) {
            url = DUrl();
            url.setScheme(SEARCH_SCHEME);
        }
        //获取修改过顺序后的列属性的索引
        const QVariantMap &map = DFMApplication::appObtuselySetting()->value("FileViewState", url).toMap();
        if (map.contains("headerList")) {
            const QVariantList &indexList = map.value("headerList").toList();
            if (indexList.length() > column)
                column = indexList.indexOf(role);
        }

        if (column < 0) {
            return -1;
        }

        return column;
    }

    return -1;
}

void DFileSystemModel::fetchMore(const QModelIndex &parent)
{
    Q_D(DFileSystemModel);

    if (d->eventLoop || !d->rootNode) {
        return;
    }

    isNeedToBreakBusyCase = false; // 这是fileview 切换的入口，切换的时候 置 flag，不要停止正常流程

    const FileSystemNodePointer &parentNode = getNodeByIndex(parent);

    if (!parentNode || parentNode->populatedChildren) {
        return;
    }

    //
    if (!releaseJobController()) {
        return;
    }
    qInfo() << "fetchMore start traverse all files in current dir = " << parentNode->fileInfo->fileUrl();
    d->jobController = fileService->getChildrenJob(this, parentNode->fileInfo->fileUrl(), QStringList(), d->filters,
                                                   QDirIterator::NoIteratorFlags, false, parentNode->fileInfo->isGvfsMountFile());

    if (!d->jobController) {
        return;
    }
    if (!d->rootNode->fileInfo->hasOrderly()) {
        // 对于无需列表, 较少返回结果的等待时间
        d->jobController->setTimeCeiling(100);
    }

    connect(d->jobController, &JobController::addChildren, this, &DFileSystemModel::onJobAddChildren, Qt::QueuedConnection);
    connect(d->jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished, Qt::QueuedConnection);
    connect(d->jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildrenOnNewThread, Qt::DirectConnection);
    /// make root file to active
    d->rootNode->fileInfo->makeToActive();
    /// start file watcher
    if (d->watcher) {
        d->watcher->startWatcher();
    }
    parentNode->populatedChildren = true;

    setState(Busy);

    d->childrenUpdated = false;
    //
    d->jobController->start();
    d->rootNodeManager->setEnable(true);
}

Qt::ItemFlags DFileSystemModel::flags(const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);
    QPointer<DFileSystemModel> me = const_cast<DFileSystemModel *>(this);
//    QMutexLocker lk(&d_ptr->mutexFlags);
    if (!me) {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid()) {
        return flags;
    }
    const FileSystemNodePointer &indexNode = getNodeByIndex(index);

    if (!indexNode) {
        return flags;
    }
    if (!d->passNameFilters(indexNode)) {
        flags &= ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        // ### TODO you shouldn't be able to set this as the current item, task 119433
        return flags & ~ indexNode->fileInfo->fileItemDisableFlags();
    }

    flags |= Qt::ItemIsDragEnabled;
    if ((index.column() == 0)) {
        if (d->readOnly) {
            return flags;
        }
        //fix bug 29914 fileInof为nullptr
        if (indexNode && indexNode->fileInfo && indexNode->fileInfo->canRename()) {
            flags |= Qt::ItemIsEditable;
        }
        if (indexNode && indexNode->fileInfo && indexNode->fileInfo->isWritable()) {
            //candrop十分耗时,在不关心Qt::ItemDropEnable的调用时ignoreDropFlag为true，不调用candrop，节省时间,bug#10926
            if (!ignoreDropFlag && indexNode && indexNode->fileInfo && indexNode->fileInfo->canDrop()) {
                flags |= Qt::ItemIsDropEnabled;
            } else {
                flags |= Qt::ItemNeverHasChildren;
            }
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
//    qDebug() << "drop mime data";
    Q_UNUSED(row);
    Q_UNUSED(column);
    if (!parent.isValid()) {
        return false;
    }

    bool success = true;
    DUrl toUrl = getUrlByIndex(parent);

    DUrlList urlList = DUrl::fromQUrlList(data->urls());

    const DAbstractFileInfoPointer &info = fileService->createFileInfo(this, toUrl);

    // 当为拖拽压缩时
    if (info->canDragCompress()
            && !info->isDir()) { // fix 101867: 如果是文件夹 就不走压缩逻辑
        qDebug() << "执行拖拽压缩";
        return FileUtils::appendCompress(toUrl, urlList);
    }

    for (DUrl &u : urlList) {
        // we only do redirection for burn:// urls for the fear of screwing everything up again
        if (u.scheme() == BURN_SCHEME) {
            for (DAbstractFileInfoPointer fi = fileService->createFileInfo(nullptr, u); fi->canRedirectionFileUrl(); fi = fileService->createFileInfo(nullptr, u)) {
                u = fi->redirectedFileUrl();
            }
        }
    }

    if (info->isSymLink()) {
        toUrl = info->rootSymLinkTarget();
    }

    if (DFMGlobal::isTrashDesktopFile(toUrl)) {
        toUrl = DUrl::fromTrashFile("/");
        fileService->moveToTrash(this, urlList);
        return true;
    } else if (DFMGlobal::isComputerDesktopFile(toUrl)) {
        return true;
    } else if (DFMGlobal::isDesktopFile(toUrl)) {
        return FileUtils::launchApp(toUrl.toLocalFile(), DUrl::toStringList(urlList));
    }

    switch (action) {
    case Qt::CopyAction:
        if (urlList.count() > 0) {
            // blumia: 如果不在新线程跑的话，用户就只能在复制完毕之后才能进行新的拖拽操作。
            QtConcurrent::run([ = ]() {
                fileService->pasteFile(this, DFMGlobal::CopyAction, toUrl, urlList);
            });
        }
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        // NOTE(zccrs): 为MoveAction时，如果执行成功，QAbstractItemView会调用clearOrRemove移除此item
        //              所以此处必须判断是否粘贴完成，不然会导致此item消失
        // fix bug 63341 ，开启普通文件删除提示功能后，打开最近使用和回收站两个窗口，拖拽最近使用到回收站，在还没有
        // 弹出对话框的时候，快速框选最近访问窗口。这时弹窗出现(使用的runinMainThread)使用的exec就会开启一个本地事件循环，将框选事件加入本地循环
        // 执行，框选事件又在等待drag事件结束（mousemove事件结束），本地事件又阻塞drag事件，相互等待。使用线程去执行
        // 业务逻辑，让drop事件快速返回。
        if (toUrl.isTrashFile()) {
            QtConcurrent::run([=]() {
                fileService->moveToTrash(this, urlList);
            });
            break;
        }
        success = !fileService->pasteFile(this, DFMGlobal::CutAction, toUrl, urlList).isEmpty();
        break;
    default:
        return false;
    }

    return success;
}

QMimeData *DFileSystemModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QSet<QUrl> urls_set;
    QList<QModelIndex>::const_iterator it = indexes.begin();

    for (; it != indexes.end(); ++it) {
        if ((*it).column() == 0) {
            const DAbstractFileInfoPointer &fileInfo = this->fileInfo(*it);
            const QUrl &url = fileInfo->mimeDataUrl();

            if (urls_set.contains(url)) {
                continue;
            }

            urls << url;
            urls_set << url;
        }
    }

    QMimeData *data = new QMimeData();

    DFMGlobal::setMimeDataUserID(data);
    data->setUrls(urls);
    qInfo() << data->data(MIME_USER_ID);
//    data->setText(urls.first().path());
//    data->setData("forDragEvent", urls.first().toEncoded());
//    FOR_DRAGEVENT = urls;
//    qDebug() << "Set FOR_DRAGEVENT urls FOR_DRAGEVENT count = " << FOR_DRAGEVENT.length();
    m_smForDragEvent->setKey(DRAG_EVENT_URLS);
    //qDebug() << DRAG_EVENT_URLS;
    if (m_smForDragEvent->isAttached()) {
        if (!m_smForDragEvent->detach()) {
            return data;
        }
    }
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);
    out << urls;
    //int size = static_cast<int>(buffer.size());
    //fix task 21485 分配一个固定的5M内存
    bool bcanwrite = m_smForDragEvent->create(5 * 1024 * 1024);
    if (bcanwrite || m_smForDragEvent->error() == QSharedMemory::AlreadyExists) {
        //因为创建失败，就没有连接内存，所以写失败
        if (!bcanwrite) {
            m_smForDragEvent->attach();
        }
        m_smForDragEvent->lock();
        char *to = static_cast<char *>(m_smForDragEvent->data());
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(static_cast<size_t>(buffer.size()), static_cast<size_t>(m_smForDragEvent->size())));
        m_smForDragEvent->unlock();
        qDebug() << " write mem finish. " << m_smForDragEvent->errorString() << m_smForDragEvent->size();
    }
    return data;
}

bool DFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    const FileSystemNodePointer &parentNode = getNodeByIndex(parent);

    if (!parentNode) {
        return false;
    }

    return (parentNode->fileInfo->canFetch() || !parentNode->fileInfo->exists()) && !parentNode->populatedChildren;
}

QModelIndex DFileSystemModel::setRootUrl(const DUrl &fileUrl)
{
    Q_D(DFileSystemModel);
    //首次进入目录记录过滤规则
    if (isFirstRun) {
        m_filters = d->filters;
        isFirstRun = false;
    }
    qDebug() << fileUrl;
    //非回收站还原规则
    if (!fileUrl.isTrashFile()) {
        d->filters = m_filters;
    }
    //回收站设置规则
    else {
        d->filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
    }
    // Restore state
    setState(Idle);

    disconnect(fileSignalManager, &FileSignalManager::requestRefreshFileModel, this, &DFileSystemModel::refresh);
    if (fileUrl == DUrl::fromUserShareFile("/")) {
        connect(fileSignalManager, &FileSignalManager::requestRefreshFileModel, this, &DFileSystemModel::refresh);
    }

    if (d->eventLoop) {
        d->eventLoop->exit(1);
    }

    // 断开获取上个目录的job的信号
    if (d->jobController) {
        disconnect(d->jobController, &JobController::addChildren, this, &DFileSystemModel::onJobAddChildren);
        disconnect(d->jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished);
        disconnect(d->jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildrenOnNewThread);
    }

    if (d->updateChildrenFuture.isRunning()) {
        // 使用QFuture::cancel() 函数无效，定义个变量控制线程的退出
        d->needQuitUpdateChildren = true;
        d->updateChildrenFuture.cancel();
        d->updateChildrenFuture.waitForFinished();
        d->needQuitUpdateChildren = false;
    }

    if (d->rootNode) {
//        const DUrl rootFileUrl = d->rootNode->fileInfo->fileUrl();

//        if (fileUrl == rootFileUrl) {
//            return createIndex(d->rootNode, 0);
//        }
//        对于相同路径也要走同样的流程

        clear();
    }

    if (d->watcher) {
        disconnect(d->watcher, nullptr, this, nullptr);
        d->watcher->deleteLater();
    }

//    d->rootNode = d->urlToNode.value(fileUrl);

    d->rootNode = createNode(Q_NULLPTR, fileService->createFileInfo(this, fileUrl), &d->rootNodeRWLock);

    d->rootNodeManager->stop();
    d->rootNodeManager->setRootNode(d->rootNode);
    d->watcher = DFileService::instance()->createFileWatcher(this, fileUrl);
    d->columnActiveRole.clear();

    if (d->watcher)
        d->watcher->setParent(this);

    if (d->watcher && !d->rootNode->fileInfo->isPrivate()) {
        connect(d->watcher, SIGNAL(fileAttributeChanged(DUrl, int)),
                this, SLOT(_q_onFileUpdated(DUrl, int)));
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

DUrlList DFileSystemModel::sortedUrls()
{
    Q_D(const DFileSystemModel);
    if (!d->rootNode) {
        return DUrlList();
    }
    return d->rootNode->getChildrenUrlList();
}

DUrlList DFileSystemModel::getNoTransparentUrls()
{
    DUrlList lst = sortedUrls();
    DUrlList lstValid;
    for (DUrl url : lst) {
        QModelIndex idx = index(url);
        if (!idx.isValid())
            continue;
        if (parent()->isTransparent(idx))
            continue;
        lstValid << url;
    }
    return lstValid;
}

DUrl DFileSystemModel::getUrlByIndex(const QModelIndex &index) const
{
    const FileSystemNodePointer &node = getNodeByIndex(index);
    if (!node) {
        return DUrl();
    }

    return node->fileInfo->fileUrl();
}

void DFileSystemModel::setSortColumn(int column, Qt::SortOrder order)
{
    int role = columnActiveRole(column);
    setSortRole(role, order);
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

    if (d->nameFilters == nameFilters) {
        return;
    }

    d->nameFiltersMatchResultMap.clear();
    d->nameFilters = nameFilters;

    emitAllDataChanged();
}

void DFileSystemModel::setFilters(QDir::Filters filters)
{
    Q_D(DFileSystemModel);

    m_filters = filters; //记录文件过滤规则
    if (d->filters == filters) {
        return;
    }

    d->nameFiltersMatchResultMap.clear();
    d->filters = filters;

    refresh();
}

void DFileSystemModel::setAdvanceSearchFilter(const QMap<int, QVariant> &formData, bool turnOn, bool updateView)
{
    Q_D(DFileSystemModel);

    if (!advanceSearchFilter()) {
        d->advanceSearchFilter.reset(new FileFilter);
    }

    advanceSearchFilter()->filterEnabled = turnOn;

    if (advanceSearchFilter()->filterRule == formData) {
        return;
    }

    advanceSearchFilter()->filterRule = formData;

    advanceSearchFilter()->f_comboValid[SEARCH_RANGE] = true;
    advanceSearchFilter()->f_includeSubDir = advanceSearchFilter()->filterRule[SEARCH_RANGE].toBool();

    advanceSearchFilter()->f_typeString = advanceSearchFilter()->filterRule[FILE_TYPE].toString();
    advanceSearchFilter()->f_comboValid[FILE_TYPE] = !advanceSearchFilter()->f_typeString.isEmpty();

    advanceSearchFilter()->f_comboValid[SIZE_RANGE] = advanceSearchFilter()->filterRule[SIZE_RANGE].canConvert<QPair<quint64, quint64> >();
    if (advanceSearchFilter()->f_comboValid[SIZE_RANGE]) {
        advanceSearchFilter()->f_sizeRange = advanceSearchFilter()->filterRule[SIZE_RANGE].value<QPair<quint64, quint64> >();
    }

    // 计算时间过滤条件
    auto calDateFilter = [ = ](_asb_LabelIndex labelIndex, QDateTime & startTime, QDateTime & endTime) {
        int dateRange = advanceSearchFilter()->filterRule[labelIndex].toInt();
        advanceSearchFilter()->f_comboValid[labelIndex] = (dateRange != 0);

        if (advanceSearchFilter()->f_comboValid[labelIndex]) {
            int firstDayOfWeek = QLocale::system().firstDayOfWeek();
            QDate today = QDate::currentDate();
            QDate tomorrow = QDate::currentDate().addDays(+1);
            int dayDist = today.dayOfWeek() - firstDayOfWeek;
            if (dayDist < 0) dayDist += 7;

            switch (dateRange) { // see DFMAdvanceSearchBar::initUI() for all cases
            case 1:
                startTime = QDateTime(today);
                endTime = QDateTime(tomorrow);
                break;
            case 2:
                startTime = QDateTime(today).addDays(-1);
                endTime = QDateTime(today);
                break;
            case 7:
                startTime = QDateTime(today).addDays(0 - dayDist);
                endTime = QDateTime(tomorrow);
                break;
            case 14:
                startTime = QDateTime(today).addDays(-7 - dayDist);
                endTime = QDateTime(today).addDays(0 - dayDist);
                break;
            case 30:
                startTime = QDateTime(QDate(today.year(), today.month(), 1));
                endTime = QDateTime(tomorrow);
                break;
            case 60:
                startTime = QDateTime(QDate(today.year(), today.month(), 1)).addMonths(-1);
                endTime = QDateTime(QDate(today.year(), today.month(), 1));
                break;
            case 365:
                startTime = QDateTime(QDate(today.year(), 1, 1));
                endTime = QDateTime(tomorrow);
                break;
            case 730:
                startTime = QDateTime(QDate(today.year(), 1, 1)).addYears(-1);
                endTime = QDateTime(QDate(today.year(), 1, 1));
                break;
            default:
                break;
            }
        }
    };

    calDateFilter(DATE_RANGE, advanceSearchFilter()->f_dateRangeStart, advanceSearchFilter()->f_dateRangeEnd);
    calDateFilter(ACCESS_DATE_RANGE, advanceSearchFilter()->f_accessDateRangeStart, advanceSearchFilter()->f_accessDateRangeEnd);
    calDateFilter(CREATE_DATE_RANGE, advanceSearchFilter()->f_createDateRangeStart, advanceSearchFilter()->f_createDateRangeEnd);

    if (updateView) {
        applyAdvanceSearchFilter();
    }
}

void DFileSystemModel::applyAdvanceSearchFilter()
{
    Q_D(DFileSystemModel);
    if (!d->rootNode)
        return;
    setState(Busy);
    d->rootNode->applyFileFilter(advanceSearchFilter());
    setState(Idle);
    sort();
}

std::shared_ptr<FileFilter> DFileSystemModel::advanceSearchFilter()
{
    Q_D(DFileSystemModel);

    return d->advanceSearchFilter;
}

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

    if (!d->rootNode || !d->rootNode->fileInfo) {
        return -1;
    }

    if (d->rootNode->fileInfo->columnIsCompact()) {
        int i = 0;

        for (const int role : d->rootNode->fileInfo->userColumnRoles()) {
            if (role == d->sortRole) {
                return i;
            }

            const QList<int> childe_roles = d->rootNode->fileInfo->userColumnChildRoles(i);

            if (childe_roles.indexOf(d->sortRole) >= 0) {
                return i;
            }

            ++i;
        }
    }

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

    if (old_sortRole == d->sortRole && old_sortOrder == d->srotOrder) {
        return;
    }

    sort();
}

bool DFileSystemModel::sort()
{
    return sort(true);
}

bool DFileSystemModel::sort(bool emitDataChange)
{
    if (!enabledSort()) {
        return false;
    }

    if (state() == Busy) {
        qWarning() << "I'm busying";

        return false;
    }

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return false;
    }

    if (QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileSystemModel::sort);

        return false;
    }

    return  doSortBusiness(emitDataChange);
}

bool DFileSystemModel::doSortBusiness(bool emitDataChange)
{
    if (isNeedToBreakBusyCase) // 有清场流程来了,其他线程无需进行处理了
        return false;

    Q_D(const DFileSystemModel);

    QMutexLocker locker(&m_mutex);

    if (isNeedToBreakBusyCase) // bug 27384: 有清场流程来了,其他线程无需进行处理了， 这里做处理是 其他线程可能获取 锁，那么这里做及时处理
        return false;

    const FileSystemNodePointer &node = d->rootNode;

    if (!node) {
        return false;
    }

//    qDebug() << "start the sort business";

    QList<FileSystemNodePointer> list = node->getChildrenList();

    bool ok = sort(node->fileInfo, list);

    if (ok) {
        node->setChildrenList(list);
        if (emitDataChange) {
            emitAllDataChanged();
        }
    }

//    qDebug() << "end the sort business";
    emit sortFinished();
    return ok;
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

    if (!d->rootNode) {
        return DAbstractFileInfoPointer();
    }

    if (fileUrl == d->rootNode->fileInfo->fileUrl()) {
        return d->rootNode->fileInfo;
    }

    const FileSystemNodePointer &node = d->rootNode->getNodeByUrl(fileUrl);

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
    if (fileUrl == rootUrl()) {
        return d->rootNode->fileInfo;
    }

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

bool DFileSystemModel::setColumnCompact(bool compact)
{
    Q_D(DFileSystemModel);

    if (d->columnCompact == compact) {
        return false;
    }

    d->columnCompact = compact;

    if (d->rootNode) {
        if (d->rootNode->fileInfo) {
            d->rootNode->fileInfo->setColumnCompact(compact);
        }

        for (const FileSystemNodePointer &child : d->rootNode->getChildrenList()) {
            child->fileInfo->setColumnCompact(compact);
        }
    }

    return true;
}

bool DFileSystemModel::columnIsCompact() const
{
    Q_D(const DFileSystemModel);

    if (d->rootNode && d->rootNode->fileInfo) {
        return d->rootNode->fileInfo->columnIsCompact();
    }

    return d->columnCompact;
}

void DFileSystemModel::setColumnActiveRole(int column, int role)
{
    Q_D(DFileSystemModel);

    d->columnActiveRole[column] = role;
}

int DFileSystemModel::columnActiveRole(int column) const
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode || !d->rootNode->fileInfo) {
        return UnknowRole;
    }

    if (!d->rootNode->fileInfo->columnIsCompact()) {
        return columnToRole(column);
    }

    const QList<int> &roles = d->rootNode->fileInfo->userColumnChildRoles(column);

    if (roles.isEmpty()) {
        return columnToRole(column);
    }

    return d->columnActiveRole.value(column, roles.first());
}

void DFileSystemModel::stopCurrentJob()
{
    releaseJobController();
    setState(Idle);
}

void DFileSystemModel::updateChildren(QList<DAbstractFileInfoPointer> list)
{
    Q_D(DFileSystemModel);

//    qDebug() << "Begin update chidren. file list count " <<  list.count();
    const FileSystemNodePointer &node = d->rootNode;

    if (!node) {
        return;
    }

    QPointer<JobController> job = d->jobController;
    QPointer<DFileSystemModel> dp = this;

    if (!job.isNull()) {
        job->pause();
    }
    node->clearChildren();

    QHash<DUrl, FileSystemNodePointer> fileHash;
    QList<FileSystemNodePointer> fileList;

    fileHash.reserve(list.size());
    fileList.reserve(list.size());

    if (dp.isNull())
        return;

    for (const DAbstractFileInfoPointer &fileInfo : list) {
        if (d->needQuitUpdateChildren) {
            break;
        }

        if (!fileInfo) {
            continue;
        }
        //挂载设备下目录添加tag后 移除该挂载设备 目录已不存在但其URL依然还保存在tag集合中
        //该问题导致这些不存在的目录依然会添加到tag的fileview下 引起其他被标记文件可能标记数据获取失败
        //为了避免引起其他问题 暂时只对tag的目录做处理
        if (fileInfo->fileUrl().scheme() == TAG_SCHEME && !fileInfo->exists()) {
            continue;
        }

        // feature: hide specified dirs of unremovable devices directly under devices' mountpoint
        if (!isDesktop      // 桌面不执行隐藏，否则将导致桌面无法实时刷新“发送到”列表：这行语句将导致 gvfsManager 提前在线程中被初始化
                && !filters().testFlag(QDir::Hidden)
                && fileInfo->isDir()
                && deviceListener->hiddenDirs().contains(fileInfo->filePath())) {
            continue;
        }

        // qDebug() << "update node url = " << fileInfo->filePath();
        const FileSystemNodePointer &chileNode = createNode(node.data(), fileInfo);
        //当文件路径和名称都相同的情况下，fileHash在赋值，会释放，fileList保存的普通指针就是悬空指针
        if (!chileNode->shouldHideByFilterRule(advanceSearchFilter()) && !fileHash[fileInfo->fileUrl()]) {
            fileHash[fileInfo->fileUrl()] = chileNode;
            fileList << chileNode;
            if (fileInfo->fileUrl().scheme() == SEARCH_SCHEME)
                emit showFilterButton();
        }
    }
    if (dp.isNull())
        return;
    if (enabledSort())
        sort(node->fileInfo, fileList);

    if (dp.isNull())
        return;
//    qDebug() << "begin insert rows count = " << QString::number(list.count());
    beginInsertRows(createIndex(node, 0), 0, list.count() - 1);

    node->setChildrenMap(fileHash);
    node->setChildrenList(fileList);
    endInsertRows();

    if (dp.isNull())
        return;

    if (d->needQuitUpdateChildren)
        return;
    if (job.isNull() || job->isFinished()) {
        setState(Idle);
    } else {
        d->childrenUpdated = true;
    }

    if (!job.isNull() && job->state() == JobController::Paused) {
        job->start();
    }

    if (dp.isNull())
        return;

//    qDebug() << "finish update children. file count = " << node->childrenCount() << (job ? job->state() : -1);
    if (!job.isNull()) {
        //刷新完成标志
        bool finished = job->isUpdatedFinished();
        qInfo() << " finish update children. isUpdatedFinished = " << finished;
        //若刷新完成通知桌面重新获取文件
        if (finished && !dp.isNull())
            emit sigJobFinished();
    } else if (!dp.isNull()) {
        emit sigJobFinished();
    }
}

void DFileSystemModel::updateChildrenOnNewThread(QList<DAbstractFileInfoPointer> list)
{
    Q_D(DFileSystemModel);
    QMutexLocker locker(&m_mutex);
    QPointer<DFileSystemModel> me = this;
    if (!me) {
        return;
    }

    if (d->jobController) {
        d->jobController->pause();
    }

    if (QThreadPool::globalInstance()->activeThreadCount() >= QThreadPool::globalInstance()->maxThreadCount()) {
        QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() + 10);
    }

    emit updateFilterRule(d->advanceSearchFilter.get());
    d->updateChildrenFuture = QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileSystemModel::updateChildren, list);
}

void DFileSystemModel::refresh(const DUrl &fileUrl)
{
    Q_D(const DFileSystemModel);

    if (d->state != Idle) {
        return;
    }

//    const FileSystemNodePointer &node = d->urlToNode.value(fileUrl);
    const FileSystemNodePointer &node = d->rootNode;

    if (!node) {
        return;
    }

    if (!fileUrl.isEmpty() && fileUrl != node->fileInfo->fileUrl()) {
        return;
    }

//    if(!isDir(node))
//        return;

    node->populatedChildren = false;

    const QModelIndex &index = createIndex(node, 0);
    if (beginRemoveRows(index, 0, rowCount(index) - 1)) {
        node->clearChildren();
        endRemoveRows();
    }

    fetchMore(index);
}

void DFileSystemModel::update()
{
    Q_D(const DFileSystemModel);

    const QModelIndex &rootIndex = createIndex(d->rootNode, 0);

    for (const FileSystemNodePointer &node : d->rootNode->getChildrenList()) {
        if (node->fileInfo)
            node->fileInfo->refresh();
    }

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

    if (d->enabledSort == enabledSort) {
        return;
    }

    d->enabledSort = enabledSort;
    emit enabledSortChanged(enabledSort);
}

bool DFileSystemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_D(DFileSystemModel);

    const FileSystemNodePointer &parentNode = parent.isValid() ? getNodeByIndex(parent) : d->rootNode;
    if (parentNode && parentNode->populatedChildren) {
        const DAbstractFileInfoPointer &fileInfo = this->fileInfo(index(row, 0));
        fileInfo->refresh();
        if (fileInfo->exists())
            return true;
        if (d->currentRemove) {
            return true;
        }
        if (beginRemoveRows(createIndex(parentNode, 0), row, row + count - 1)) {
            for (int i = 0; i < count; ++i) {
                Q_UNUSED(parentNode->takeNodeByIndex(row));
            }
            endRemoveRows();
        }
    }

    return true;
}

bool DFileSystemModel::remove(const DUrl &url)
{
    Q_D(DFileSystemModel);

    const FileSystemNodePointer &parentNode = d->rootNode;

    if (parentNode && parentNode->populatedChildren) {
        int index = parentNode->indexOfChild(url);
        if (index < 0) {
            return false;
        }
        d->currentRemove = true;
        if (beginRemoveRows(createIndex(parentNode, 0), index, index)) {
            Q_UNUSED(parentNode->takeNodeByIndex(index));
            endRemoveRows();
        }
        d->currentRemove = false;

        return true;
    }

    return false;
}

const FileSystemNodePointer DFileSystemModel::getNodeByIndex(const QModelIndex &index) const
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode) {
        return FileSystemNodePointer();
    }

    FileSystemNode *indexNode = static_cast<FileSystemNode *>(index.internalPointer());

    if (!indexNode) {
        return FileSystemNodePointer();
    }

    if (indexNode == d->rootNode.constData()) {
        return d->rootNode;
    }

    if (d->rootNode->getNodeByIndex(index.row()).data() != indexNode
            || indexNode->ref <= 0) {
        return FileSystemNodePointer();
    }

    return FileSystemNodePointer(indexNode);
}

QModelIndex DFileSystemModel::createIndex(const FileSystemNodePointer &node, int column) const
{
    int row = (node->parent && node->parent->childrenCount() > 0)
              ? node->parent->indexOfChild(node)
              : 0;

    return createIndex(row, column, const_cast<FileSystemNode *>(node.data()));
}

bool DFileSystemModel::isDir(const FileSystemNodePointer &node) const
{
    return node->fileInfo->isDir();
}

bool DFileSystemModel::sort(const DAbstractFileInfoPointer &parentInfo, QList<FileSystemNodePointer> &list) const
{
    Q_D(const DFileSystemModel);

    if (!parentInfo) {
        return false;
    }

    DAbstractFileInfo::CompareFunction sortFun = parentInfo->compareFunByColumn(d->sortRole);

    if (!sortFun) {
        return false;
    }

    qSort(list.begin(), list.end(), [sortFun, d, this](const FileSystemNodePointer node1, const FileSystemNodePointer node2) {
        if (this->isNeedToBreakBusyCase) //bug 27384: 当是网络文件的时候，这里奇慢，需要快速跳出 qSort操作，目前我只想到这种方案：不做比较，或者 直接跳出sort 更好
            return false;

        return sortFun(node1->fileInfo, node2->fileInfo, d->srotOrder);
    });

    if (columnIsCompact() && d->rootNode && d->rootNode->fileInfo) {
        int column = 0;

        for (int role : d->rootNode->fileInfo->userColumnRoles()) {
            if (role == d->sortRole) {
                return true;
            }

            if (d->rootNode->fileInfo->userColumnChildRoles(column).indexOf(d->sortRole) >= 0) {
                const_cast<DFileSystemModel *>(this)->setColumnActiveRole(column, d->sortRole);
            }

            ++column;
        }
    }

    return true;
}

const FileSystemNodePointer DFileSystemModel::createNode(FileSystemNode *parent, const DAbstractFileInfoPointer &info, QReadWriteLock *lock)
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
    Q_D(const DFileSystemModel);
    QString url(info->filePath());
    if (m_allFileSystemNodes.contains(url) && d->rootNode) {
        qDebug() << "recreate node url = " << url;
        d->rootNode->removeFileSystemNode(m_allFileSystemNodes[url]);
    }

    FileSystemNodePointer node(new FileSystemNode(parent, info, this, lock));

    node->fileInfo->setColumnCompact(d->columnCompact);
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

    if (!d->rootNode) {
        return;
    }

    isNeedToBreakBusyCase = true; // 清场的时候，必须进口跳出相关流程

    QMutexLocker locker(&m_mutex); // bug 26972, while the sort case is ruuning, there should be crashed ASAP, so add locker here!

    const QModelIndex &index = createIndex(d->rootNode, 0);
    if (beginRemoveRows(index, 0, d->rootNode->childrenCount() - 1)) {
        deleteNode(d->rootNode);
        endRemoveRows();
    }
}

void DFileSystemModel::setState(DFileSystemModel::State state)
{
    Q_D(DFileSystemModel);

    if (d->state == state) {
        return;
    }

    d->state = state;

    emit stateChanged(state);
}

void DFileSystemModel::onJobAddChildren(const DAbstractFileInfoPointer &fileInfo)
{
//    static QMutex mutex;
//    static QWaitCondition condition;

//    QTimer *volatile timer = new QTimer;
//    timer->setSingleShot(true);
//    timer->moveToThread(qApp->thread());
//    timer->setParent(this);
//    connect(timer, &QTimer::timeout, this, [this, fileInfo, &timer] {
//        timer->deleteLater();
//        addFile(fileInfo);
//        timer = Q_NULLPTR;
//        condition.wakeAll();
//    }, Qt::DirectConnection);
//    mutex.lock();
//    timer->metaObject()->invokeMethod(timer, "start", Q_ARG(int, 0));

//    if (timer) {
//        condition.wait(&mutex);
//    }

//    mutex.unlock();
    Q_D(DFileSystemModel);
    d->rootNodeManager->addFile(fileInfo, FileNodeManagerThread::AppendFile);
    if (fileInfo->fileUrl().scheme() == SEARCH_SCHEME)
        emit showFilterButton();
}

void DFileSystemModel::onJobFinished()
{
    Q_D(const DFileSystemModel);

    if (d->childrenUpdated && !d->rootNodeManager->isRunning()) {
        setState(Idle);
    }
}

void DFileSystemModel::addFile(const DAbstractFileInfoPointer &fileInfo)
{
    Q_D(const DFileSystemModel);

    const FileSystemNodePointer parentNode = d->rootNode;
    const DUrl &fileUrl = fileInfo->fileUrl();

    if (parentNode && parentNode->populatedChildren && !parentNode->childContains(fileUrl)) {
        QPointer<DFileSystemModel> me = this;

        int row = -1;

        if (enabledSort()) {
            row = 0;

            QFuture<void> result;

            // tmp: 暂时不排序 排序的宏在大量添加文件操作时会崩（最近访问目录不存在大量文件添加的情况 可放开）
            if (fileInfo->hasOrderly() && fileUrl.isRecentFile()) {
                DAbstractFileInfo::CompareFunction compareFun = fileInfo->compareFunByColumn(d->sortRole);

                if (compareFun) {
                    result = QtConcurrent::run(QThreadPool::globalInstance(), [&] {
                        forever
                        {
                            if (!me || row >= parentNode->childrenCount()) {
                                break;
                            }

                            const FileSystemNodePointer &node = parentNode->getNodeByIndex(row);
                            if (node) {
                                if (compareFun(fileInfo, node->fileInfo, d->srotOrder)) {
                                    break;
                                }
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
                    forever
                    {
                        if (!me || row >= parentNode->childrenCount()) {
                            break;
                        }

                        const FileSystemNodePointer &node = parentNode->getNodeByIndex(row);
                        if (node) {
                            if (node->fileInfo && node->fileInfo->isFile()) {
                                break;
                            }

                        }

                        ++row;
                    }
                });
            }
            while (!result.isFinished()) {
                qApp->processEvents();
            }
        }
        if (!me) {
            return;
        }

        if (row == -1) {
            row = parentNode->childrenCount();
        }

        if (m_allFileSystemNodes.contains(fileInfo->filePath())) {
            qDebug() << "File already exist url = " <<  fileInfo->filePath();
            return;
        }

        //qDebug() << "insert node row = " << QString::number(row);
        beginInsertRows(createIndex(parentNode, 0), row, row);

//        FileSystemNodePointer node = d->urlToNode.value(fileUrl);

//        if(!node) {
        FileSystemNodePointer node = createNode(parentNode.data(), fileInfo);

//            d->urlToNode[fileUrl] = node;
//        }

        parentNode->insertChildren(row, fileUrl, node);

        endInsertRows();
    }
}

void DFileSystemModel::emitAllDataChanged()
{
    Q_D(const DFileSystemModel);

    if (!d->rootNode) {
        return;
    }

    QModelIndex parentIndex = createIndex(d->rootNode, 0);
    QModelIndex topLeftIndex = index(0, 0, parentIndex);
    QModelIndex rightBottomIndex = index(d->rootNode->childrenCount(), columnCount(parentIndex), parentIndex);

    QMetaObject::invokeMethod(this, "dataChanged", Qt::QueuedConnection,
                              Q_ARG(QModelIndex, topLeftIndex), Q_ARG(QModelIndex, rightBottomIndex));
}

void DFileSystemModel::selectAndRenameFile(const DUrl &fileUrl)
{
    /// TODO: 暂时放在此处实现，后面将移动到DFileService中实现。
    if (fileUrl.scheme() == DFMMD_SCHEME) { //自动整理路径特殊实现，fix bug#24715
        auto realFileUrl = MergedDesktopController::convertToRealPath(fileUrl);
        if (AppController::selectionAndRenameFile.first == realFileUrl) {
            quint64 windowId = AppController::selectionAndRenameFile.second;
            if (windowId != parent()->windowId()) {
                return;
            }

            AppController::selectionAndRenameFile = qMakePair(DUrl(), 0);
            DFMUrlBaseEvent event(this, fileUrl);
            event.setWindowId(windowId);
            emit newFileByInternal(fileUrl);
        }
    } else if (fileUrl.isVaultFile()) { //! 设置保险箱新建文件选中并重命名状态
        DUrl url = DUrl::fromLocalFile(VaultController::vaultToLocal(fileUrl));
        if (AppController::selectionAndRenameFile.first == url) {
            quint64 windowId = AppController::selectionAndRenameFile.second;

            if (windowId != parent()->windowId()) {
                return;
            }

            AppController::selectionAndRenameFile = qMakePair(DUrl(), 0);
            DFMUrlBaseEvent event(this, fileUrl);
            event.setWindowId(windowId);

            TIMER_SINGLESHOT_OBJECT(const_cast<DFileSystemModel *>(this), 100, {
                emit fileSignalManager->requestSelectRenameFile(event);
            }, event)

            emit newFileByInternal(fileUrl);
        }
    } else if (AppController::selectionAndRenameFile.first == fileUrl) {
        quint64 windowId = AppController::selectionAndRenameFile.second;

        if (windowId != parent()->windowId()) {
            return;
        }

        AppController::selectionAndRenameFile = qMakePair(DUrl(), 0);
        DFMUrlBaseEvent event(this, fileUrl);
        event.setWindowId(windowId);

        TIMER_SINGLESHOT_OBJECT(const_cast<DFileSystemModel *>(this), 100, {
            emit fileSignalManager->requestSelectRenameFile(event);
        }, event)

        emit newFileByInternal(fileUrl);
    } else if (AppController::selectionFile.first == fileUrl) {
        quint64 windowId = AppController::selectionFile.second;

        if (windowId != parent()->windowId()) {
            return;
        }

        AppController::selectionFile = qMakePair(DUrl(), 0);
        DFMUrlListBaseEvent event(parent()->parent(), DUrlList() << fileUrl);
        event.setWindowId(windowId);

        TIMER_SINGLESHOT_OBJECT(const_cast<DFileSystemModel *>(this), 100, {
            emit fileSignalManager->requestSelectFile(event);
            emit this->requestSelectFiles(event.urlList());
        }, event, this)

    } else if (AppController::multiSelectionFilesCache.second != 0) {
        quint64 winId{ AppController::multiSelectionFilesCache.second };
        if (winId == parent()->windowId() || AppController::flagForDDesktopRenameBar) { //###: flagForDDesktopRenameBar is false usually.

            if (!AppController::multiSelectionFilesCache.first.isEmpty()) {

                if (AppController::multiSelectionFilesCache.first.contains(fileUrl) == true) {

                    ++AppController::multiSelectionFilesCacheCounter;
                    if (static_cast<int>(AppController::multiSelectionFilesCacheCounter.load(std::memory_order_seq_cst)) ==
                            AppController::multiSelectionFilesCache.first.size()) {

                        DFMUrlListBaseEvent event{ parent()->parent(),  AppController::multiSelectionFilesCache.first};
                        event.setWindowId(winId);

                        ////###: clean cache!
                        AppController::multiSelectionFilesCache.first.clear();
                        AppController::multiSelectionFilesCache.second = 0;

                        ///###: make ref-counter be 0(zero).
                        AppController::multiSelectionFilesCacheCounter.store(0, std::memory_order_seq_cst);

                        ///###:request to select files which was renamed successfully.
                        QTimer::singleShot(100, [event, this] {
                            emit fileSignalManager->requestSelectFile(event);
                            emit this->requestSelectFiles(event.urlList());
                        });
                    }
                }
            }
        }
    }
}

bool DFileSystemModel::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
    Q_D(DFileSystemModel);

    if (d->beginRemoveRowsFlag) {
        return false;
    } else {
        QAbstractItemModel::beginRemoveRows(parent, first, last);
        d->beginRemoveRowsFlag = true;
        return true;
    }
}

void DFileSystemModel::endRemoveRows()
{
    Q_D(DFileSystemModel);

    d->beginRemoveRowsFlag = false;
    QAbstractItemModel::endRemoveRows();
}
//fix bug 33014,在文件或者文件很多时，切换到computer时DFileSystemModel已释放但是d->jobController未释放，发送更新消息就崩溃，在DFileSystemModel析构时释放d->jobController
bool DFileSystemModel::releaseJobController()
{
    Q_D(DFileSystemModel);

    if (d->jobController) {
        disconnect(d->jobController, &JobController::addChildren, this, &DFileSystemModel::onJobAddChildren);
        disconnect(d->jobController, &JobController::finished, this, &DFileSystemModel::onJobFinished);
        disconnect(d->jobController, &JobController::childrenUpdated, this, &DFileSystemModel::updateChildrenOnNewThread);

        if (d->jobController->isFinished()) {
            d->jobController->deleteLater();
            d->jobController = nullptr;
        } else {
            QEventLoop eventLoop;
            QPointer<DFileSystemModel> me = this;
            d->eventLoop = &eventLoop;

            connect(d->jobController, &JobController::destroyed, &eventLoop, &QEventLoop::quit);

            d->jobController->stopAndDeleteLater();

            d->jobController = nullptr;

            int code = eventLoop.exec();

            d->eventLoop = Q_NULLPTR;

            if (code != 0) {
                //                if (d->jobController) { //有时候d->jobController已销毁，会导致崩溃
                //fix bug 33007 在释放d->jobController时，eventLoop退出异常，
                //此时d->jobController有可能已经在析构了，不能调用terminate
                //                    d->jobController->terminate();
                //                    d->jobController->quit();
                //                    d->jobController.clear();
                //                    d->jobController->stopAndDeleteLater();
                //                }
                return false;
            }

            if (!me) {
                return false;
            }
        }
    }
    return true;
}

#include "moc_dfilesystemmodel.cpp"


