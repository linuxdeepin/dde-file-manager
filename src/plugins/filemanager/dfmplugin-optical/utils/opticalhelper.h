// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALHELPER_H
#define OPTICALHELPER_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/clipboard.h>

#include <QIcon>
#include <QRegularExpression>

inline constexpr char kDiscburnStaging[] { "discburn" };
inline constexpr char kBurnSegOndisc[] { "disc_files" };
inline constexpr char kBurnSegStaging[] { "staging_files" };

namespace dfmplugin_optical {

class OpticalHelper : public QObject
{
    Q_DISABLE_COPY(OpticalHelper)
public:
    static OpticalHelper *instance();

    static QString scheme();
    static QIcon icon();
    static QString iconString();
    static QUrl discRoot(const QString &dev);
    static QUrl localStagingRoot();
    static QUrl localStagingFile(const QUrl &dest);
    static QUrl localStagingFile(QString dev);
    static QUrl localDiscFile(const QUrl &dest);
    static QString burnDestDevice(const QUrl &url);
    static QString burnFilePath(const QUrl &url);
    static bool burnIsOnDisc(const QUrl &url);
    static bool burnIsOnStaging(const QUrl &url);
    static QUrl tansToBurnFile(const QUrl &in);
    static QUrl transDiscRootById(const QString &id);
    static bool isSupportedUDFVersion(const QString &version);
    static bool isSupportedUDFMedium(int type);
    static void createStagingFolder(const QString &path);
    static bool isDupFileNameInPath(const QString &path, const QUrl &url);
    static bool isBurnEnabled();
    static QStringList allOpticalDiscMountPoints();
    static QString findMountPoint(const QString &path);

    bool isTransparent(const QUrl &url, DFMGLOBAL_NAMESPACE::TransparentStatus *status);

private:
    explicit OpticalHelper(QObject *parent = nullptr);

    static QRegularExpression burnRxp();
};

}

#endif   // OPTICALHELPER_H
