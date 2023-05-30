// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUTFILESWORKER_H
#define DOCUTFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/abstractworker.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <dfm-io/dfile.h>

#include <QObject>

DPFILEOPERATIONS_BEGIN_NAMESPACE
class StorageInfo;

class DoCutFilesWorker : public FileOperateBaseWorker
{
    friend class CutFiles;
    Q_OBJECT
    explicit DoCutFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoCutFilesWorker() override;

protected:
    bool doWork() override;
    void stop() override;
    bool initArgs() override;
    void onUpdateProgress() override;

    bool cutFiles();
    bool doCutFile(const FileInfoPointer &fromInfo, const FileInfoPointer &targetPathInfo);
    bool doRenameFile(const FileInfoPointer &sourceInfo, const FileInfoPointer &targetPathInfo, FileInfoPointer &toInfo, const QString fileName, bool *ok);
    bool renameFileByHandler(const FileInfoPointer &sourceInfo, const FileInfoPointer &targetInfo);

    void emitCompleteFilesUpdatedNotify(const qint64 &writCount);

private:
    bool checkSymLink(const FileInfoPointer &fromInfo);
    bool checkSelf(const FileInfoPointer &fromInfo);
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOCUTFILESWORKER_H
