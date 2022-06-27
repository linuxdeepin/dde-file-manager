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
        kFollowSymlink = 0x01,
        kDontSkipAVFSDStorage = 0x02,
        kDontSkipPROCStorage = 0x04,
        kDontSkipCharDeviceFile = 0x08,
        kDontSkipBlockDeviceFile = 0x10,
        kDontSkipFIFOFile = 0x20,
        kDontSkipSocketFile = 0x40,
        kExcludeSourceFile = 0x80,   // 不计算传入的文件列表
        kSingleDepth = 0x100   // 深度为1
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
    void fileFound(const QUrl &url);
    void directoryFound(const QUrl &url);
    void sizeChanged(qint64 size);
    void dataNotify(qint64 size, int filesCount, int directoryCount);

private:
    void run() override;

    using QThread::exit;
    using QThread::quit;
    using QThread::start;
    using QThread::terminate;

private:
    void setSizeInfo();
    void statistcsExtFileSystem();
    void statistcsOtherFileSystem();
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(DFMBASE_NAMESPACE::FileStatisticsJob::FileHints)

#endif   // FILESTATISTICSJOB_H
