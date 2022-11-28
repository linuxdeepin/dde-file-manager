#ifndef WORKERDATA_H
#define WORKERDATA_H
#include "dfmplugin_fileoperations_global.h"
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <QSharedPointer>

DPFILEOPERATIONS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class WorkerData
{
public:
    WorkerData();
    quint16 dirSize { 0 };   // size of dir
    AbstractJobHandler::JobFlags jobFlags { AbstractJobHandler::JobFlag::kNoHint };   // job flag
    QMap<AbstractJobHandler::JobErrorType, AbstractJobHandler::SupportAction> errorOfAction;
    std::atomic_bool needSyncEveryRW { false };
    std::atomic_bool isFsTypeVfat { false };
    std::atomic_int64_t currentWriteSize { 0 };
    QAtomicInteger<qint64> zeroOrlinkOrDirWriteSize { 0 };   // The copy size is 0. The write statistics size of the linked file and directory
    QAtomicInteger<qint64> blockRenameWriteSize { 0 };   // The copy size is 0. The write statistics size of the linked file and directory
    QAtomicInteger<qint64> skipWriteSize { 0 };   // 跳过的文件大
    std::atomic_bool signalThread { true };
};
DPFILEOPERATIONS_END_NAMESPACE
#endif   // WORKERDATA_H
