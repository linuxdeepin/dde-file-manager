/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QObject>
#include <QIcon>
#include <QFileInfo>

#include <QDir>
#include "desktopfile.h"
#include "properties.h"
#include "durl.h"
#include "dfmglobal.h"
#include "dabstractfileinfo.h"

/**
 * @class FileUtils
 * @brief Utility class providing static helper methods for file management
 */
class FileUtils {
public:

    static QString XDG_RUNTIME_DIR;

    static bool removeRecurse(const QString &path, const QString &name);
    static void recurseFolder(const QString &path, const QString &parent,
                            QStringList *list);
    static int filesCount(const QString& dir);
    static qint64 totalSize(const QString& dir);
    static qint64 totalSize(const DUrlList &files);
    static qint64 totalSize(const DUrlList &files, const qint64& maxLimit, bool &isInLimit);
    static bool isArchive(const QString& path);
    static bool canFastReadArchive(const QString& path);
    static QStringList getApplicationNames();
    static QList<DesktopFile> getApplications();
    static QString getRealSuffix(const QString &name);
    static QIcon searchGenericIcon(const QString &category,
       const QIcon &defaultIcon = QIcon::fromTheme("unknown"));
    static QIcon searchMimeIcon(QString mime,
      const QIcon &defaultIcon = QIcon::fromTheme("unknown"));
    static QIcon searchAppIcon(const DesktopFile &app,
      const QIcon &defaultIcon = QIcon::fromTheme("application-x-executable"));
    static QString formatSize(qint64 num , bool withUnitVisible = true, int precision = 1, int forceUnit = -1, QStringList unitList = QStringList());
    static QString diskUsageString(qint64 usedSize, qint64 totalSize);
    static DUrl newDocumentUrl(const DAbstractFileInfoPointer targetDirInfo, const QString& baseName, const QString& suffix);
    static QString newDocmentName(QString targetdir, const QString& baseName, const QString& suffix);
    static bool cpTemplateFileToTargetDir(const QString& targetdir, const QString& baseName, const QString& suffix, WId windowId);

    static bool openFile(const QString& filePath);
    static bool launchApp(const QString& desktopFile, const QStringList& filePaths = {}); // open filePaths by desktopFile
    static bool launchAppByDBus(const QString& desktopFile, const QStringList& filePaths = {});
    static bool launchAppByGio(const QString& desktopFile, const QStringList& filePaths = {});

    static bool openFilesByApp(const QString& desktopFile, const QStringList &filePaths);
    static bool isFileManagerSelf(const QString& desktopFile);
    static QString defaultTerminalPath();

    static bool setBackground(const QString& pictureFilePath);

    static QString md5(const QString& data);
    static QByteArray md5(QFile *file, const QString &filePath);

    static bool isFileExecutable(const QString& path);
    static bool shouldAskUserToAddExecutableFlag(const QString& path);
    static bool isFileRunnable(const QString& path);
    static bool isFileWindowsUrlShortcut(const QString& path); /*check file is windows url shortcut*/
    static QString getInternetShortcutUrl(const QString& path);/*get InternetShortcut url of windows url shortcut*/

    static QString getFileMimetype(const QString& path);
    static bool isExecutableScript(const QString& path);
    static bool openExcutableScriptFile(const QString& path, int flag);
    static bool addExecutableFlagAndExecuse(const QString& path, int flag);
    static bool openExcutableFile(const QString& path, int flag);
    static bool runCommand(const QString& cmd, const QStringList& args, const QString& wd = QString());

    static QByteArray imageFormatName(QImage::Format f);

    static QString getFileContent(const QString &file);
    static bool writeTextFile(const QString& filePath, const QString& content);
    static void migrateConfigFileFromCache(const QString& key);
    static QMap<QString, QString> getKernelParameters();

    static DFMGlobal::MenuExtension getMenuExtension(const DUrlList& urlList);
    static bool isGvfsMountFile(const QString &filePath);
    static bool isFileExists(const QString& filePath);

    static QJsonObject getJsonObjectFromFile(const QString& filePath);
    static QJsonArray getJsonArrayFromFile(const QString& filePath);
    static bool writeJsonObjectFile(const QString& filePath, const QJsonObject& obj);
    static bool writeJsonnArrayFile(const QString& filePath, const QJsonArray& array);

    static void mountAVFS();
    static void umountAVFS();
};

#endif // FILEUTILS_H
