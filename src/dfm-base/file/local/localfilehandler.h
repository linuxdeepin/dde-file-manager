// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEHANDLER_H
#define LOCALFILEHANDLER_H

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <dfm-io/error/error.h>
#include <dfm-io/dfile.h>

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

    QUrl touchFile(const QUrl &url, const QUrl &tempUrl = QUrl());
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
    QString trashFile(const QUrl &url);
    bool deleteFile(const QUrl &url);
    bool deleteFileRecursive(const QUrl &url);
    bool setFileTime(const QUrl &url, const QDateTime &accessDateTime, const QDateTime &lastModifiedTime);
    bool renameFilesBatch(const QMap<QUrl, QUrl> &urls, QMap<QUrl, QUrl> &successUrls);
    bool doHiddenFileRemind(const QString &name, bool *checkRule = nullptr);

    QString defaultTerminalPath();
    GlobalEventType lastEventType();
    QString errorString();
    DFMIOErrorCode errorCode();
    QList<QUrl> getInvalidPath();

private:
    QScopedPointer<LocalFileHandlerPrivate> d;
};

}

#endif   // LOCALFILEHANDLER_H
