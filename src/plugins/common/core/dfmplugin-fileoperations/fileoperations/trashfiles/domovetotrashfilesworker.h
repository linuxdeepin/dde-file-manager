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
#ifndef DOMOVETOTRASHFILESWORKER_H
#define DOMOVETOTRASHFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QObject>

#include <dfm-io/core/dfile.h>

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

protected:
    bool doMoveToTrash();
    bool isCanMoveToTrash(const QUrl &url, bool *result);

private:
    QUrl buildTrashUrl(const QString &completeBaseName, const QString &suffix);

private:
    AbstractFileInfoPointer targetFileInfo { nullptr };   // target file information
    QAtomicInteger<qint64> completeFilesCount { 0 };   // move to trash success file count
    qint8 isSameDisk { -1 };   // the source file and trash files is in same disk
    QString trashLocalDir;   // the trash file locak dir
    QSharedPointer<StorageInfo> trashStorageInfo { nullptr };   // target file's device infor
    QMap<QString, QString> fstabMap;
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DOMOVETOTRASHFILESWORKER_H
