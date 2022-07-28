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
#ifndef DODELETEFILESWORKER_H
#define DODELETEFILESWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "fileoperations/fileoperationutils/abstractworker.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QObject>

DPFILEOPERATIONS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class DoDeleteFilesWorker : public AbstractWorker
{
    friend class DeleteFiles;
    Q_OBJECT
    explicit DoDeleteFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoDeleteFilesWorker() override;

protected:
    bool doWork() override;
    void stop() override;
    void onUpdateProgress() override;

protected:
    bool deleteAllFiles();
    bool deleteFilesOnCanNotRemoveDevice();
    bool deleteFilesOnOtherDevice();
    bool deleteFileOnOtherDevice(const QUrl &url);
    bool deleteDirOnOtherDevice(const AbstractFileInfoPointer &dir);
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const QString &errorMsg = QString());

private:
    QAtomicInteger<qint64> deleteFilesCount { 0 };
};
DPFILEOPERATIONS_END_NAMESPACE

#endif   // DODELETEFILESWORKER_H
