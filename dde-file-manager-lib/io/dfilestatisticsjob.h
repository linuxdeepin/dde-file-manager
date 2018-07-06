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
    ~DFileStatisticsJob();

    enum State {
        StoppedState,
        RunningState,
        PausedState
    };

    Q_ENUM(State)

    enum FileHint {
        FollowSymlink = 0x01
    };

    Q_ENUM(FileHint)
    Q_DECLARE_FLAGS(FileHints, FileHint)

    State state() const;
    FileHints fileHints() const;

    qint64 totalSize() const;
    int filesCount() const;
    int directorysCount() const;

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

private:
    void run() override;

    using QThread::start;
    using QThread::terminate;
    using QThread::quit;
    using QThread::exit;

    QScopedPointer<DFileStatisticsJobPrivate> d_ptr;
};

DFM_END_NAMESPACE

#endif // DFILESTATISTICSJOB_H
