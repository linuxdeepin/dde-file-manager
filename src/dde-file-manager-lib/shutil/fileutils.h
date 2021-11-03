/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "desktopinfo.h"

/**
 * @class FileUtils
 * @brief Utility class providing static helper methods for file management
 */
class FileUtils
{
public:

    static QString XDG_RUNTIME_DIR;
    static QStringList CURRENT_ISGVFSFILE_PATH;

    static bool removeRecurse(const QString &path, const QString &name);
    static bool isAncestorUrl(const DUrl &ancestor, const DUrl &url);
    static bool isNetworkAncestorUrl(const DUrl &dest, const bool isDestGvfs,
                                     const DUrl &source, const bool isSourceGvfs);
    static void recurseFolder(const QString &path, const QString &parent,
                              QStringList *list);
    static int filesCount(const QString &dir);
    static QStringList filesList(const QString &dir);
    static qint64 singleDirSize(const DUrl &url);
    static qint64 totalSize(const QString &targetFile);
    static qint64 totalSize(const DUrlList &files);
    //计算本地文件的大小
    static qint64 totalSize(const DUrlList &files, qint32 &dirSize,qint32 &fileCount);
    static qint64 totalSize(const DUrlList &files, const qint64 &maxLimit, bool &isInLimit);
    static bool isArchive(const QString &path);
    static bool canFastReadArchive(const QString &path);
    static QStringList getApplicationNames();
    static QString getRealSuffix(const QString &name);
    static QIcon searchGenericIcon(const QString &category,
                                   const QIcon &defaultIcon = QIcon::fromTheme("unknown"));
    static QIcon searchMimeIcon(QString mime,
                                const QIcon &defaultIcon = QIcon::fromTheme("unknown"));
    static QIcon searchAppIcon(const DesktopFile &app,
                               const QIcon &defaultIcon = QIcon::fromTheme("application-x-executable"));
    static QString formatSize(qint64 num, bool withUnitVisible = true, int precision = 1, int forceUnit = -1, QStringList unitList = QStringList());
    static QString diskUsageString(quint64 &usedSize, quint64 &totalSize, QString strVolTag = "");
    static QString defaultOpticalSize(const QString &tagName, quint64 &usedSize, quint64 &totalSize);
    static DUrl newDocumentUrl(const DAbstractFileInfoPointer targetDirInfo, const QString &baseName, const QString &suffix);
    static QString newDocmentName(QString targetdir, const QString &baseName, const QString &suffix);
    static bool cpTemplateFileToTargetDir(const QString &targetdir, const QString &baseName, const QString &suffix, WId windowId);

    static bool openFile(const QString &filePath);
    static bool openFiles(const QStringList &filePaths);
    static bool openEnterFiles(const QStringList &filePaths);
    static bool launchApp(const QString &desktopFile, const QStringList &filePaths = {}); // open filePaths by desktopFile
    static bool launchAppByDBus(const QString &desktopFile, const QStringList &filePaths = {});
    static bool launchAppByGio(const QString &desktopFile, const QStringList &filePaths = {});

    static bool openFilesByApp(const QString &desktopFile, const QStringList &filePaths);
    static bool isFileManagerSelf(const QString &desktopFile);
    static QString defaultTerminalPath();

    static bool setBackground(const QString &pictureFilePath);

    static QString md5(const QString &fpath);
    static QByteArray md5(QFile *file, const QString &filePath);

    static bool isFileExecutable(const QString &path);
    static bool shouldAskUserToAddExecutableFlag(const QString &path);
    static bool isFileRunnable(const QString &path);
    static bool isFileWindowsUrlShortcut(const QString &path); /*check file is windows url shortcut*/
    static QString getInternetShortcutUrl(const QString &path);/*get InternetShortcut url of windows url shortcut*/

    static QString getFileMimetype(const QString &path);
    static bool isExecutableScript(const QString &path);
    static bool openExcutableScriptFile(const QString &path, int flag);
    static bool addExecutableFlagAndExecuse(const QString &path, int flag);
    static bool openExcutableFile(const QString &path, int flag);
    static bool runCommand(const QString &cmd, const QStringList &args, const QString &wd = QString());

    static void mkpath(const DUrl &path);
    static QString displayPath(const QString &pathStr);

    static QByteArray imageFormatName(QImage::Format f);

    static QString getFileContent(const QString &file);
    static bool writeTextFile(const QString &filePath, const QString &content);
    static void migrateConfigFileFromCache(const QString &key);
    static QMap<QString, QString> getKernelParameters();

    static DFMGlobal::MenuExtension getMenuExtension(const DUrlList &urlList);
    static bool isGvfsMountFile(const QString &filePath, const bool &isEx = false);
    static bool isFileExists(const QString &filePath);

    static QJsonObject getJsonObjectFromFile(const QString &filePath);
    static QJsonArray getJsonArrayFromFile(const QString &filePath);
    static bool writeJsonObjectFile(const QString &filePath, const QJsonObject &obj);
    static bool writeJsonnArrayFile(const QString &filePath, const QJsonArray &array);

    static void mountAVFS();
    static void umountAVFS();

    static bool isDesktopFile(const QString &filePath);
    static bool isDesktopFile(const QFileInfo &fileInfo);
    static bool isDesktopFile(const QString &filePath, QMimeType &mimetype);
    static bool isDesktopFile(const QFileInfo &fileInfo, QMimeType &mimetyp);
    static void addRecentFile(const QString &filePath, const DesktopFile &desktopFile, const QString &mimetype);

    // 启用deepin-compressor追加压缩
    static bool appendCompress(const DUrl &toUrl, const DUrlList &fromUrlList);
    //获取内存叶大小
    static int getMemoryPageSize();
    //判断是否是挂载ftp目录下的文件
    static bool isFtpFile(const DUrl &url);
    //判断是否是操作系统同盘的本地文件
    static bool isFileOnDisk(const QString &path);
    //获取线程cpu核个数
    static qint32 getCpuProcessCount();
    // 获取smbd服务是否打开
    static bool isSambaServiceRunning();
    // 判断当前的网络挂载路径是否挂载了
    static bool isNetworkUrlMounted(const DUrl &url);

    static DUrl smbFileUrl(const QString &filePath);
    static bool isSmbUnmountedFile(const DUrl &url);
};

#endif // FILEUTILS_H
