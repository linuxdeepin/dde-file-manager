/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef FILEEXTENDEDATTRIBUTESTETHREAD_H
#define FILEEXTENDEDATTRIBUTESTETHREAD_H

#include "dfm-base/dfm_base_global.h"

#include <QThread>
#include <QUrl>
DFMBASE_BEGIN_NAMESPACE
class FileExtendAttribThreadPrivate;
class FileExtendAttribThread : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(FileExtendAttribThread)
public:
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

    explicit FileExtendAttribThread(QObject *parent = nullptr);

    virtual ~FileExtendAttribThread() override;

    void startThread(const QList<QUrl> &urls);

    void stopThread();

    void dirSizeProcess(const QUrl &url);

signals:
    void sigDirSizeChange(qint64 size);

private:
    void run() override;

private:
    FileExtendAttribThreadPrivate *fileExtendAttribThreadPrivate { nullptr };
};
DFMBASE_END_NAMESPACE
#endif   // FILEEXTENDEDATTRIBUTESTETHREAD_H
