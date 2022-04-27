/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DOCOPYFILESWORKER_H
#define DOCOPYFILESWORKER_H

#include "dfm_common_service_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "fileoperations/fileoperationutils/abstractworker.h"
#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include "dfm-base/utils/threadcontainer.hpp"
#include "dfm-base/file/local/localfilehandler.h"
#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"

#include <dfm-io/core/dfile.h>

#include <QObject>
#include <QMutex>
#include <QTime>
#include <QQueue>
#include <QThread>
#include <QThreadPool>
#include <QMultiMap>

class QWaitCondition;
DSC_BEGIN_NAMESPACE
USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
class StorageInfo;
class DoCopyFilesWorker : public FileOperateBaseWorker
{
    friend class CopyFiles;
    Q_OBJECT
    explicit DoCopyFilesWorker(QObject *parent = nullptr);

    struct SmallFileThreadCopyInfo
    {
        AbstractFileInfoPointer fromInfo { nullptr };
        AbstractFileInfoPointer toInfo { nullptr };
    };

public:
    virtual ~DoCopyFilesWorker() override;

protected:
    bool doWork() override;
    void stop() override;
    void setStat(const AbstractJobHandler::JobState &stat) override;
    bool initArgs() override;
    void endWork() override;
    AbstractJobHandler::SupportActions supportActions(const AbstractJobHandler::JobErrorType &error) override;

protected:
    bool copyFiles();
    bool doCopyFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                    bool *workContinue);
    bool creatSystemLink(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *workContinue);
    bool checkAndCopyFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo, bool *workContinue);
    bool checkAndCopyDir(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo, bool *workContinue);
    bool doThreadPoolCopyFile();

private slots:
    void onUpdateProccess() override;

private:
    QSharedPointer<QQueue<QSharedPointer<SmallFileThreadCopyInfo>>> smallFileThreadCopyInfoQueue;   // copy small file thread information Queue
    QSharedPointer<QMutex> smallFileThreadCopyInfoQueueMutex { nullptr };   // copy small file thread information Queue's mutex
    QSharedPointer<QThreadPool> threadPool { nullptr };   // copy small file thread pool
};

DSC_END_NAMESPACE

#endif   // DOCOPYFILESWORKER_H
