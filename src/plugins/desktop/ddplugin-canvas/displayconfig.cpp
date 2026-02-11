// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displayconfig.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-io/dfmio_utils.h>

#include <QThread>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QSettings>
#include <QDebug>

using namespace ddplugin_canvas;

class DisplayConfigGlobal : public DisplayConfig
{
};
Q_GLOBAL_STATIC(DisplayConfigGlobal, displayConfig)

static const char *const kGroupGeneral = "GeneralConfig";
static const char *const kKeyProfile = "Profile";
static const char *const kKeySortBy = "SortBy";
static const char *const kKeySortOrder = "SortOrder";
static const char *const kKeyAutoAlign = "AutoSort";
static const char *const kKeyIconLevel = "IconLevel";
static const char *const kKeyCustomWaterMask = "WaterMaskUseJson";

namespace desktop_dconfig {
static const char *const kConfigName = "org.deepin.dde.file-manager.desktop";
static const char *const kConfKeyAutoAlign = "autoAlign";
}

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

DisplayConfig::DisplayConfig(QObject *parent)
    : QObject(parent)
{
    // 只能在主线程创建
    Q_ASSERT(qApp->thread() == thread());

    auto configPath = path();
    fmDebug() << "DisplayConfig: file path" << configPath;

    QFileInfo configFile(configPath);
    if (!configFile.exists()) {
        fmInfo() << "Config file does not exist, creating directory:" << configFile.absoluteDir().path();
        configFile.absoluteDir().mkpath(".");
    }

    settings = new QSettings(configPath, QSettings::IniFormat);
    // to disable automerge after upgrading
    compatibilityFuncForDisbaleAutoMerage(settings);

    workThread = new QThread(this);
    moveToThread(workThread);
    workThread->start();

    // delay sync
    syncTimer = new QTimer();
    syncTimer->setSingleShot(true);
    syncTimer->setInterval(1000);
    connect(
            syncTimer, &QTimer::timeout, this, [this]() {
                QMutexLocker lk(&mtxLock);
                settings->sync();
            },
            Qt::QueuedConnection);
}

DisplayConfig::~DisplayConfig()
{
    if (workThread) {
        workThread->quit();
        int wait = 5;
        while (workThread->isRunning() && wait--) {
            fmDebug() << "Waiting for DisplayConfig thread to exit, attempts left:" << wait;
            bool exited = workThread->wait(100);
            fmDebug() << "DisplayConfig thread exited:" << exited;
        }
    }

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
    for (const QString &key : keys) {
        const QString &strValue = settings->value(key).toString();
        if (strValue.isEmpty())
            continue;

        ret.append(strValue);
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
        fmDebug() << "Profile saved successfully with" << values.size() << "valid entries";
        return true;
    }

    fmWarning() << "No valid profile entries to save";
    return false;
}

QHash<QString, QPoint> DisplayConfig::coordinates(const QString &key)
{
    QHash<QString, QPoint> ret;
    if (key.isEmpty()) {
        fmWarning() << "Empty key provided for coordinates lookup";
        return ret;
    }

    QMutexLocker lk(&mtxLock);
    settings->beginGroup(key);
    auto posKeys = settings->childKeys();
    for (const QString &posKey : posKeys) {
        QPoint pos;
        if (!covertPostion(posKey, pos)) {
            fmDebug() << "Invalid position format:" << posKey;
            continue;
        }
        const QString &strValue = settings->value(posKey).toString();
        if (strValue.isEmpty())
            continue;
        ret.insert(strValue, pos);
    }
    settings->endGroup();

    fmDebug() << "Loaded" << ret.size() << "coordinates for key:" << key;
    return ret;
}

bool DisplayConfig::setCoordinates(const QString &key, const QHash<QString, QPoint> &pos)
{
    if (key.isEmpty()) {
        fmWarning() << "Empty key provided for setCoordinates";
        return false;
    }

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
        if (!ok) {
            role = -1;
            fmDebug() << "Sort role not found or invalid, using default";
        }
    }

    // sort order
    {
        int val = settings->value(kKeySortOrder, static_cast<int>(Qt::AscendingOrder)).toInt();
        order = val == Qt::AscendingOrder ? Qt::AscendingOrder : Qt::DescendingOrder;
    }

    settings->endGroup();
    fmDebug() << "Retrieved sort method - role:" << role << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending");
}

bool DisplayConfig::setSortMethod(const int &role, const Qt::SortOrder &order)
{
    if (role < 0) {
        fmWarning() << "Invalid sort role provided:" << role;
        return false;
    }

    fmInfo() << "Setting sort method - role:" << role << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending");

    QHash<QString, QVariant> values;
    values.insert(kKeySortBy, role);
    values.insert(kKeySortOrder, static_cast<int>(order));
    setValues(kGroupGeneral, values);
    return true;
}

bool DisplayConfig::autoAlign()
{
    int config = dfmbase::DConfigManager::instance()->value(desktop_dconfig::kConfigName,
                                                            desktop_dconfig::kConfKeyAutoAlign,
                                                            -1)
                         .toInt();

    if (config != -1)
        return config > 0;
    return value(kGroupGeneral, kKeyAutoAlign, false).toBool();
}

void DisplayConfig::setAutoAlign(bool align)
{
    QHash<QString, QVariant> values;
    values.insert(kKeyAutoAlign, align);
    setValues(kGroupGeneral, values);

    dfmbase::DConfigManager::instance()->setValue(desktop_dconfig::kConfigName,
                                                  desktop_dconfig::kConfKeyAutoAlign,
                                                  align ? 1 : 0);
}

int DisplayConfig::iconLevel()
{
    return value(kGroupGeneral, kKeyIconLevel, -1).toInt();
}

bool DisplayConfig::setIconLevel(int lv)
{
    if (lv < 0) {
        fmWarning() << "Invalid icon level provided:" << lv;
        return false;
    }

    fmInfo() << "Setting icon level to:" << lv;

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
    configPath = DFMIO::DFMUtils::buildFilePath(configPath.toStdString().c_str(),
                                                QApplication::organizationName().toStdString().c_str(),
                                                "dde-desktop",
                                                "dde-desktop.conf", nullptr);

    return configPath;
}

bool DisplayConfig::covertPostion(const QString &strPos, QPoint &pos)
{
    auto coords = strPos.split("_");
    if (coords.size() != 2) {
        fmDebug() << "Invalid position format, expected 'x_y':" << strPos;
        return false;
    }

    bool ok = false;
    int x = coords.value(0).toInt(&ok);
    if (!ok) {
        fmDebug() << "Invalid x coordinate in position:" << strPos;
        return false;
    }

    int y = coords.value(1).toInt(&ok);
    if (!ok) {
        fmDebug() << "Invalid y coordinate in position:" << strPos;
        return false;
    }

    pos = QPoint(x, y);
    return true;
}

QString DisplayConfig::covertPostion(const QPoint &pos)
{
    if (pos.x() < 0 || pos.y() < 0) {
        fmDebug() << "Invalid position coordinates:" << pos;
        return QString();
    }

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
