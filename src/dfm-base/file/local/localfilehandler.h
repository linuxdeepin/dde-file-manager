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

class LocalFileHandler
{
public:
    LocalFileHandler();
    virtual ~LocalFileHandler();
    virtual bool touchFile(const QUrl &url);
    virtual bool mkdir(const QUrl &dir);
    virtual bool rmdir(const QUrl &url);
    virtual bool renameFile(const QUrl &url, const QUrl &newUrl);
    virtual bool renameFileBatchReplace(const QList<QUrl> &urls, const QPair<QString, QString> &pair, QMap<QUrl, QUrl> &successUrls);
    virtual bool renameFileBatchAppend(const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &pair, QMap<QUrl, QUrl> &successUrls);
    virtual bool renameFileBatchCustom(const QList<QUrl> &urls, const QPair<QString, QString> &pair, QMap<QUrl, QUrl> &successUrls);
    virtual bool openFile(const QUrl &file);
    virtual bool openFiles(const QList<QUrl> &files);
    virtual bool openFileByApp(const QUrl &file, const QString &appDesktop);
    virtual bool openFilesByApp(const QList<QUrl> &files, const QString &appDesktop);
    virtual bool createSystemLink(const QUrl &sourcfile, const QUrl &link);
    virtual bool setPermissions(const QUrl &url, QFileDevice::Permissions permissions);
    virtual bool setPermissionsRecursive(const QUrl &url, QFileDevice::Permissions permissions);
    virtual bool moveFile(const QUrl &sourceUrl, const QUrl &destUrl, DFMIO::DFile::CopyFlag flag = DFMIO::DFile::CopyFlag::kNone);
    virtual bool copyFile(const QUrl &sourceUrl, const QUrl &destUrl, DFMIO::DFile::CopyFlag flag = DFMIO::DFile::CopyFlag::kNone);
    virtual bool trashFile(const QUrl &url);
    virtual bool deleteFile(const QUrl &file);
    virtual bool setFileTime(const QUrl &url, const QDateTime &accessDateTime, const QDateTime &lastModifiedTime);

    QString defaultTerminalPath();
    QString errorString();
    DFMIOErrorCode errorCode();
    DFMBASE_NAMESPACE::GlobalEventType lastEventType();

    bool renameFilesBatch(const QMap<QUrl, QUrl> &urls, QMap<QUrl, QUrl> &successUrls);

private:
    bool launchApp(const QString &desktopFile, const QStringList &filePaths = {});
    bool launchAppByDBus(const QString &desktopFile, const QStringList &filePaths = {});
    bool launchAppByGio(const QString &desktopFile, const QStringList &filePaths = {});

    bool isFileManagerSelf(const QString &desktopFile);
    bool isSmbUnmountedFile(const QUrl &url);
    QUrl smbFileUrl(const QString &filePath);
    QString getFileMimetypeFromGio(const QUrl &url);
    void addRecentFile(const QString &filePath, const DesktopFile &desktopFile, const QString &mimetype);
    QString getFileMimetype(const QUrl &url);

    bool isExecutableScript(const QString &path);
    bool isFileExecutable(const QString &path);
    bool openExcutableScriptFile(const QString &path, int flag);
    bool openExcutableFile(const QString &path, int flag);
    bool isFileRunnable(const QString &path);
    bool shouldAskUserToAddExecutableFlag(const QString &path);
    bool addExecutableFlagAndExecuse(const QString &path, int flag);
    bool isFileWindowsUrlShortcut(const QString &path);
    QString getInternetShortcutUrl(const QString &path);

    bool doOpenFile(const QUrl &url, const QString &desktopFile = QString());
    bool doOpenFiles(const QList<QUrl> &urls, const QString &desktopFile = QString());

private:
    void setError(DFMIOError error);

    DFMIOError lastError;
    DFMBASE_NAMESPACE::GlobalEventType lastEvent = DFMBASE_NAMESPACE::GlobalEventType::kUnknowType;
};

}

#endif   // LOCALFILEHANDLER_H
