/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "config.h"

#include <QThread>
#include <QStandardPaths>
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
    m_settings = new QSettings(configPath, QSettings::IniFormat);
    auto work = new QThread(this);
    this->moveToThread(work);
    work->start();

    auto syncTimer = new QTimer();
    syncTimer->setInterval(2000);
    connect(syncTimer, &QTimer::timeout, this, [ = ]() {
        if (needSync) {
            needSync = false;
            m_settings->sync();
        }
    }, Qt::QueuedConnection);
    syncTimer->start();
}

void Config::setConfig(const QString &group, const QString &key, const QVariant &value)
{
    m_settings->beginGroup(group);
    m_settings->setValue(key, value);
    m_settings->endGroup();
    needSync = true;
}

void Config::setConfigList(const QString &group, const QStringList &keys, const QVariantList &values)
{
    m_settings->beginGroup(group);
    for (int i = 0; i < keys.length(); ++i) {
        m_settings->setValue(keys.value(i), values.value(i));
    }
    m_settings->endGroup();
    needSync = true;
}

void Config::removeConfig(const QString &group, const QString &key)
{
    m_settings->beginGroup(group);
    m_settings->remove(key);
    m_settings->endGroup();
    needSync = true;
}

void Config::removeConfigList(const QString &group, const QStringList &keys)
{
    m_settings->beginGroup(group);
    for (int i = 0; i < keys.length(); ++i) {
        m_settings->remove(keys.value(i));
    }
    m_settings->endGroup();
    needSync = true;
}
