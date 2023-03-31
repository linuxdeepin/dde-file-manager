// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEHANDLER_P_H
#define LOCALFILEHANDLER_P_H

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <dfm-io/error/error.h>
#include <dfm-io/dfile.h>

#include <QString>

class QUrl;
class QString;

namespace dfmbase {

class DesktopFile;
class LocalFileHandler;
class LocalFileHandlerPrivate
{
public:
    explicit LocalFileHandlerPrivate(LocalFileHandler *handler);
    ~LocalFileHandlerPrivate() = default;

public:
    bool launchApp(const QString &desktopFile, const QStringList &filePaths = {});
    bool launchAppByDBus(const QString &desktopFile, const QStringList &filePaths = {});
    bool launchAppByGio(const QString &desktopFile, const QStringList &filePaths = {});

    bool isFileManagerSelf(const QString &desktopFile);
    bool isInvalidSymlinkFile(const QUrl &url);
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
    void loadTemplateInfo(const QUrl &url, const QUrl &templateUrl = QUrl());

    bool doOpenFile(const QUrl &url, const QString &desktopFile = QString());
    bool doOpenFiles(const QList<QUrl> &urls, const QString &desktopFile = QString());

    void setError(DFMIOError error);

public:
    LocalFileHandler *q { nullptr };
    DFMIOError lastError;
    GlobalEventType lastEvent = GlobalEventType::kUnknowType;
};

}

#endif   // LOCALFILEHANDLER_P_H
