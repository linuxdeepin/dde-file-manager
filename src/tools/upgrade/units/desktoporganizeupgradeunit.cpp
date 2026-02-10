// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktoporganizeupgradeunit.h"

#include <QStandardPaths>
#include <QSettings>
#include <QDir>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

using namespace dfm_upgrade;

DesktopOrganizeUpgradeUnit::DesktopOrganizeUpgradeUnit()
    : UpgradeUnit()
{
    auto cfgRoots = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    QString cfgRoot = cfgRoots.isEmpty() ? "" : cfgRoots.first();
    cfgPath = cfgRoot.isEmpty()
            ? ""
            : cfgRoot + QString("/deepin/dde-desktop/ddplugin-organizer.conf");
}

QString DesktopOrganizeUpgradeUnit::name()
{
    return "DesktopUpgradeUnit";
}

bool DesktopOrganizeUpgradeUnit::initialize(const QMap<QString, QString> &args)
{
    if (cfgPath.isEmpty()) {
        qCWarning(logToolUpgrade) << "no config path found, stop upgrade desktop organizer";
        return false;
    }

    QSettings sets(cfgPath, QSettings::IniFormat);
    if (sets.value("Version", "").toString().isEmpty())
        return true;

    return false;
}

bool DesktopOrganizeUpgradeUnit::upgrade()
{
    qCDebug(logToolUpgrade) << "about to remove file" << cfgPath;
    int ret = ::remove(cfgPath.toStdString().c_str());
    return ret == 0;
}
