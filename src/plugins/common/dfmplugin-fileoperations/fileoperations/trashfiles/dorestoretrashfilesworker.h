// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DORESTORETRASHFILESWORKER_H
#define DORESTORETRASHFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <dfm-io/dfile.h>

#include <QObject>

class QStorageInfo;
USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class DoRestoreTrashFilesWorker : public FileOperateBaseWorker
{
    friend class RestoreTrashFiles;
    Q_OBJECT
    explicit DoRestoreTrashFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoRestoreTrashFilesWorker() override;

protected:
    bool doWork() override;
    bool statisticsFilesSize() override;
    bool initArgs() override;
    void onUpdateProgress() override;

protected:
    bool translateUrls();
    bool doRestoreTrashFiles();
    //check disk space available before do move job
    DFileInfoPointer createParentDir(const QUrl &fromUrl, const DFileInfoPointer &restoreInfo, bool *result);
    DFileInfoPointer checkRestoreInfo(const QUrl &url);

private:
    bool mergeDir(const QUrl &urlSource, const QUrl &urlTarget, dfmio::DFile::CopyFlag flag);

private:
    QAtomicInteger<qint64> completeFilesCount { 0 };   // move to trash success file count
    QList<QUrl> handleSourceFiles;   // List of all handled files
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOCUTFILESWORKER_H
