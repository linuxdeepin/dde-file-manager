/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
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
#ifndef FILESTATISTICSJOB_H
#define FILESTATISTICSJOB_H

#include "dfm-base/dfm_base_global.h"
#include "fileutils.h"

#include <QThread>

namespace dfmbase {

class FileStatisticsJobPrivate;
class FileStatisticsJob : public QThread
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(FileHints fileHints READ fileHints WRITE setFileHints)
    QScopedPointer<FileStatisticsJobPrivate> d;

public:
    explicit FileStatisticsJob(QObject *parent = nullptr);
    ~FileStatisticsJob() override;

    enum State {
        kStoppedState,
        kRunningState,
        kPausedState
    };

    Q_ENUM(State)

    enum FileHint {
        kNoHint = 0x0000,
        kNoFollowSymlink = 0x0001,
        kExcludeSourceFile = 0x0002,
        kSingleDepth = 0x0004,

        kDontSkipAVFSDStorage = 0x0010,
        kDontSkipPROCStorage = 0x0020,
        kDontSkipCharDeviceFile = 0x0040,
        kDontSkipBlockDeviceFile = 0x0080,
        kDontSkipFIFOFile = 0x0100,
        kDontSkipSocketFile = 0x0200,
    };

    Q_ENUM(FileHint)
    Q_DECLARE_FLAGS(FileHints, FileHint)

    State state() const;
    FileHints fileHints() const;

    qint64 totalSize() const;
    qint64 totalProgressSize() const;
    int filesCount() const;
    int directorysCount(bool includeSelf = true) const;

    SizeInfoPointer getFileSizeInfo();

public Q_SLOTS:
    void
    start(const QList<QUrl> &sourceUrls);
    void stop();
    void togglePause();

    void setFileHints(FileHints fileHints);

Q_SIGNALS:
    void stateChanged(State state);
    void sizeChanged(qint64 size);   // emit frequently, suggest use 'dataNotify' if data fuzzy
    void dataNotify(qint64 size, int filesCount, int directoryCount);

private:
    void run() override;

    using QThread::exit;
    using QThread::quit;
    using QThread::start;
    using QThread::terminate;

private:
    void setSizeInfo();
    void statistcsOtherFileSystem();
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(DFMBASE_NAMESPACE::FileStatisticsJob::FileHints)

#endif   // FILESTATISTICSJOB_H
