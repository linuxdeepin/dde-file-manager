// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
