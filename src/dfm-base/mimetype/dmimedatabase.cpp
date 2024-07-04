// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmimedatabase.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>

#include <QUrl>
#include <QFileInfo>
#include <QRegularExpression>

using namespace dfmbase;

static QStringList wrongMimeTypeNames {
    Global::Mime::kTypeAppXOleStorage, Global::Mime::kTypeAppZip
};
static QStringList officeSuffixList {
    "docx", "xlsx", "pptx", "doc", "ppt", "xls", "wps"
};
static const QStringList blackList { "/sys/kernel/security/apparmor/revision", "/sys/kernel/security/apparmor/policy/revision", "/sys/power/wakeup_count", "/proc/kmsg" };

DMimeDatabase::DMimeDatabase()
{
}

QMimeType DMimeDatabase::mimeTypeForFile(const QUrl &url, QMimeDatabase::MatchMode mode) const
{
    const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(url);
    return mimeTypeForFile(fileInfo, mode);
}

QMimeType DMimeDatabase::mimeTypeForFile(const FileInfoPointer &fileInfo, QMimeDatabase::MatchMode mode) const
{
    // 如果是低速设备，则先从扩展名去获取mime信息；对于本地文件，保持默认的获取策略
    QMimeType result;

    if (!fileInfo)
        return QMimeType();

    QString path = fileInfo->pathOf(PathInfoType::kPath);
    bool isMatchExtension = mode == QMimeDatabase::MatchExtension;
    if (!isMatchExtension) {
        //fix bug 35448 【文件管理器】【5.1.2.2-1】【sp2】预览ftp路径下某个文件夹后，文管卡死,访问特殊系统文件卡死
        if (fileInfo->nameOf(NameInfoType::kFileName).endsWith(".pid") || path.endsWith("msg.lock")
            || fileInfo->nameOf(NameInfoType::kFileName).endsWith(".lock") || fileInfo->nameOf(NameInfoType::kFileName).endsWith("lockfile")) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QRegularExpression regExp("^/run/user/\\d+/gvfs/(?<scheme>\\w+(-?)\\w+):\\S*",
                                      QRegularExpression::DotMatchesEverythingOption
                                              | QRegularExpression::DontCaptureOption
                                              | QRegularExpression::OptimizeOnFirstUsageOption);
#else
            QRegularExpression regExp("^/run/user/\\d+/gvfs/(?<scheme>\\w+(-?)\\w+):\\S*",
                                      QRegularExpression::DotMatchesEverythingOption
                                              | QRegularExpression::DontCaptureOption);
#endif

            const QRegularExpressionMatch &match = regExp.match(path, 0, QRegularExpression::NormalMatch,
                                                                QRegularExpression::DontCheckSubjectStringMatchOption);

            isMatchExtension = match.hasMatch();
        } else {
            // filemanger will be blocked when blacklist contais the filepath.
            QString filePath = fileInfo->pathOf(PathInfoType::kAbsoluteFilePath);
            if (fileInfo->isAttributes(OptInfoType::kIsSymLink)) {
                filePath = fileInfo->pathOf(PathInfoType::kSymLinkTarget);
            }
            isMatchExtension = blackList.contains(filePath);
        }
    }

    if (isMatchExtension || DeviceUtils::isLowSpeedDevice(QUrl::fromLocalFile(path))) {
        result = QMimeDatabase::mimeTypeForFile(fileInfo->pathOf(PathInfoType::kFilePath), QMimeDatabase::MatchExtension);
    } else {
        result = QMimeDatabase::mimeTypeForFile(fileInfo->pathOf(PathInfoType::kFilePath), mode);
    }

    // temporary dirty fix, once WPS get installed, the whole mimetype database thing get fscked up.
    // we used to patch our Qt to fix this issue but the patch no longer works, we don't have time to
    // look into this issue ATM.
    // https://bugreports.qt.io/browse/QTBUG-71640
    // https://codereview.qt-project.org/c/qt/qtbase/+/244887
    // `file` command works but libmagic didn't even comes with any pkg-config support..

    if (officeSuffixList.contains(fileInfo->nameOf(NameInfoType::kSuffix))
        && wrongMimeTypeNames.contains(result.name())) {
        QList<QMimeType> results = QMimeDatabase::mimeTypesForFileName(fileInfo->nameOf(NameInfoType::kFileName));
        if (!results.isEmpty()) {
            return results.first();
        }
    }
    return result;
}

QMimeType DMimeDatabase::mimeTypeForFile(const QString &fileName, QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs) const
{
    if (!inod.isEmpty() && inodMimetypeCache.contains(inod)) {
        return inodMimetypeCache.value(inod);
    }
    return mimeTypeForFile(QFileInfo(fileName), mode, inod, isGvfs);
}

QMimeType DMimeDatabase::mimeTypeForFile(const QFileInfo &fileInfo, QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs) const
{
    Q_UNUSED(isGvfs)
    // 如果是低速设备，则先从扩展名去获取mime信息；对于本地文件，保持默认的获取策略
    bool canCache = !inod.isEmpty();
    if (!inod.isEmpty() && inodMimetypeCache.contains(inod)) {
        return inodMimetypeCache.value(inod);
    }
    if (fileInfo.isDir()) {
        return QMimeDatabase::mimeTypeForFile(QFileInfo("/home"), mode);
    }
    QMimeType result;
    QString path = fileInfo.path();

    bool isMatchExtension = mode == QMimeDatabase::MatchExtension;

    //fix bug 35448 【文件管理器】【5.1.2.2-1】【sp2】预览ftp路径下某个文件夹后，文管卡死,访问特殊系统文件卡死
    if (!isMatchExtension) {
        if (fileInfo.fileName().endsWith(".pid") || path.endsWith("msg.lock")
            || fileInfo.fileName().endsWith(".lock") || fileInfo.fileName().endsWith("lockfile")) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QRegularExpression regExp("^/run/user/\\d+/gvfs/(?<scheme>\\w+(-?)\\w+):\\S*",
                                      QRegularExpression::DotMatchesEverythingOption
                                              | QRegularExpression::DontCaptureOption
                                              | QRegularExpression::OptimizeOnFirstUsageOption);
#else
            QRegularExpression regExp("^/run/user/\\d+/gvfs/(?<scheme>\\w+(-?)\\w+):\\S*",
                                      QRegularExpression::DotMatchesEverythingOption
                                              | QRegularExpression::DontCaptureOption);
#endif

            const QRegularExpressionMatch &match = regExp.match(path, 0, QRegularExpression::NormalMatch,
                                                                QRegularExpression::DontCheckSubjectStringMatchOption);

            isMatchExtension = match.hasMatch();
        } else {
            // filemanger will be blocked when blacklist contais the filepath.
            // fix task #29124, bug #108805
            QString filePath = fileInfo.absoluteFilePath();
            if (fileInfo.isSymLink()) {
                filePath = fileInfo.symLinkTarget();
            }
            isMatchExtension = blackList.contains(filePath);
        }
    }
    if (isMatchExtension || DeviceUtils::isLowSpeedDevice(QUrl::fromLocalFile(path))) {
        result = QMimeDatabase::mimeTypeForFile(fileInfo, QMimeDatabase::MatchExtension);
    } else {
        result = QMimeDatabase::mimeTypeForFile(fileInfo, mode);
    }

    // temporary dirty fix, once WPS get installed, the whole mimetype database thing get fscked up.
    // we used to patch our Qt to fix this issue but the patch no longer works, we don't have time to
    // look into this issue ATM.
    // https://bugreports.qt.io/browse/QTBUG-71640
    // https://codereview.qt-project.org/c/qt/qtbase/+/244887
    // `file` command works but libmagic didn't even comes with any pkg-config support..

    if (officeSuffixList.contains(fileInfo.suffix()) && wrongMimeTypeNames.contains(result.name())) {
        QList<QMimeType> results = QMimeDatabase::mimeTypesForFileName(fileInfo.fileName());
        if (!results.isEmpty()) {
            if (canCache) {
                const_cast<DMimeDatabase *>(this)->inodMimetypeCache.insert(inod, results.first());
            }
            return results.first();
        }
    }
    if (canCache) {
        const_cast<DMimeDatabase *>(this)->inodMimetypeCache.insert(inod, result);
    }
    return result;
}

QMimeType DMimeDatabase::mimeTypeForUrl(const QUrl &url) const
{
    if (dfmbase::FileUtils::isLocalFile(url))
        return mimeTypeForFile(url);

    return QMimeDatabase::mimeTypeForUrl(url);
}
