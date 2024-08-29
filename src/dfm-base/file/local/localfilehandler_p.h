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
    QUrl loadTemplateInfo(const QUrl &url, const QUrl &templateUrl = QUrl());

    bool doOpenFile(const QUrl &url, const QString &desktopFile = QString());
    bool doOpenFiles(const QList<QUrl> &urls, const QString &desktopFile = QString());
    bool doOpenFiles(const QMultiMap<QString, QString> &infos, const QMap<QString, QString> &mimeTypes);

    void setError(DFMIOError error);
    QUrl loadTemplateUrl(const QString &suffix);

    static void doAddRecentFile(const QVariantMap &item);
    static QVariantMap buildRecentItem(const QString &path, const DesktopFile &desktop, const QString &mimeType);
    static void addRecentFile(const QString &desktop, const QList<QString> urls,
                              const QMap<QString, QString> &mimeTypes);
    static void addRecentFile(const QString &desktop, const QList<QUrl> urls,
                              const QString &mimeType);

public:
    LocalFileHandler *q { nullptr };
    DFMIOError lastError;
    GlobalEventType lastEvent = GlobalEventType::kUnknowType;
    QList<QUrl> invalidPath;   //BUG:https://pms.uniontech.com/bug-view-259909.html,记录无效链接路径
};

}

#endif   // LOCALFILEHANDLER_P_H
