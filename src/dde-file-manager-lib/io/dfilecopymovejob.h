/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef DFILECOPYMOVEJOB_H
#define DFILECOPYMOVEJOB_H

#include <QObject>
#include <dfmglobal.h>
#include "dabstractfileinfo.h"

typedef QExplicitlySharedDataPointer<DAbstractFileInfo> DAbstractFileInfoPointer;

DFM_BEGIN_NAMESPACE

class DFileCopyMoveJobPrivate;
class DFileCopyMoveJob : public QThread
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_d_ptr), DFileCopyMoveJob)

    Q_PROPERTY(Mode mode READ mode WRITE setMode)
//    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(Error error READ error NOTIFY errorChanged)
    Q_PROPERTY(FileHints fileHints READ fileHints WRITE setFileHints)
    Q_PROPERTY(QString errorString READ errorString CONSTANT)

public:
    enum Mode {
        CopyMode,
        MoveMode,
        CutMode,
        RemoteMode,
        UnknowMode = 255
    };

    Q_ENUM(Mode)

    enum State {
        StoppedState,
        RunningState,
        PausedState,
        SleepState,
        IOWaitState // 可能会长时间等待io的状态
    };

    Q_ENUM(State)

    enum RefineState {
        NoRefine,
        RefineLocal,
        RefineBlock
    };

    Q_ENUM(RefineState)

    enum RefineCopyProccessSate {
        NoProccess,
        OpenFromFileProccessOver,
        ReadFileProccessOver,
    };

    Q_ENUM(RefineCopyProccessSate)


    enum Error {
        NoError,
        CancelError,
        PermissionError,
        SpecialFileError,
        FileExistsError,
        DirectoryExistsError,
        OpenError,
        ReadError,
        WriteError,
        SymlinkError,
        MkdirError,
        ResizeError,
        MmapError,
        RemoveError,
        RenameError,
        SymlinkToGvfsError,
        UnknowUrlError,
        NonexistenceError,
        IntegrityCheckingError,
        FileSizeTooBigError,
        NotEnoughSpaceError,
        TargetReadOnlyError,
        TargetIsSelfError,
        NotSupportedError,
        PermissionDeniedError,
        SeekError,
        UnknowError,
    };

    Q_ENUM(Error)

    enum FileHint {
        NoHint = 0x00,
        FollowSymlink = 0x01,
        Attributes = 0x02, // 复制文件时携带它的扩展属性
        AttributesOnly = 0x04, // 只复制文件的扩展属性
//        CreateParents = 0x08, // 复制前在目标文件夹创建来源文件路径中的所有目录
        RemoveDestination = 0x10, // 复制文件前先删除已存在的
        ResizeDestinationFile = 0x20, // 复制文件前对目标文件执行resize操作
        DontIntegrityChecking = 0x40, // 复制文件时不进行完整性校验
        DontFormatFileName = 0x80, // 不要自动处理文件名中的非法字符
        DontSortInode = 0x100, // 不要对目录中的文件按inode排序
        ForceDeleteFile = 0x200 // 强制删除文件夹(去除文件夹的只读权限)
    };

    Q_ENUM(FileHint)
    Q_DECLARE_FLAGS(FileHints, FileHint)

    enum Action {
        NoAction = 0x00,
        RetryAction = 0x01,
        ReplaceAction = 0x02,
        MergeAction = 0x04,
        SkipAction = 0x08,
        CoexistAction = 0x10,
        CancelAction = 0x20,
        EnforceAction = 0x40
    };

    Q_ENUM(Action)
    Q_DECLARE_FLAGS(Actions, Action)

    enum GvfsRetryType {
        GvfsRetryNoAction = 0x01,
        GvfsRetrySkipAction = 0x02,
        GvfsRetryCancelAction = 0x03,
        GvfsRetryDefault = 0x04
    };

    Q_ENUM(GvfsRetryType)
    Q_DECLARE_FLAGS(GvfsRetryTypes, Action)

    class Handle
    {
    public:
        virtual ~Handle() {}
        virtual Action handleError(DFileCopyMoveJob *job, Error error,
                                   const DAbstractFileInfoPointer sourceInfo,
                                   const DAbstractFileInfoPointer targetInfo) = 0;
        virtual QString getNewFileName(DFileCopyMoveJob *job, const DAbstractFileInfoPointer sourceInfo);
        virtual QString getNonExistsFileName(const DAbstractFileInfoPointer sourceInfo,
                                             const DAbstractFileInfoPointer targetDirectory);
    };

    explicit DFileCopyMoveJob(QObject *parent = nullptr);
    ~DFileCopyMoveJob() override;

    Handle *errorHandle() const;
    void setErrorHandle(Handle *handle, QThread *threadOfHandle = nullptr);

    void setActionOfErrorType(Error error, Action action);

    Mode mode() const;
    State state() const;
    Error error() const;
    FileHints fileHints() const;
    QString errorString() const;

    DUrlList sourceUrlList() const;
    DUrlList targetUrlList() const;
    DUrl targetUrl() const;

    bool fileStatisticsIsFinished() const;
    qint64 totalDataSize() const;
    int totalFilesCount() const;
    QList<QPair<DUrl, DUrl> > completedFiles() const;
    QList<QPair<DUrl, DUrl> > completedDirectorys() const;
    //获取当前是否可以显示进度条
    bool isCanShowProgress() const;

    void setRefine(const RefineState &refinestat);

    void setCurTrashData(QVariant fileNameList);
    //设置当前拷贝显示了进度条
    void setProgressShow(const bool &isShow);
    void copyBigFileOnDiskJobWait();
    void copyBigFileOnDiskJobRun();

    static Actions supportActions(Error error);

public Q_SLOTS:
    void start(const DUrlList &sourceUrls, const DUrl &targetUrl);
    void stop();
    void togglePause();

    void setMode(Mode mode);
    void setFileHints(FileHints fileHints);
    void taskDailogClose();

Q_SIGNALS:
    // 此类工作在一个新的线程中，信号不要以引用的方式传递参数，容易出现一些较为诡异的崩溃问题
    // 问题现象一般为，槽函数中的参数是个无效的对象(内存中已被销毁)，不知是否和槽函数形参也为
    // 引用类型有关
    void stateChanged(State state);
    void errorCanClear();
    void errorChanged(Error error);
    void currentJobChanged(const DUrl from, const DUrl to, const bool iserroeroc);
    void finished(const DUrl from, const DUrl to);
    void completedFilesCountChanged(int count);
    void fileStatisticsFinished();
    void progressChanged(qreal progress, qint64 writeData);
    void currentFileProgressChanged(qreal progress, qint64 writeData);
    void speedUpdated(qint64 speed);
    void stopAllGioDervic();

    // 拷贝进度条显示100%时提示数据同步 信号
    void sendDataSyncing(QString syncTip, QString over);

protected:
    DFileCopyMoveJob(DFileCopyMoveJobPrivate *dd, QObject *parent);

    void run() override;

    QScopedPointer<DFileCopyMoveJobPrivate> d_d_ptr;

private:
    using QThread::start;
    using QThread::terminate;
    using QThread::quit;
    using QThread::exit;

    Q_PRIVATE_SLOT(d_func(), void _q_updateProgress())
};

DFM_END_NAMESPACE

Q_DECLARE_OPERATORS_FOR_FLAGS(DFM_NAMESPACE::DFileCopyMoveJob::Actions)
Q_DECLARE_METATYPE(DFM_NAMESPACE::DFileCopyMoveJob::Error)

#endif // DFILECOPYMOVEJOB_H
