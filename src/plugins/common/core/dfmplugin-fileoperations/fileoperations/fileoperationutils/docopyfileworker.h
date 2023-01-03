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

#ifndef DOCOPYFILEWORKER_H
#define DOCOPYFILEWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "workerdata.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/file/local/localfilehandler.h"

#include <QObject>

#include <dfm-io/core/dfile.h>

class QWaitCondition;
class QMutex;
USING_IO_NAMESPACE
DPFILEOPERATIONS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class DoCopyFileWorker : public QObject
{
    Q_OBJECT
public:
    enum : u_int8_t {
        kNormal,
        kPasued,
        kStoped,
    };

public:
    explicit DoCopyFileWorker(const QSharedPointer<WorkerData> &data, QObject *parent = nullptr);
    ~DoCopyFileWorker() override;
    // main thread using
    void pause();
    void resume();
    void stop();
    void operateAction(const AbstractJobHandler::SupportAction action);
    bool doCopyFilePractically(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo,
                               bool *skip);

signals:
    void ErrorFinished();
    void CompleteSize(const int size);
    void currentTask(const QUrl source, const QUrl target);
    void errorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType error,
                     const quint64 id, const QString &errorMsg = QString());
    void retryErrSuccess(const quint64 id);
    void copyFile(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo);
public slots:
    void doFileCopy(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer toInfo);

private:
    bool stateCheck();
    void workerWait();
    bool actionOperating(const AbstractJobHandler::SupportAction action, const qint64 size, bool *skip);
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                           const AbstractJobHandler::JobErrorType &error, const QString &errorMsg = QString());

    void readAheadSourceFile(const AbstractFileInfoPointer &fileInfo);
    bool createFileDevices(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                           QSharedPointer<DFMIO::DFile> &fromeFile, QSharedPointer<DFMIO::DFile> &toFile,
                           bool *skip);
    bool createFileDevice(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                          const AbstractFileInfoPointer &needOpenInfo, QSharedPointer<DFMIO::DFile> &file,
                          bool *skip);
    bool openFiles(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                   const QSharedPointer<DFMIO::DFile> &fromeFile, const QSharedPointer<DFMIO::DFile> &toFile,
                   bool *skip);
    bool openFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                  const QSharedPointer<DFMIO::DFile> &file, const DFMIO::DFile::OpenFlags &flags,
                  bool *skip);
    bool resizeTargetFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                          const QSharedPointer<DFMIO::DFile> &file, bool *skip);
    bool doReadFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                    const QSharedPointer<DFMIO::DFile> &fromDevice,
                    char *data, const qint64 &blockSize, qint64 &readSize, bool *skip);
    bool doWriteFile(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                     const QSharedPointer<DFMIO::DFile> &toDevice,
                     const char *data, const qint64 readSize, bool *skip);
    void setTargetPermissions(const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo);
    bool verifyFileIntegrity(const qint64 &blockSize, const ulong &sourceCheckSum,
                             const AbstractFileInfoPointer &fromInfo, const AbstractFileInfoPointer &toInfo,
                             QSharedPointer<DFMIO::DFile> &toFile);
    void checkRetry();
    bool isStoped();

private:
    QSharedPointer<QWaitCondition> waitCondition { nullptr };
    QSharedPointer<DFMBASE_NAMESPACE::LocalFileHandler> localFileHandler { nullptr };   // file base operations handler
    QSharedPointer<QMutex> mutex { nullptr };
    std::atomic_int8_t state { kNormal };
    AbstractJobHandler::SupportAction currentAction { AbstractJobHandler::SupportAction::kNoAction };   // current action
    QSharedPointer<WorkerData> workData { nullptr };
    std::atomic_bool retry { false };
};
DPFILEOPERATIONS_END_NAMESPACE
#endif   // DOCOPYFILEWORKER_H
