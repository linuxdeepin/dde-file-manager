// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOMOVETOTRASHFILESWORKER_H
#define DOMOVETOTRASHFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>

#include <dfm-io/dfile.h>

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class StorageInfo;
class DoMoveToTrashFilesWorker : public FileOperateBaseWorker
{
    friend class MoveToTrashFiles;
    Q_OBJECT
    explicit DoMoveToTrashFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoMoveToTrashFilesWorker() override;

protected:
    bool doWork() override;
    bool statisticsFilesSize() override;
    void onUpdateProgress() override;

protected:
    bool doMoveToTrash();
    bool isCanMoveToTrash(const QUrl &url, bool *result);
    QUrl trashTargetUrl(const QUrl &url);
    AbstractJobHandler::SupportAction doHandleErrorNoSpace(const QUrl &url);
    bool canWriteFile(const QUrl &url) const;

private:
    FileInfoPointer targetFileInfo { nullptr };   // target file information
    QAtomicInteger<qint64> completeFilesCount { 0 };   // move to trash success file count
    qint8 isSameDisk { -1 };   // the source file and trash files is in same disk
    QString trashLocalDir;   // the trash file locak dir
    QSharedPointer<StorageInfo> trashStorageInfo { nullptr };   // target file's device infor
    QMap<QString, QString> fstabMap;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOMOVETOTRASHFILESWORKER_H
