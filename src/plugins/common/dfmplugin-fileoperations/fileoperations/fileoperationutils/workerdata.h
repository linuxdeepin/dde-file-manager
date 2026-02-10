// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKERDATA_H
#define WORKERDATA_H
#include "dfmplugin_fileoperations_global.h"
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/threadcontainer.h>

#include <QSharedPointer>
#include <QQueue>
#include <dfm-io/dfileinfo.h>

#include <fcntl.h>

DPFILEOPERATIONS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

typedef QSharedPointer<dfmio::DFileInfo> DFileInfoPointer;
class WorkerData
{
public:
    struct BlockFileCopyInfo
    {
        bool closeflag;
        bool isdir;
        QAtomicInt openFlag { O_CREAT | O_WRONLY | O_TRUNC };
        FileInfoPointer frominfo;
        FileInfoPointer toinfo;
        char *buffer;
        qint64 size;
        qint64 currentpos;
        QFileDevice::Permissions permission;
        BlockFileCopyInfo()
            : closeflag(true), isdir(false), frominfo(nullptr), toinfo(nullptr), buffer(nullptr), size(0), currentpos(0), permission(QFileDevice::ReadOwner)
        {
        }
        BlockFileCopyInfo(const BlockFileCopyInfo &other)
            : closeflag(other.closeflag), isdir(other.isdir), frominfo(other.frominfo), toinfo(other.toinfo), buffer(other.buffer), size(other.size), currentpos(other.currentpos), permission(other.permission)
        {
        }
        ~BlockFileCopyInfo()
        {
            if (buffer) {
                delete[] buffer;
                buffer = nullptr;
            }
        }
    };

    WorkerData();

    quint16 dirSize { 0 };   // size of dir
    AbstractJobHandler::JobFlags jobFlags { AbstractJobHandler::JobFlag::kNoHint };   // job flag
    QMap<AbstractJobHandler::JobErrorType, AbstractJobHandler::SupportAction> errorOfAction;
    std::atomic_bool exBlockSyncEveryWrite { false };
    std::atomic_bool isBlockDevice { false };
    std::atomic_bool isSourceFileLocal { false };   // source file on local device
    std::atomic_bool isTargetFileLocal { false };   // target file on local device
    std::atomic_int64_t currentWriteSize { 0 };
    QAtomicInteger<qint64> zeroOrlinkOrDirWriteSize { 0 };   // The copy size is 0. The write statistics size of the linked file and directory
    QAtomicInteger<qint64> blockRenameWriteSize { 0 };   // The copy size is 0. The write statistics size of the linked file and directory
    QAtomicInteger<qint64> skipWriteSize { 0 };   // 跳过的文件大
    QAtomicInteger<qint64> completeFileCount { 0 };   // copy complete file count
    std::atomic_bool singleThread { true };
    DThreadMap<QUrl, qint64> everyFileWriteSize;
    DThreadList<QSharedPointer<DPFILEOPERATIONS_NAMESPACE::WorkerData::BlockFileCopyInfo>> blockCopyInfoQueue;
};
DPFILEOPERATIONS_END_NAMESPACE
using BlockFileCopyInfoPointer = QSharedPointer<DPFILEOPERATIONS_NAMESPACE::WorkerData::BlockFileCopyInfo>;
Q_DECLARE_METATYPE(QSharedPointer<DPFILEOPERATIONS_NAMESPACE::WorkerData::BlockFileCopyInfo>)
#endif   // WORKERDATA_H
