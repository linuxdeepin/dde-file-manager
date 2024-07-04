// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/desktopfile.h>

namespace dfmbase {

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
    static QString formatSize(qint64 num, bool withUnitVisible = true, int precision = 1, int forceUnit = -1, QStringList unitList = QStringList());
    static int supportedMaxLength(const QString &fileSystem);
    static bool isGvfsFile(const QUrl &url);
    static bool isMtpFile(const QUrl &url);
    static bool isGphotoFile(const QUrl &url);
    static QString preprocessingFileName(QString name);
    static bool processLength(const QString &srcText, int srcPos, int maxLen, bool useCharCount, QString &dstText, int &dstPos);
    static bool isContainProhibitPath(const QList<QUrl> &urls);

    // check if is trash/computer desktop file containing Deepin_id of dde-trash/dde-computer
    static bool isDesktopFile(const QUrl &url);
    static bool isDesktopFileSuffix(const QUrl &url);
    static bool isDesktopFileInfo(const FileInfoPointer &info);

    static bool isTrashDesktopFile(const QUrl &url);
    static bool isComputerDesktopFile(const QUrl &url);
    static bool isHomeDesktopFile(const QUrl &url);
    static bool isSameDevice(const QUrl &url1, const QUrl &url2);
    static bool isSameFile(const QUrl &url1, const QUrl &url2,
                           const Global::CreateFileInfoType infoCache = Global::CreateFileInfoType::kCreateFileInfoAuto);
    static bool isLocalDevice(const QUrl &url);
    static bool isCdRomDevice(const QUrl &url);
    static bool trashIsEmpty();
    static QUrl trashRootUrl();
    static bool isTrashFile(const QUrl &url);
    static bool isTrashRootFile(const QUrl &url);
    static bool isHigherHierarchy(const QUrl &urlBase, const QUrl &urlCompare);
    static bool isLocalFile(const QUrl &url);
    static int getFileNameLength(const QUrl &url, const QString &name);

    static QMap<QUrl, QUrl> fileBatchReplaceText(const QList<QUrl> &originUrls, const QPair<QString, QString> &pair);
    static QMap<QUrl, QUrl> fileBatchAddText(const QList<QUrl> &originUrls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &pair);
    static QMap<QUrl, QUrl> fileBatchCustomText(const QList<QUrl> &originUrls, const QPair<QString, QString> &pair);
    static QString cutFileName(const QString &name, int maxLength, bool useCharCount);
    static QString nonExistSymlinkFileName(const QUrl &fileUrl, const QUrl &parentUrl = QUrl());
    static QString toUnicode(const QByteArray &data, const QString &fileName = QString());
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    static QByteArray detectCharset(const QByteArray &data, const QString &fileName = QString {});
#endif
    static quint16 getMemoryPageSize();
    static qint32 getCpuProcessCount();

    static void cacheCopyingFileUrl(const QUrl &url);
    static void removeCopyingFileUrl(const QUrl &url);
    static bool containsCopyingFileUrl(const QUrl &url);

    static void notifyFileChangeManual(DFMBASE_NAMESPACE::Global::FileNotifyType type, const QUrl &url);
    static bool isNumOrChar(const QChar ch);
    static bool isNumber(const QChar ch);
    static bool isSymbol(const QChar ch);
    static bool compareByStringEx(const QString &str1, const QString &str2);
    static QString numberStr(const QString &str, int pos);
    static bool compareString(const QString &str1, const QString &str2, Qt::SortOrder order);

    static QString encryptString(const QString &str);
    static QString decryptString(const QString &str);

    static QString dateTimeFormat();
    static bool setBackGround(const QString &pictureFilePath);
    static QString nonExistFileName(FileInfoPointer fromInfo, FileInfoPointer targetDir);

    // If toDevice is true, convert the path to the device name
    // otherwise convert the path to the mount point name
    static QString bindPathTransform(const QString &path, bool toDevice);
    static int dirFfileCount(const QUrl &url);
    static bool fileCanTrash(const QUrl &url);
    static QUrl bindUrlTransform(const QUrl &url);
    static QString trashPathToNormal(const QString &trash);
    static QString normalPathToTrash(const QString &normal);
    static bool supportLongName(const QUrl &url);

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

}

typedef QSharedPointer<DFMBASE_NAMESPACE::FileUtils::FilesSizeInfo> SizeInfoPointer;

#endif   // FILEUTILS_H
