// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systempathutil.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <QDir>
#include <QDebug>
#include <QStandardPaths>

using namespace dfmbase;
SystemPathUtil *SystemPathUtil::instance()
{
    static SystemPathUtil util;
    return &util;
}

QString SystemPathUtil::systemPath(const QString &key)
{
    if (systemPathsMap.isEmpty())
        initialize();

    QString path { systemPathsMap.value(key) };
    if (Q_UNLIKELY(!QDir(path).exists()) && xdgDirs.contains(key)) {
        bool flag = QDir::home().mkpath(path);
        qCDebug(logDFMBase) << "mkpath" << path << flag;
    }
    return path;
}

QString SystemPathUtil::systemPathOfUser(const QString &key, const QString &user) const
{
    if (xdgDirs.contains(key))
        return "/home/" + user + "/" + key;

    return {};
}

QString SystemPathUtil::systemPathDisplayName(const QString &key) const
{
    if (systemPathDisplayNamesMap.contains(key))
        return systemPathDisplayNamesMap.value(key);

    return QString();
}

QString SystemPathUtil::systemPathDisplayNameByPath(QString path)
{
    // shorten the path from /persistent/* (if it is) to /*
    // to make sure the $HOME/* can be translated correctly
    cleanPath(&path);
    QStringList &&keys = systemPathsMap.keys();
    auto ret = std::find_if(keys.cbegin(), keys.cend(), [this, path](const QString &key) {
        return (systemPathsMap.value(key) == path);
    });
    if (ret != keys.cend())
        return systemPathDisplayName(*ret);
    return QString();
}

QString SystemPathUtil::systemPathIconName(const QString &key) const
{
    if (systemPathIconNamesMap.contains(key))
        return systemPathIconNamesMap.value(key);
    return QString();
}

QString SystemPathUtil::systemPathIconNameByPath(QString path)
{
    cleanPath(&path);

    if (isSystemPath(path)) {
        QStringList &&keys = systemPathsMap.keys();
        auto ret = std::find_if(keys.cbegin(), keys.cend(), [this, &path](const QString &key) {
            return (systemPathsMap.value(key) == path);
        });
        if (ret != keys.cend())
            return systemPathIconName(*ret);
    }
    return QString();
}

bool SystemPathUtil::isSystemPath(QString path) const
{
    cleanPath(&path);

    return systemPathsSet.contains(path);
}

bool SystemPathUtil::checkContainsSystemPath(const QList<QUrl> &urlList)
{
    if (urlList.isEmpty())
        return false;
    if (urlList.first().scheme() == Global::Scheme::kFile)
        return checkContainsSystemPathByFileUrl(urlList);

    return checkContainsSystemPathByFileInfo(urlList);
}

SystemPathUtil::SystemPathUtil(QObject *parent)
    : QObject(parent),
      xdgDirs { "Desktop", "Videos", "Music", "Pictures", "Documents", "Downloads", "Trash" }
{
    initialize();
}

SystemPathUtil::~SystemPathUtil()
{
}

void SystemPathUtil::initialize()
{
    loadSystemPaths();

    systemPathDisplayNamesMap["Home"] = tr("Home");
    systemPathDisplayNamesMap["Desktop"] = tr("Desktop");
    systemPathDisplayNamesMap["Videos"] = tr("Videos");
    systemPathDisplayNamesMap["Music"] = tr("Music");
    systemPathDisplayNamesMap["Pictures"] = tr("Pictures");
    systemPathDisplayNamesMap["Documents"] = tr("Documents");
    systemPathDisplayNamesMap["Downloads"] = tr("Downloads");
    systemPathDisplayNamesMap["Trash"] = tr("Trash");
    systemPathDisplayNamesMap["System Disk"] = tr("System Disk");
    systemPathDisplayNamesMap["Recent"] = tr("Recent");

    systemPathIconNamesMap["Home"] = "user-home";
    systemPathIconNamesMap["Desktop"] = "user-desktop";
    systemPathIconNamesMap["Videos"] = "folder-videos";
    systemPathIconNamesMap["Music"] = "folder-music";
    systemPathIconNamesMap["Pictures"] = "folder-pictures";
    systemPathIconNamesMap["Documents"] = "folder-documents";
    systemPathIconNamesMap["Downloads"] = "folder-downloads";
    systemPathIconNamesMap["Trash"] = "user-trash";
    systemPathIconNamesMap["System Disk"] = "drive-harddisk-root";
    systemPathIconNamesMap["Recent"] = "document-open-recent";
    systemPathIconNamesMap["Network"] = "network-workgroup";
}

void SystemPathUtil::mkPath(const QString &path)
{
    if (!QDir(path).exists()) {
        bool flag = QDir::home().mkpath(path);
        qCDebug(logDFMBase) << "mkpath" << path << flag;
    }
}

void SystemPathUtil::cleanPath(QString *path) const
{
    Q_ASSERT(path);
    // 这里去掉/data的目的 是让通过数据盘路径进入的用户目录下的Docunment,Vedios等文件也可以被翻译
    static const QString &userHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    const QString &homeBindPath = FileUtils::bindPathTransform(userHome, true);
    if (path->startsWith(homeBindPath)) {
        path->replace(homeBindPath, userHome);
    }

    if (path->size() > 1 && path->at(0) == '/' && path->endsWith("/")) {
        path->chop(1);
    }
}

bool SystemPathUtil::checkContainsSystemPathByFileInfo(const QList<QUrl> &urlList)
{
    for (const auto &url : urlList) {
        auto info = InfoFactory::create<FileInfo>(url);
        if (info && isSystemPath(info->pathOf(PathInfoType::kAbsoluteFilePath)))
            return true;
    }

    return false;
}

bool SystemPathUtil::checkContainsSystemPathByFileUrl(const QList<QUrl> &urlList)
{
    return std::any_of(urlList.begin(), urlList.end(),[this](const QUrl &url){
        return isSystemPath(url.path());
    });
}

void SystemPathUtil::loadSystemPaths()
{
    systemPathsMap["Home"] = StandardPaths::location(StandardPaths::kHomePath);
    systemPathsMap["Desktop"] = StandardPaths::location(StandardPaths::kDesktopPath);
    systemPathsMap["Videos"] = StandardPaths::location(StandardPaths::kVideosPath);
    systemPathsMap["Music"] = StandardPaths::location(StandardPaths::kMusicPath);
    systemPathsMap["Pictures"] = StandardPaths::location(StandardPaths::kPicturesPath);
    systemPathsMap["Documents"] = StandardPaths::location(StandardPaths::kDocumentsPath);
    systemPathsMap["Downloads"] = StandardPaths::location(StandardPaths::kDownloadsPath);
    systemPathsMap["System Disk"] = StandardPaths::location(StandardPaths::kDiskPath);

    systemPathsSet.reserve(systemPathsMap.size());

    for (const QString &key : systemPathsMap.keys()) {
        const QString &path = systemPathsMap.value(key);

        systemPathsSet << path;

        if (xdgDirs.contains(key))
            mkPath(path);
    }
}
