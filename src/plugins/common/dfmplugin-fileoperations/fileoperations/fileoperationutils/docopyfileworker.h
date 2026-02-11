// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <dfm-io/doperator.h>

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
        kPaused,
        kStopped,
    };

    enum class NextDo : u_int8_t {
        kDoCopyCurrentOp,   // 继续执行当前的写或者读操作
        kDoCopyCurrentFile,   // 继续执行当前文件拷贝
        kDoCopyReDoCurrentFile,   // 重新执行当前文件的拷贝
        kDoCopyNext,   // 继续执行下一个文件的拷贝
        kDoCopyErrorAddCancel,   // 当前拷贝出错，退出拷贝
        kDoCopyFallback,   // copy_file_range失败，需要fallback到其他方法
    };

    struct ProgressData
    {
        QUrl copyFile;
        QSharedPointer<WorkerData> data { nullptr };
    };

    enum class WriteMode {
        Normal,
        Direct   // O_DIRECT mode
    };

    struct FileWriter
    {
        int fd;
        WriteMode mode;
        size_t alignment;

        FileWriter(int fd = -1, WriteMode mode = WriteMode::Normal)
            : fd(fd), mode(mode), alignment(4096) { }   // 4K alignment for O_DIRECT
    };

public:
    explicit DoCopyFileWorker(const QSharedPointer<WorkerData> &data, QObject *parent = nullptr);
    ~DoCopyFileWorker() override;
    // main thread using
    void pause();
    void resume();
    void stop();
    void operateAction(const AbstractJobHandler::SupportAction action);
    // normal copy
    [[nodiscard]] NextDo doCopyFilePractically(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo,
                                               bool *skip);
    // O_DIRECT copy for safe sync mode
    [[nodiscard]] NextDo doCopyFileWithDirectIO(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo,
                                                bool *skip);
    // Traditional DFMIO copy
    [[nodiscard]] NextDo doCopyFileTraditional(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo,
                                               bool *skip);
    // normal copy
    [[nodiscard]] NextDo doCopyFileByRange(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo,
                                           bool *skip);
    // small file copy
    void doFileCopy(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo);
    // copy file by dfmio
    bool doDfmioFileCopy(const DFileInfoPointer fromInfo, const DFileInfoPointer toInfo, bool *skip);
signals:
    void ErrorFinished();
    void CompleteSize(const int size);
    void currentTask(const QUrl source, const QUrl target);
    void errorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error,
                     const bool isTo, const quint64 id, const QString &errorMsg,
                     const bool allUsErrorMsg);
    void retryErrSuccess(const quint64 id);

private:   // file copy
    bool stateCheck();
    void workerWait();
    bool actionOperating(const AbstractJobHandler::SupportAction action, const qint64 size, bool *skip);
    AbstractJobHandler::SupportAction doHandleErrorAndWait(const QUrl &from, const QUrl &to,
                                                           const AbstractJobHandler::JobErrorType &error,
                                                           const bool isTo = false,
                                                           const QString &errorMsg = QString());

    void readAheadSourceFile(const DFileInfoPointer &fileInfo);
    bool createFileDevices(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                           QSharedPointer<DFMIO::DFile> &fromeFile, QSharedPointer<DFMIO::DFile> &toFile,
                           bool *skip);
    bool createFileDevice(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                          const DFileInfoPointer &needOpenInfo, QSharedPointer<DFMIO::DFile> &file,
                          bool *skip);
    bool openFiles(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                   const QSharedPointer<DFMIO::DFile> &fromeFile, const QSharedPointer<DFMIO::DFile> &toFile,
                   bool *skip);
    bool openFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                  const QSharedPointer<DFMIO::DFile> &file, const DFMIO::DFile::OpenFlags &flags,
                  bool *skip);
    bool resizeTargetFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                          const QSharedPointer<DFMIO::DFile> &file, bool *skip);
    NextDo doReadFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                      const QSharedPointer<DFMIO::DFile> &fromDevice,
                      char *data, const qint64 &blockSize, qint64 &readSize, bool *skip);
    NextDo doWriteFile(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                       const QSharedPointer<DFMIO::DFile> &toDevice, const QSharedPointer<DFMIO::DFile> &fromDevice,
                       const char *data, const qint64 readSize, bool *skip);
    NextDo doWriteFileErrorRetry(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                                 const QSharedPointer<DFMIO::DFile> &toDevice, const QSharedPointer<DFMIO::DFile> &fromDevice, const qint64 readSize, bool *skip,
                                 const qint64 currentPos,
                                 const qint64 &surplusSize, qint64 &curWrite);
    void setTargetPermissions(const QUrl &fromUrl, const QUrl &toUrl);
    bool verifyFileIntegrity(const qint64 &blockSize, const ulong &sourceCheckSum,
                             const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                             QSharedPointer<DFMIO::DFile> &toFile);
    void checkRetry();
    bool isStopped();
    int openFileBySys(const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo,
                      const int flags, bool *skip, const bool isSource = true);

    // O_DIRECT support methods
    FileWriter openDestinationFile(const QString &dest, WriteMode preferredMode);
    FileWriter reopenDestinationFileForResume(const QString &dest, WriteMode preferredMode);
    char *allocateAlignedBuffer(size_t size, size_t alignment);
    bool handlePauseResume(FileWriter &writer, const QString &dest, bool *skip);
    NextDo actionToNextDo(AbstractJobHandler::SupportAction action, qint64 size, bool *skip);
    bool shouldFallbackFromCopyFileRange(int errorCode) const;
    AbstractJobHandler::JobErrorType mapSystemErrorToJobError(int systemErrno, bool isWriteError);

public:
    static void progressCallback(int64_t current, int64_t total, void *progressData);

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
    DThreadList<QSharedPointer<dfmio::DOperator>> fileOps;
};
DPFILEOPERATIONS_END_NAMESPACE
#endif   // DOCOPYFILEWORKER_H
