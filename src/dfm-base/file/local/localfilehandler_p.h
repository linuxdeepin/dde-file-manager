// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEHANDLER_P_H
#define LOCALFILEHANDLER_P_H

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/applaunchutils.h>

#include <dfm-io/error/error.h>
#include <dfm-io/dfile.h>

#include <QString>

#include <optional>

class QUrl;
class QString;

namespace dfmbase {

class DesktopFile;
class LocalFileHandler;
class LocalFileHandlerPrivate
{
    friend class LocalFileHandler;

public:
    explicit LocalFileHandlerPrivate(LocalFileHandler *handler);
    ~LocalFileHandlerPrivate() = default;

public:
    bool launchApp(const QString &desktopFile, const QStringList &filePaths = {});
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

private:
    // 处理单个文件的打开
    bool handleSingleFileOpen(QUrl &fileUrl, const QUrl &sourceUrl, bool &result);

    // 解析符号链接,返回最终指向的文件URL
    // 处理相对路径,避免循环链接,支持网络文件检查
    std::optional<QUrl> resolveSymlink(const QUrl &url);

    // 处理可执行文件
    bool handleExecutableFile(const QUrl &fileUrl, bool *result);

    // 收集要打开的文件路径
    void collectFilePath(const QUrl &fileUrl, QList<QUrl> *pathList);

public:
    LocalFileHandler *q { nullptr };
    DFMIOError lastError;
    GlobalEventType lastEvent = GlobalEventType::kUnknowType;
    QList<QUrl> invalidPath;   // BUG:259909,记录无效链接路径
};

}

#endif   // LOCALFILEHANDLER_P_H
