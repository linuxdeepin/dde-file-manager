/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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

class DAbstractFileInfo;

DFM_BEGIN_NAMESPACE

class DFileCopyMoveJobPrivate;
class DFileCopyMoveJob : public QThread
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_d_ptr), DFileCopyMoveJob)

    Q_PROPERTY(Mode mode READ mode WRITE setMode)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(Error error READ error NOTIFY errorChanged)
    Q_PROPERTY(FileHints fileHints READ fileHints WRITE setFileHints)
    Q_PROPERTY(QString errorString READ errorString CONSTANT)

public:
    enum Mode {
        CopyMode,
        MoveMode
    };

    Q_ENUM(Mode)

    enum State {
        StoppedState,
        RunningState,
        PausedState,
        SleepState
    };

    Q_ENUM(State)

    enum Error {
        NoError,
        CancelError,
        PermissionError,
        FileExistsError,
        DirectoryExistsError,
        OpenError,
        ReadError,
        WriteError,
        SymlinkError,
        MkdirError,
        ResizeError,
        RemoveError,
        RenameError,
        OutOfSpaceError,
        UnknowUrlError,
        NonexistenceError,
        UnknowError
    };

    Q_ENUM(Error)

    enum FileHint {
        FollowSymlink = 0x01,
        Attributes = 0x02, // 复制文件时携带它的扩展属性
        AttributesOnly = 0x04, // 只复制文件的扩展属性
//        CreateParents = 0x08, // 复制前在目标文件夹创建来源文件路径中的所有目录
        RemoveDestination = 0x10, // 复制文件前先删除已存在的
        ResizeDestinationFile = 0x20 // 复制文件前对目标文件执行resize操作
    };

    Q_ENUM(FileHint)
    Q_DECLARE_FLAGS(FileHints, FileHint)

    enum Action {
        NoAction,
        RetryAction,
        ReplaceAction,
        MergeAction,
        SkipAction,
        CoexistAction,
        CancelAction
    };

    Q_ENUM(Action)

    class Handle {
    public:
        virtual Action handleError(DFileCopyMoveJob *job, Error error,
                                   const DAbstractFileInfo *sourceInfo,
                                   const DAbstractFileInfo *targetInfo) = 0;
        virtual QString getNewFileName(DFileCopyMoveJob *job, const DAbstractFileInfo *sourceInfo);
        virtual QString getNonExistsFileName(DFileCopyMoveJob *job, const DAbstractFileInfo *sourceInfo, const DAbstractFileInfo *targetDirectory);
    };

    explicit DFileCopyMoveJob(QObject *parent = nullptr);
    ~DFileCopyMoveJob();

    Handle *errorHandle() const;
    void setErrorHandle(Handle *handle);

    void setActionOfErrorType(Error error, Action action);

    Mode mode() const;
    State state() const;
    Error error() const;
    FileHints fileHints() const;
    QString errorString() const;

    qint64 totalDataSize() const;
    int totalFilesCount() const;
    QList<QPair<DUrl, DUrl> > completedFiles() const;
    QList<QPair<DUrl, DUrl> > completedDirectorys() const;

public Q_SLOTS:
    void start(const DUrlList &sourceUrls, const DUrl &targetUrl);
    void stop();
    void togglePause();

    void setMode(Mode mode);
    void setFileHints(FileHints fileHints);

Q_SIGNALS:
    void stateChanged(State state);
    void errorChanged(Error error);
    void finished(const DUrl &from, const DUrl &to);
    void completedFilesCountChanged(int count);
    void fileStatisticsFinished();
    void progressChanged(qreal progress, qint64 writeData);
    void currentFileProgressChanged(qreal progress, qint64 writeData);
    void speedUpdated(qreal speed);

protected:
    DFileCopyMoveJob(DFileCopyMoveJobPrivate &dd, QObject *parent);

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

#endif // DFILECOPYMOVEJOB_H
