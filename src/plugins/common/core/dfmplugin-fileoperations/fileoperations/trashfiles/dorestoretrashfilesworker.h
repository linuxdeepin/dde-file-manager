/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DORESTORETRASHFILESWORKER_H
#define DORESTORETRASHFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <dfm-io/core/dfile.h>

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

protected:
    bool doRestoreTrashFiles();
    //check disk space available before do move job
    bool createParentDir(const AbstractFileInfoPointer &trashInfo, const AbstractFileInfoPointer &restoreInfo, AbstractFileInfoPointer &targetFileInfo, bool *result);

private:
    QString readTrashInfo(const QUrl &url);
    bool mergeDir(const QUrl &urlSource, const QUrl &urlTarget, dfmio::DFile::CopyFlag flag);

private:
    QAtomicInteger<qint64> completeFilesCount { 0 };   // move to trash success file count
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOCUTFILESWORKER_H
