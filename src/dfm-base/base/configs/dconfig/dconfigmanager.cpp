// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dconfigmanager.h"
#include "private/dconfigmanager_p.h"

#include <DConfig>

#include <QDebug>
#include <QSet>

static constexpr char kCfgAppId[] { "org.deepin.dde.file-manager" };

using namespace dfmbase;
DCORE_USE_NAMESPACE

DConfigManager::DConfigManager(QObject *parent)
    : QObject(parent), d(new DConfigManagerPrivate(this))
{
    addConfig(kDefaultCfgPath);
    addConfig(kViewDConfName);
    addConfig(kAnimationDConfName);
}

DConfigManager *DConfigManager::instance()
{
    static DConfigManager ins;
    return &ins;
}

DConfigManager::~DConfigManager()
{
#ifdef DTKCORE_CLASS_DConfig
    QWriteLocker locker(&d->lock);

    auto configs = d->configs.values();
    std::for_each(configs.begin(), configs.end(), [](DConfig *cfg) { delete cfg; });
    d->configs.clear();
#endif
}

bool DConfigManager::addConfig(const QString &config, QString *err)
{
#ifdef DTKCORE_CLASS_DConfig
    QWriteLocker locker(&d->lock);

    if (d->configs.contains(config)) {
        if (err)
            *err = "config is already added";
        qCInfo(logDFMBase) << config << "already added.";
        return false;
    }

    auto cfg = DConfig::create(kCfgAppId, config, "", this);
    if (!cfg) {
        if (err)
            *err = "cannot create config";
        qCWarning(logDFMBase) << config << "object cannot be created!";
        return false;
    }

    if (!cfg->isValid()) {
        if (err)
            *err = "config is not valid";
        qCWarning(logDFMBase) << config << "not valid config!";
        delete cfg;
        return false;
    }

    d->configs.insert(config, cfg);
    locker.unlock();
    connect(cfg, &DConfig::valueChanged, this, [=](const QString &key) { Q_EMIT valueChanged(config, key); });
#endif
    return true;
}

bool DConfigManager::removeConfig(const QString &config, QString *err)
{
#ifdef DTKCORE_CLASS_DConfig
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
#ifdef DTKCORE_CLASS_DConfig
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
#ifdef DTKCORE_CLASS_DConfig
    QReadLocker locker(&d->lock);

    if (d->configs.contains(config))
        return d->configs.value(config)->value(key, fallback);
    else
        qCWarning(logDFMBase) << "Config: " << config << "is not registered!!!";
    return fallback;
#else
    return fallback;
#endif
}

void DConfigManager::setValue(const QString &config, const QString &key, const QVariant &value)
{
#ifdef DTKCORE_CLASS_DConfig
    QReadLocker locker(&d->lock);

    if (d->configs.contains(config))
        d->configs.value(config)->setValue(key, value);
#endif
}

bool DConfigManager::validateConfigs(QStringList &invalidConfigs) const
{
#ifdef DTKCORE_CLASS_DConfig
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
