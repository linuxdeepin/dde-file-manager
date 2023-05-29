// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCOPYFILEWORKER_H
#define DOCOPYFILEWORKER_H

#include "dfmplugin_fileoperations_global.h"
#include "workerdata.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-io/dfile.h>

#include <QObject>

#include <fcntl.h>

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
    void skipMemcpyBigFile(const QUrl url);
    void operateAction(const AbstractJobHandler::SupportAction action);
    bool doCopyFilePractically(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                               bool *skip);
    void doFileCopy(const FileInfoPointer fromInfo, const FileInfoPointer toInfo);
    void readExblockFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo);
    void writeExblockFile();
    void doMemcpyLocalBigFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, char *dest, char *source, size_t size);
signals:
    void ErrorFinished();
    void CompleteSize(const int size);
    void currentTask(const QUrl source, const QUrl target);
    void errorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                     const bool isTo, const quint64 id, const QString &errorMsg,
                     const bool allUsErrorMsg);
    void retryErrSuccess(const quint64 id);
    void skipCopyLocalBigFile(const QUrl fromUrl);

private:   // file copy
    bool stateCheck();
    void workerWait();
    bool actionOperating(const AbstractJobHandler::SupportAction action, const qint64 size, bool *skip);
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const bool isTo = false,
                                                           const QString &errorMsg = QString());

    void readAheadSourceFile(const FileInfoPointer fileInfo);
    bool createFileDevices(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                           QSharedPointer<DFMIO::DFile> &fromeFile, QSharedPointer<DFMIO::DFile> &toFile,
                           bool *skip);
    bool createFileDevice(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                          const FileInfoPointer needOpenInfo, QSharedPointer<DFMIO::DFile> &file,
                          bool *skip);
    bool openFiles(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                   const QSharedPointer<DFMIO::DFile> &fromeFile, const QSharedPointer<DFMIO::DFile> &toFile,
                   bool *skip);
    bool openFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                  const QSharedPointer<DFMIO::DFile> &file, const DFMIO::DFile::OpenFlags &flags,
                  bool *skip);
    bool resizeTargetFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                          const QSharedPointer<DFMIO::DFile> &file, bool *skip);
    bool doReadFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                    const QSharedPointer<DFMIO::DFile> &fromDevice,
                    char *data, const qint64 &blockSize, qint64 &readSize, bool *skip);
    bool doWriteFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                     const QSharedPointer<DFMIO::DFile> &toDevice,
                     const char *data, const qint64 readSize, bool *skip);
    void setTargetPermissions(const FileInfoPointer fromInfo, const FileInfoPointer toInfo);
    bool verifyFileIntegrity(const qint64 &blockSize, const ulong &sourceCheckSum,
                             const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                             QSharedPointer<DFMIO::DFile> &toFile);
    void checkRetry();
    bool isStopped();

private:   // block file copy
    //清理当前拷贝信息
    void releaseCopyInfo(const BlockFileCopyInfoPointer &info);
    bool writeBlockFile(const BlockFileCopyInfoPointer &info, bool *skip);
    void syncBlockFile(const BlockFileCopyInfoPointer &info, bool doOnce);
    bool doWriteBlockFileCopy(const BlockFileCopyInfoPointer blockFileInfo);
    int doOpenFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo, const bool isTo,
                   const int openFlag, bool *skip);
    BlockFileCopyInfoPointer doReadExBlockFile(const FileInfoPointer fromInfo, const FileInfoPointer toInfo,
                                               const int fd, bool *skip);
    void createExBlockFileCopyInfo(const FileInfoPointer fromInfo,
                                   const FileInfoPointer toInfo,
                                   const qint64 currentPos,
                                   const bool closeFlag,
                                   const qint64 size,
                                   char *buffer = nullptr,
                                   const bool isDir = false,
                                   const QFileDevice::Permissions permission = QFileDevice::Permission::ReadOwner);

private:
    QSharedPointer<QWaitCondition> waitCondition { nullptr };
    QSharedPointer<DFMBASE_NAMESPACE::LocalFileHandler> localFileHandler { nullptr };   // file base operations handler
    QSharedPointer<QMutex> mutex { nullptr };
    std::atomic_int8_t state { kNormal };
    AbstractJobHandler::SupportAction currentAction { AbstractJobHandler::SupportAction::kNoAction };   // current action
    QSharedPointer<WorkerData> workData { nullptr };
    std::atomic_bool retry { false };
    int blockFileFd { -1 };
    QList<QUrl> skipUrls;
    QUrl memcpySkipUrl;
};
DPFILEOPERATIONS_END_NAMESPACE
#endif   // DOCOPYFILEWORKER_H
