/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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

#ifndef DFMSTANDARDPATHS_H
#define DFMSTANDARDPATHS_H

#include "dfm-base/dfm_base_global.h"

#include <QString>
#include <QStandardPaths>

class QUrl;

DFMBASE_BEGIN_NAMESPACE

class DFMStandardPaths
{
public:
    enum StandardLocation {
        kTrashPath,
        kTrashExpungedPath,
        kTrashFilesPath,
        kTrashInfosPath,
        kTranslationPath,
        kTemplatesPath,
        kMimeTypePath,
        kPluginsPath,
        kExtensionsPath,
        kExtensionsAppEntryPath,
        kThumbnailPath,
        kThumbnailFailPath,
        kThumbnailLargePath,
        kThumbnailNormalPath,
        kThumbnailSmallPath,
        kApplicationConfigPath, /* ~/.config */
        kApplicationSharePath, /* /usr/share/dde-file-manager */
        kRecentPath,
        kHomePath,
        kDesktopPath,
        kVideosPath,
        kMusicPath,
        kPicturesPath,
        kDocumentsPath,
        kDownloadsPath,
        kCachePath, /* ~/.cache/dde-file-manager */
        kDiskPath,
        kNetworkRootPath,
        kUserShareRootPath,
        kComputerRootPath,
        kRoot,
        kVault
    };

    static QString location(StandardLocation type);
    static QString fromStandardUrl(const QUrl &standarUrl);
    static QUrl toStandardUrl(const QString &localPath);

#ifdef QMAKE_TARGET
    static QString getConfigPath();
#endif
    static QString getCachePath();

private:
    DFMStandardPaths();
};

DFMBASE_END_NAMESPACE

#endif   // DFMSTANDARDPATHS_H
