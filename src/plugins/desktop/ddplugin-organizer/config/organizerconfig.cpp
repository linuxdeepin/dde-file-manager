/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "organizerconfig_p.h"
#include "organizer_defines.h"

#include <QStandardPaths>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

DDP_ORGANIZER_USE_NAMESPACE

namespace  {
inline constexpr char kGroupGeneral[] = "";   // "General" is default group in QSetting, so using empty string to access 'General' group.
inline constexpr char kKeyEnable[] = "Enable";
inline constexpr char kKeyMode[] = "Mode";

inline constexpr char kGroupNormalized[] = "Normalized";
inline constexpr char kKeyClassification[] = "Classification";

} // namepace

OrganizerConfigPrivate::OrganizerConfigPrivate(OrganizerConfig *qq) : q(qq)
{

}

OrganizerConfigPrivate::~OrganizerConfigPrivate()
{
    delete settings;
    settings = nullptr;
}

QVariant OrganizerConfigPrivate::value(const QString &group, const QString &key, const QVariant &defaultVar)
{
    if (key.isEmpty())
        return defaultVar;

    QVariant ret;
    if (group.isEmpty()) {
        ret = settings->value(key, defaultVar);
    } else {
        settings->beginGroup(group);
        ret = settings->value(key, defaultVar);
        settings->endGroup();
    }

    return ret;
}

void OrganizerConfigPrivate::setValue(const QString &group, const QString &key, const QVariant &var)
{
    settings->beginGroup(group);
    settings->setValue(key, var);
    settings->endGroup();
}

OrganizerConfig::OrganizerConfig(QObject *parent)
    : QObject(parent)
    , d(new OrganizerConfigPrivate(this))
{
    // 只能在主线程创建
    Q_ASSERT(qApp->thread() == thread());

    auto configPath = path();
    qDebug() << "OrganizerConfig: file path" << configPath;

    QFileInfo configFile(configPath);
    if (!configFile.exists())
        configFile.absoluteDir().mkpath(".");

    d->settings = new QSettings(configPath, QSettings::IniFormat);
}

bool OrganizerConfig::isEnable() const
{
    return d->value(kGroupGeneral, kKeyEnable, false).toBool();
}

void OrganizerConfig::setEnable(bool e)
{
    d->setValue(kGroupGeneral, kKeyEnable, e);
}

int OrganizerConfig::mode() const
{
    return d->value(kGroupGeneral, kKeyMode, OrganizerMode::kNormalized).toInt();
}

void OrganizerConfig::setMode(int m)
{
    d->setValue(kGroupGeneral, kKeyMode, m);
}

void OrganizerConfig::sync()
{
    d->settings->sync();
}

int OrganizerConfig::classification() const
{
    return d->value(kGroupNormalized, kKeyClassification, Classifier::kType).toInt();
}

void OrganizerConfig::setClassification(int cf)
{
    d->setValue(kGroupNormalized, kKeyClassification, cf);
}

OrganizerConfig::~OrganizerConfig()
{
    delete d;
    d = nullptr;
}

QString OrganizerConfig::path() const
{
    // user config path
    auto paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    Q_ASSERT(!paths.isEmpty());

    QString configPath = paths.first();
    configPath = configPath
            + QDir::separator() + QApplication::organizationName()
            + QDir::separator() + QApplication::applicationName()
            + QDir::separator() + "ddplugin-organizer.conf";

    return configPath;
}
