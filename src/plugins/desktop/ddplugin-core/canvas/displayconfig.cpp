/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
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
#include "displayconfig.h"

#include <QThread>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QSettings>
#include <QDebug>

class DisplayConfigGlobal : public DisplayConfig{};
Q_GLOBAL_STATIC(DisplayConfigGlobal, displayConfig)

static const char * const kGroupGeneral = "GeneralConfig";
static const char * const kKeyProfile = "Profile";
static const char * const kKeySortBy = "SortBy";
static const char * const kKeySortOrder = "SortOrder";
static const char * const kKeyAutoAlign = "AutoSort";
static const char * const kKeyIconLevel = "IconLevel";
static const char * const kKeyCustomWaterMask = "WaterMaskUseJson";

static void compatibilityFuncForDisbaleAutoMerage(QSettings *set)
{
    Q_ASSERT(set);
    static const QString keyAutoMerge = "AutoMerge";
    set->beginGroup(kGroupGeneral);
    if (set->contains(keyAutoMerge)) {
        set->remove(keyAutoMerge);
        set->sync();
    }
    set->endGroup();
}

DisplayConfig *DisplayConfig::instance()
{
    return displayConfig;
}

DisplayConfig::DisplayConfig(QObject *parent) : QObject(parent)
{
    // 只能在主线程创建
    Q_ASSERT(qApp->thread() == thread());

    auto configPath = path();
    qDebug() << "DisplayConfig: file path" << configPath;

    QFileInfo configFile(configPath);
    if (!configFile.exists()) {
        configFile.absoluteDir().mkpath(".");
    }

    settings = new QSettings(configPath, QSettings::IniFormat);
    // to disable automerge after upgrading
    compatibilityFuncForDisbaleAutoMerage(settings);

    auto work = new QThread(this);
    this->moveToThread(work);
    work->start();
    settings->setParent(work);

    // delay sync
    syncTimer = new QTimer(this);
    syncTimer->setSingleShot(true);
    syncTimer->setInterval(1000);
    connect(syncTimer, &QTimer::timeout, this, [this]() {
        QMutexLocker lk(&mtxLock);
        settings->sync();
    }, Qt::QueuedConnection);
}

DisplayConfig::~DisplayConfig()
{
    delete settings;
    settings = nullptr;

    delete syncTimer;
    syncTimer = nullptr;
}

QList<QString> DisplayConfig::profile()
{
    QList<QString> ret;
    QMutexLocker lk(&mtxLock);
    settings->beginGroup(kKeyProfile);
    const QStringList &keys = settings->childKeys();
    for (const QString &key: keys) {
        const QString &value = settings->value(key).toString();
        if (value.isEmpty())
            continue;

        ret.append(value);
    }
    settings->endGroup();
    return ret;
}

bool DisplayConfig::setProfile(const QList<QString> &profile)
{
    // clear all
    remove(kKeyProfile, "");

    QHash<QString, QVariant> values;
    int idx = 1;
    for (auto iter = profile.cbegin(); iter != profile.cend(); ++iter, ++idx) {
        if (iter->isEmpty())
            continue;
        values.insert(QString::number(idx), *iter);
    }

    // save
    if (!values.isEmpty()) {
        setValues(kKeyProfile, values);
        return true;
    }

    return false;
}

QHash<QString, QPoint> DisplayConfig::coordinates(const QString &key)
{
    QHash<QString, QPoint> ret;
    if (key.isEmpty())
        return ret;

    QMutexLocker lk(&mtxLock);
    settings->beginGroup(key);
    auto posKeys = settings->childKeys();
    for (const QString &posKey : posKeys) {
        QPoint pos;
        if (!covertPostion(posKey, pos))
            continue;
        const QString &value = settings->value(posKey).toString();
        if (value.isEmpty())
            continue;
        ret.insert(value, pos);
    }
    settings->endGroup();

    return ret;
}

bool DisplayConfig::setCoordinates(const QString &key, const QHash<QString, QPoint> &pos)
{
    if (key.isEmpty())
        return false;

    // clear old data
    remove(key, QString());

    QHash<QString, QVariant> values;
    for (auto iter = pos.cbegin(); iter != pos.cend(); ++iter) {
        auto posKey = covertPostion(iter.value());
        // invaild pos
        if (posKey.isEmpty() || iter.key().isEmpty())
            continue;
        values.insert(posKey, iter.key());
    }

    if (!values.isEmpty())
        setValues(key, values);

    return true;
}

void DisplayConfig::sortMethod(int &role, Qt::SortOrder &order)
{
    QMutexLocker lk(&mtxLock);
    settings->beginGroup(kGroupGeneral);

    // sort role
    {
        bool ok = false;
        role = settings->value(kKeySortBy).toInt(&ok);
        if (!ok)
            role = -1;
    }

    // sort order
    {
        int val = settings->value(kKeySortOrder, static_cast<int>(Qt::AscendingOrder)).toInt();
        order = val == Qt::AscendingOrder ? Qt::AscendingOrder : Qt::DescendingOrder;
    }

    settings->endGroup();
}

bool DisplayConfig::setSortMethod(const int &role, const Qt::SortOrder &order)
{
    if (role < 0)
        return false;

    QHash<QString, QVariant> values;
    values.insert(kKeySortBy, role);
    values.insert(kKeySortOrder, static_cast<int>(order));
    setValues(kGroupGeneral, values);
    return true;
}

bool DisplayConfig::autoAlign()
{
    return value(kGroupGeneral, kKeyAutoAlign, false).toBool();
}

void DisplayConfig::setAutoAlign(bool align)
{
    QHash<QString, QVariant> values;
    values.insert(kKeyAutoAlign, align);
    setValues(kGroupGeneral, values);
}

int DisplayConfig::iconLevel()
{
    return value(kGroupGeneral, kKeyIconLevel, -1).toInt();
}

bool DisplayConfig::setIconLevel(int lv)
{
    if (lv < 0)
        return false;

    QHash<QString, QVariant> values;
    values.insert(kKeyIconLevel, lv);
    setValues(kGroupGeneral, values);

    return true;
}

bool DisplayConfig::customWaterMask()
{
    return value(kGroupGeneral, kKeyCustomWaterMask, true).toBool();
}

void DisplayConfig::setValues(const QString &group, const QHash<QString, QVariant> &values)
{
    QMutexLocker lk(&mtxLock);
    settings->beginGroup(group);

    for (auto iter = values.cbegin(); iter != values.cend(); ++iter)
        settings->setValue(iter.key(), iter.value());

    settings->endGroup();
    sync();
}

void DisplayConfig::remove(const QString &group, const QString &key)
{
    QMutexLocker lk(&mtxLock);
    settings->beginGroup(group);

    // If key is an empty string,
    // all keys in the current group are removed
    settings->remove(key);

    settings->endGroup();
    sync();
}

void DisplayConfig::remove(const QString &group, const QStringList &keys)
{
    QMutexLocker lk(&mtxLock);
    settings->beginGroup(group);

    for (const QString &key : keys)
        settings->remove(key);

    settings->endGroup();

    sync();
}

QString DisplayConfig::path() const
{
    // user config path
    auto paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    Q_ASSERT(!paths.isEmpty());

    QString configPath = paths.first();
    configPath = configPath
            + "/" + QApplication::organizationName()
            + "/" + QApplication::applicationName()
            + "/" + QApplication::applicationName() + ".conf";

    return configPath;
}

bool DisplayConfig::covertPostion(const QString &strPos, QPoint &pos)
{
    auto coords = strPos.split("_");
    if (coords.size() != 2)
        return false;

    bool ok = false;
    int x = coords.value(0).toInt(&ok);
    if (!ok)
        return false;

    int y = coords.value(1).toInt(&ok);
    if (!ok)
        return false;

    pos = QPoint(x, y);
    return true;
}

QString DisplayConfig::covertPostion(const QPoint &pos)
{
    if (pos.x() < 0 || pos.y() < 0)
        return QString();

    return QString("%0_%1").arg(pos.x()).arg(pos.y());
}

void DisplayConfig::sync()
{
    metaObject()->invokeMethod(syncTimer, "start", Q_ARG(int, 1000));
}

QVariant DisplayConfig::value(const QString &group, const QString &key, const QVariant &defaultVar)
{
    if (key.isEmpty())
        return defaultVar;

    QVariant ret;
    QMutexLocker lk(&mtxLock);
    if (group.isEmpty()) {
        ret = settings->value(key, defaultVar);
    } else {
        settings->beginGroup(group);
        ret = settings->value(key, defaultVar);
        settings->endGroup();
    }

    return ret;
}

