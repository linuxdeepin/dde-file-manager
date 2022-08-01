/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "dconfigmanager.h"
#include "private/dconfigmanager_p.h"
#include "configsynchronizer.h"

#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/utils/finallyutil.h"

#include <DConfig>

#include <QSet>

static constexpr char kCfgAppId[] { "org.deepin.dde.file-manager" };

using namespace dfmbase;
DCORE_USE_NAMESPACE

#define DCONFIG_SUPPORTED (DTK_VERSION >= DTK_VERSION_CHECK(5, 5, 29, 0))

DConfigManager::DConfigManager(QObject *parent)
    : QObject(parent), d(new DConfigManagerPrivate(this))
{
    addConfig(kDefaultCfgPath);
}

DConfigManager *DConfigManager::instance()
{
    static DConfigManager ins;
    return &ins;
}

DConfigManager::~DConfigManager()
{
#if DCONFIG_SUPPORTED
    QWriteLocker locker(&d->lock);

    auto configs = d->configs.values();
    std::for_each(configs.begin(), configs.end(), [](DConfig *cfg) { delete cfg; });
    d->configs.clear();
#endif
}

bool DConfigManager::addConfig(const QString &config, QString *err)
{
#if DCONFIG_SUPPORTED
    QWriteLocker locker(&d->lock);

    if (d->configs.contains(config)) {
        if (err)
            *err = "config is already added";
        return false;
    }

    auto cfg = DConfig::create(kCfgAppId, config, "", this);
    if (!cfg) {
        if (err)
            *err = "cannot create config";
        return false;
    }

    if (!cfg->isValid()) {
        if (err)
            *err = "config is not valid";
        delete cfg;
        return false;
    }

    d->configs.insert(config, cfg);
    locker.unlock();
    connect(cfg, &DConfig::valueChanged, this, [=](const QString &key) { emit valueChanged(config, key); });
#endif
    return true;
}

bool DConfigManager::removeConfig(const QString &config, QString *err)
{
#if DCONFIG_SUPPORTED
    QWriteLocker locker(&d->lock);

    if (d->configs.contains(config)) {
        delete d->configs[config];
        d->configs.remove(config);
    }
#endif
    return true;
}

QStringList DConfigManager::keys(const QString &config) const
{
#if DCONFIG_SUPPORTED
    QReadLocker locker(&d->lock);

    if (!d->configs.contains(config))
        return QStringList();

    return d->configs[config]->keyList();
#else
    return QStringList();
#endif
}

bool DConfigManager::contains(const QString &config, const QString &key) const
{
    return key.isEmpty() ? false : keys(config).contains(key);
}

QVariant DConfigManager::value(const QString &config, const QString &key, const QVariant &fallback) const
{
#if DCONFIG_SUPPORTED
    QReadLocker locker(&d->lock);

    if (d->configs.contains(config))
        return d->configs.value(config)->value(key, fallback);
    return fallback;
#else
    return QVariant();
#endif
}

void DConfigManager::setValue(const QString &config, const QString &key, const QVariant &value)
{
#if DCONFIG_SUPPORTED
    QReadLocker locker(&d->lock);

    if (d->configs.contains(config))
        d->configs.value(config)->setValue(key, value);
#endif
}

bool DConfigManager::validateConfigs(QStringList &invalidConfigs) const
{
#if DCONFIG_SUPPORTED
    QReadLocker locker(&d->lock);

    bool ret = true;
    for (auto iter = d->configs.cbegin(); iter != d->configs.cend(); ++iter) {
        bool valid = iter.value()->isValid();
        if (!valid)
            invalidConfigs << iter.key();
        ret &= valid;
    }
    return ret;
#else
    return true;
#endif
}
