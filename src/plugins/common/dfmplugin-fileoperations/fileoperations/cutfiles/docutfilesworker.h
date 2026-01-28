// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUTFILESWORKER_H
#define DOCUTFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
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
    bool initArgs() override;
    void onUpdateProgress() override;
    void endWork() override;

    bool cutFiles();
    bool doCutFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &targetPathInfo, bool *skip);
    bool renameFileByHandler(const DFileInfoPointer &sourceInfo, const DFileInfoPointer &targetInfo, bool *skip);
    DFileInfoPointer trySameDeviceRename(const DFileInfoPointer &sourceInfo, const DFileInfoPointer &targetPathInfo,
                                         const QString fileName, bool *ok, bool *skip);

    void emitCompleteFilesUpdatedNotify(const qint64 &writCount);
    bool doMergDir(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip);

private:
    bool checkSymLink(const DFileInfoPointer &fromInfo);
    bool checkSelf(const DFileInfoPointer &fromInfo);
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOCUTFILESWORKER_H
