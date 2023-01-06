// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
const QString Config::keyIconSizeMode = "IconSizeMode";

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
