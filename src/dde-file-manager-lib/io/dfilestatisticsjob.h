// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILESTATISTICSJOB_H
#define DFILESTATISTICSJOB_H

#include <dfmglobal.h>

#include <QObject>

DFM_BEGIN_NAMESPACE

class DFileStatisticsJobPrivate;
class DFileStatisticsJob : public QThread
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFileStatisticsJob)

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(FileHints fileHints READ fileHints WRITE setFileHints)

public:
    explicit DFileStatisticsJob(QObject *parent = nullptr);
    ~DFileStatisticsJob() override;

    enum State {
        StoppedState,
        RunningState,
        PausedState
    };

    Q_ENUM(State)

    enum FileHint {
        FollowSymlink = 0x01,
        DontSkipAVFSDStorage = 0x02,
        DontSkipPROCStorage = 0x04,
        DontSkipCharDeviceFile = 0x08,
        DontSkipBlockDeviceFile = 0x10,
        DontSkipFIFOFile = 0x20,
        DontSkipSocketFile = 0x40,
        ExcludeSourceFile = 0x80, // 不计算传入的文件列表
        SingleDepth = 0x100 // 深度为1
    };

    Q_ENUM(FileHint)
    Q_DECLARE_FLAGS(FileHints, FileHint)

    State state() const;
    FileHints fileHints() const;

    qint64 totalSize() const;
    qint64 totalProgressSize() const;
    int filesCount() const;
    int directorysCount(bool includeSelf = true) const;

public Q_SLOTS:
    void start(const DUrlList &sourceUrls);
    void stop();
    void togglePause();

    void setFileHints(FileHints fileHints);

Q_SIGNALS:
    void stateChanged(State state);
    void sizeChanged(qint64 size);
    void fileFound(const DUrl &url);
    void directoryFound(const DUrl &url);
    void dataNotify(qint64 size, int filesCount, int directoryCount);

private:
    void run() override;

    using QThread::start;
    using QThread::terminate;
    using QThread::quit;
    using QThread::exit;

    QScopedPointer<DFileStatisticsJobPrivate> d_ptr;
};

DFM_END_NAMESPACE

Q_DECLARE_OPERATORS_FOR_FLAGS(DFM_NAMESPACE::DFileStatisticsJob::FileHints)

#endif // DFILESTATISTICSJOB_H
