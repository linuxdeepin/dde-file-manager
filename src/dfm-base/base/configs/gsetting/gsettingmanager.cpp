// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gsettingmanager.h"
#include "private/gsettingmanager_p.h"

#include <QGSettings>

DFMBASE_USE_NAMESPACE

GSettingManager *GSettingManager::instance()
{
    static GSettingManager ins;
    return &ins;
}

GSettingManager::~GSettingManager()
{
    QWriteLocker locker(&d->lock);
    auto settings = d->settings.values();
    std::for_each(settings.begin(), settings.end(), [](QGSettings *setting) { delete setting; });
    d->settings.clear();
}

bool GSettingManager::isSchemaInstalled(const QString &schemaId)
{
    return QGSettings::isSchemaInstalled(schemaId.toLocal8Bit());
}

bool GSettingManager::addSettings(const QString &schemaId, const QString &path, QString *err)
{
    QWriteLocker locker(&d->lock);
    if (d->settings.contains(schemaId)) {
        if (err)
            *err = schemaId + " already added";
        return false;
    }

    auto gset = new QGSettings(schemaId.toLocal8Bit(), path.toLocal8Bit(), this);
    d->settings.insert(schemaId, gset);
    locker.unlock();
    connect(gset, &QGSettings::changed, this, [=](const QString &key) { Q_EMIT valueChanged(schemaId, key); });
    return true;
}

bool GSettingManager::removeSettings(const QString &schemaId, QString *err)
{
    QWriteLocker locker(&d->lock);
    if (d->settings.contains(schemaId)) {
        delete d->settings[schemaId];
        d->settings.remove(schemaId);
    }
    return true;
}

QGSettings *GSettingManager::setting(const QString &schemaId) const
{
    QReadLocker locker(&d->lock);
    return d->settings.contains(schemaId) ? d->settings.value(schemaId) : nullptr;
}

QVariant GSettingManager::get(const QString &schemaId, const QString &key) const
{
    QReadLocker locker(&d->lock);
    if (!d->settings.contains(schemaId))
        return QVariant();
    return d->settings.value(schemaId)->get(key);
}

void GSettingManager::set(const QString &schemaId, const QString &key, const QVariant &value)
{
    QReadLocker locker(&d->lock);
    if (!d->settings.contains(schemaId))
        return;
    d->settings.value(schemaId)->set(key, value);
}

bool GSettingManager::trySet(const QString &schemaId, const QString &key, const QVariant &value)
{
    QReadLocker locker(&d->lock);
    if (!d->settings.contains(schemaId))
        return false;
    return d->settings.value(schemaId)->trySet(key, value);
}

QStringList GSettingManager::keys(const QString &schemaId) const
{
    QReadLocker locker(&d->lock);
    return d->settings.contains(schemaId) ? d->settings.value(schemaId)->keys() : QStringList();
}

QVariantList GSettingManager::choices(const QString &schemaId, const QString &key) const
{
    QReadLocker locker(&d->lock);
    return d->settings.contains(schemaId) ? d->settings.value(schemaId)->choices(key) : QVariantList();
}

void GSettingManager::reset(const QString &schemaId, const QString &key)
{
    QReadLocker locker(&d->lock);
    if (d->settings.contains(schemaId))
        d->settings.value(schemaId)->reset(key);
}

GSettingManager::GSettingManager(QObject *parent)
    : QObject(parent), d(new GSettingManagerPrivate(this))
{
}
