/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

#ifndef LOCALFILEHANDLER_H
#define LOCALFILEHANDLER_H

#include "dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <dfm-io/error/error.h>
#include <dfm-io/core/dfile.h>

#include <QString>
#include <QFileDevice>

class QUrl;
class QString;

namespace dfmbase {

class DesktopFile;

class LocalFileHandlerPrivate;
class LocalFileHandler
{
public:
    LocalFileHandler();
    ~LocalFileHandler();

    bool touchFile(const QUrl &url, const QUrl &tempUrl = QUrl());
    bool mkdir(const QUrl &dir);
    bool rmdir(const QUrl &url);
    bool renameFile(const QUrl &url, const QUrl &newUrl, const bool needCheck = true);
    bool openFile(const QUrl &file);
    bool openFiles(const QList<QUrl> &files);
    bool openFileByApp(const QUrl &file, const QString &appDesktop);
    bool openFilesByApp(const QList<QUrl> &files, const QString &appDesktop);
    bool createSystemLink(const QUrl &sourcfile, const QUrl &link);
    bool setPermissions(const QUrl &url, QFileDevice::Permissions permissions);
    bool setPermissionsRecursive(const QUrl &url, QFileDevice::Permissions permissions);
    bool moveFile(const QUrl &sourceUrl, const QUrl &destUrl, DFMIO::DFile::CopyFlag flag = DFMIO::DFile::CopyFlag::kNone);
    bool copyFile(const QUrl &sourceUrl, const QUrl &destUrl, DFMIO::DFile::CopyFlag flag = DFMIO::DFile::CopyFlag::kNone);
    bool trashFile(const QUrl &url);
    bool deleteFile(const QUrl &file);
    bool setFileTime(const QUrl &url, const QDateTime &accessDateTime, const QDateTime &lastModifiedTime);

    bool renameFilesBatch(const QMap<QUrl, QUrl> &urls, QMap<QUrl, QUrl> &successUrls);
    bool doHiddenFileRemind(const QString &name, bool *checkRule = nullptr);

    QString defaultTerminalPath();
    GlobalEventType lastEventType();
    QString errorString();
    DFMIOErrorCode errorCode();

private:
    QScopedPointer<LocalFileHandlerPrivate> d;
};

}

#endif   // LOCALFILEHANDLER_H
