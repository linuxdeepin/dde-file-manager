/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/desktopfile.h"

DFMBASE_BEGIN_NAMESPACE

class FileUtils
{
public:
    struct FilesSizeInfo
    {
        qint64 totalSize { 0 };
        quint16 dirSize { 0 };
        quint32 fileCount { 0 };
        QList<QUrl> allFiles;
    };

public:
    static bool mkdir(const QUrl &url, const QString &dirName, QString *errorString = nullptr);
    static bool touch(const QUrl &url, const QString &fileName, QString *errorString = nullptr);
    static QString formatSize(qint64 num, bool withUnitVisible = true, int precision = 1, int forceUnit = -1, QStringList unitList = QStringList());
    static int supportedMaxLength(const QString &fileSystem);
    static bool isGvfsFile(const QUrl &url);
    static QString preprocessingFileName(QString name);

    //check if is trash/computer desktop file containing Deepin_id of dde-trash/dde-computer
    static bool isDesktopFile(const QUrl &url);
    static bool isTrashDesktopFile(const QUrl &url);
    static bool isComputerDesktopFile(const QUrl &url);
    static bool isHomeDesktopFile(const QUrl &url);
    static bool isSameDevice(const QUrl &url1, const QUrl &url2);
    static bool isSameFile(const QUrl &url1, const QUrl &url2);
    static bool isLowSpeedDevice(const QUrl &url);
    static bool isLocalDevice(const QUrl &url);
    static bool isCdRomDevice(const QUrl &url);
    static bool trashIsEmpty();
    static bool isHigherHierarchy(const QUrl &urlBase, const QUrl &urlCompare);

    static QMap<QUrl, QUrl> fileBatchReplaceText(const QList<QUrl> &originUrls, const QPair<QString, QString> &pair);
    static QMap<QUrl, QUrl> fileBatchAddText(const QList<QUrl> &originUrls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &pair);
    static QMap<QUrl, QUrl> fileBatchCustomText(const QList<QUrl> &originUrls, const QPair<QString, QString> &pair);
    static QString cutString(const QString &text, int dataByteSize, const QTextCodec *codec);
    static QString nonExistSymlinkFileName(const QUrl &fileUrl, const QUrl &parentUrl = QUrl());
    static QString toUnicode(const QByteArray &data, const QString &fileName = QString());
    static QByteArray detectCharset(const QByteArray &data, const QString &fileName = QString {});

    static quint16 getMemoryPageSize();
    // icon
    static QIcon searchAppIcon(const DesktopFile &app, const QIcon &defaultIcon = QIcon::fromTheme("application-x-executable"));
    static QIcon searchGenericIcon(const QString &category, const QIcon &defaultIcon = QIcon::fromTheme("unknown"));
    static QIcon searchMimeIcon(QString mime, const QIcon &defaultIcon = QIcon::fromTheme("unknown"));

    static void cacheCopyingFileUrl(const QUrl &url);
    static void removeCopyingFileUrl(const QUrl &url);
    static bool containsCopyingFileUrl(const QUrl &url);

    static void notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType type, const QUrl &url);
    static bool compareString(const QString &str1, const QString &str2, Qt::SortOrder order);

    static QString dateTimeFormat();
    static bool setBackGround(const QString &pictureFilePath);
    static QString nonExistFileName(AbstractFileInfoPointer fromInfo, AbstractFileInfoPointer targetDir, std::function<bool(const QString &)> functionCheck = nullptr);

private:
    static QMutex cacheCopyingMutex;
    static QSet<QUrl> copyingUrl;
};

class DesktopAppUrl
{
public:
    static QUrl trashDesktopFileUrl();
    static QUrl computerDesktopFileUrl();
    static QUrl homeDesktopFileUrl();
};

class Match
{
public:
    explicit Match(const QString &group);
    bool match(const QString &path, const QString &name);

private:
    QList<QPair<QString, QString>> patternList;
};

DFMBASE_END_NAMESPACE

typedef QSharedPointer<DFMBASE_NAMESPACE::FileUtils::FilesSizeInfo> SizeInfoPointer;

#endif   // FILEUTILS_H
