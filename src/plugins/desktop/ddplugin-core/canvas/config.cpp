/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "config.h"

#include <QThread>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>

const QString Config::kGroupGeneral = "GeneralConfig";
const QString Config::kKeyProfile = "Profile";
const QString Config::kKeySortBy = "SortBy";
const QString Config::kKeySortOrder = "SortOrder";
const QString Config::kKeyAutoAlign = "AutoSort";
const QString Config::kKeyIconLevel = "IconLevel";
const QString Config::kKeyQuickHide = "QuickHide";
const QString Config::kKeyAutoMerge = "AutoMerge";
const QString Config::kKeyWaterMask = "WaterMaskUseJson";

Config::Config()
{
    auto configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    configPath = configPath
            + "/" + QApplication::organizationName()
            + "/" + QApplication::applicationName()
            + "/" + QApplication::applicationName() + ".conf";

    qDebug() << configPath;

    QFileInfo configFile(configPath);
    if (!configFile.exists()) {
        configFile.absoluteDir().mkpath(".");
    }
    // 不使用DFMDesktopSettings，有几率启动崩溃
    settingsObj = new QSettings(configPath, QSettings::IniFormat);
    auto work = new QThread(this);
    this->moveToThread(work);
    work->start();

    syncTimer.setSingleShot(true);
    syncTimer.setInterval(1000);
    connect(&syncTimer, &QTimer::timeout, this, [=]() {
        QMutexLocker lk(&mtxLock);
        settingsObj->sync();
    }, Qt::QueuedConnection);
}

void Config::setConfig(const QString &group, const QString &key, const QVariant &value)
{
    QMutexLocker lk(&mtxLock);
    settingsObj->beginGroup(group);
    settingsObj->setValue(key, value);
    settingsObj->endGroup();
    sync();
}

void Config::setConfigList(const QString &group, const QStringList &keys, const QVariantList &values)
{
    QMutexLocker lk(&mtxLock);
    settingsObj->beginGroup(group);
    for (int i = 0; i < keys.length(); ++i) {
        settingsObj->setValue(keys.value(i), values.value(i));
    }
    settingsObj->endGroup();
    sync();
}

void Config::removeConfig(const QString &group, const QString &key)
{
    QMutexLocker lk(&mtxLock);
    settingsObj->beginGroup(group);
    settingsObj->remove(key);
    settingsObj->endGroup();
    sync();
}

void Config::removeConfigList(const QString &group, const QStringList &keys)
{
    QMutexLocker lk(&mtxLock);
    settingsObj->beginGroup(group);
    for (int i = 0; i < keys.length(); ++i) {
        settingsObj->remove(keys.value(i));
    }
    settingsObj->endGroup();
    sync();
}

void Config::sync()
{
    metaObject()->invokeMethod(&syncTimer, "start", Q_ARG(int, 1000));
}

QVariant Config::getConfig(const QString &group, const QString &key, const QVariant &defaultValue)
{
    QMutexLocker lk(&mtxLock);
    settingsObj->beginGroup(group);
    QVariant result = settingsObj->value(key, defaultValue);
    settingsObj->endGroup();
    return result;
}
