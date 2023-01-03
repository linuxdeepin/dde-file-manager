/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef BURNHELPER_H
#define BURNHELPER_H

#include "dfmplugin_burn_global.h"

#include <QUrl>
#include <QDir>
#include <QVariantMap>
#include <QFileInfo>

namespace dfmplugin_burn {

namespace Persistence {
inline constexpr char kBurnStateGroup[] { "BurnState" };
inline constexpr char kWoringKey[] { "Working" };
inline constexpr char kIdKey[] { "id" };
}   // namespace Persistence

class BurnHelper
{
public:
    static int showOpticalBlankConfirmationDialog();
    static int showOpticalImageOpSelectionDialog();
    static QUrl localStagingFile(QString dev);
    static QUrl localStagingFile(const QUrl &dest);
    static QUrl fromBurnFile(const QString &dev);
    static QString parseXorrisoErrorMessage(const QStringList &msg);
    static QString burnDestDevice(const QUrl &url);
    static QString burnFilePath(const QUrl &url);
    static QList<QVariantMap> discDataGroup();
    static void updateBurningStateToPersistence(const QString &id, const QString &dev, bool working);
    static void mapStagingFilesPath(const QList<QUrl> &srcList, const QList<QUrl> &targetList);
    static bool isBurnEnabled();
    static bool burnIsOnLocalStaging(const QUrl &url);
    static QFileInfoList localFileInfoList(const QString &path);
    static QFileInfoList localFileInfoListRecursive(const QString &path, QDir::Filters filters = (QDir::Files | QDir::NoSymLinks));
};

}   // namespace dfmplugin_burn

#endif   // BURNHELPER_H
