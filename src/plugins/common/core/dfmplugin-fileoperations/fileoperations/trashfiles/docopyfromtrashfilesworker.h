/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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
#ifndef DOCOPYFROMTRASHFILESWORKER_H
#define DOCOPYFROMTRASHFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <dfm-io/core/dfile.h>

#include <QObject>

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
class DoCopyFromTrashFilesWorker : public FileOperateBaseWorker
{
    friend class CopyFromTrashTrashFiles;
    Q_OBJECT
    explicit DoCopyFromTrashFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoCopyFromTrashFilesWorker() override;

protected:
    bool doWork() override;
    bool statisticsFilesSize() override;
    bool initArgs() override;

protected:
    bool doOperate();
    bool createParentDir(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo, AbstractFileInfoPointer &targetFileInfo, bool *result);
    bool copyFile(const QUrl &urlSource, const QUrl &urlTarget, dfmio::DFile::CopyFlag flag);

private:
    QAtomicInteger<qint64> completeFilesCount { 0 };
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOCOPYFROMTRASHFILESWORKER_H
