/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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

const QString Config::groupGeneral = "GeneralConfig";
const QString Config::keyProfile = "Profile";
const QString Config::keySortBy = "SortBy";
const QString Config::keySortOrder = "SortOrder";
const QString Config::keyAutoAlign = "AutoSort";
const QString Config::keyIconLevel = "IconLevel";
const QString Config::keyQuickHide = "QuickHide";
const QString Config::keyAutoMerge = "AutoMerge";
const QString Config::keyWaterMask = "WaterMaskUseJson";

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
    //不使用DFMDesktopSettings，有几率启动崩溃
    m_settings = new QSettings(configPath,QSettings::IniFormat);
    auto work = new QThread(this);
    this->moveToThread(work);
    work->start();

    m_syncTimer.setSingleShot(true);
    m_syncTimer.setInterval(1000);
    connect(&m_syncTimer, &QTimer::timeout, this, [ = ]() {
        QMutexLocker lk(&m_mtxLock);
        m_settings->sync();
    }, Qt::QueuedConnection);
}

void Config::setConfig(const QString &group, const QString &key, const QVariant &value)
{
    QMutexLocker lk(&m_mtxLock);
    m_settings->beginGroup(group);
    m_settings->setValue(key, value);
    m_settings->endGroup();
    sync();
}

void Config::setConfigList(const QString &group, const QStringList &keys, const QVariantList &values)
{
    QMutexLocker lk(&m_mtxLock);
    m_settings->beginGroup(group);
    for (int i = 0; i < keys.length(); ++i) {
        m_settings->setValue(keys.value(i), values.value(i));
    }
    m_settings->endGroup();
    sync();
}

void Config::removeConfig(const QString &group, const QString &key)
{
    QMutexLocker lk(&m_mtxLock);
    m_settings->beginGroup(group);
    m_settings->remove(key);
    m_settings->endGroup();
    sync();
}

void Config::removeConfigList(const QString &group, const QStringList &keys)
{
    QMutexLocker lk(&m_mtxLock);
    m_settings->beginGroup(group);
    for (int i = 0; i < keys.length(); ++i) {
        m_settings->remove(keys.value(i));
    }
    m_settings->endGroup();
    sync();
}

void Config::sync()
{
    metaObject()->invokeMethod(&m_syncTimer,"start",Q_ARG(int,1000));
}

QVariant Config::getConfig(const QString &group, const QString &key, const QVariant &defaultValue)
{
    QMutexLocker lk(&m_mtxLock);
    m_settings->beginGroup(group);
    QVariant result = m_settings->value(key, defaultValue);
    m_settings->endGroup();
    return result;
}
