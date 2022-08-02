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
#include "configsynchronizer.h"
#include "dconfig/dconfigmanager.h"
#include "settingbackend.h"
#include "private/configsynchronizer_p.h"

#include <QDebug>

DFMBASE_USE_NAMESPACE

ConfigSynchronizer::ConfigSynchronizer(QObject *parent)
    : QObject { parent }, d { new ConfigSynchronizerPrivate(this) }
{
}

ConfigSynchronizer::~ConfigSynchronizer()
{
}

ConfigSynchronizer *ConfigSynchronizer::instance()
{
    static ConfigSynchronizer ins;
    return &ins;
}

bool ConfigSynchronizer::watchChange(const SyncPair &pair)
{
    if (!pair.isValid())
        return false;

    const auto &&uniKey = pair.serialize();
    if (d->syncPairs.contains(uniKey)) {
        qInfo() << QString("%1 already watched").arg(uniKey);
        return false;
    }

    d->syncPairs.insert(uniKey, pair);
    if (pair.set.type == kAppAttr)
        SettingBackend::instance()->addSettingAccessor(static_cast<Application::ApplicationAttribute>(pair.set.val), pair.saver);
    else if (pair.set.type == kGenAttr)
        SettingBackend::instance()->addSettingAccessor(static_cast<Application::GenericAttribute>(pair.set.val), pair.saver);

    return true;
}

ConfigSynchronizerPrivate::ConfigSynchronizerPrivate(ConfigSynchronizer *qq)
    : q(qq)
{
    initConn();
}

void ConfigSynchronizerPrivate::initConn()
{
    q->connect(DConfigManager::instance(), &DConfigManager::valueChanged, q, [this](const QString &path, const QString &key) {
        onDConfChanged(path, key);
    });
}

void ConfigSynchronizerPrivate::onDConfChanged(const QString &cfgPath, const QString &cfgKey)
{
    auto newVal = DConfigManager::instance()->value(cfgPath, cfgKey);
    syncToAppSet(cfgPath, cfgKey, newVal);
}

void ConfigSynchronizerPrivate::syncToAppSet(const QString &cfgPath, const QString &cfgKey, const QVariant &var)
{
    const auto &confKey = SyncPair::serialize({}, { cfgPath, cfgKey });
    if (confKey.isEmpty())
        return;

    for (auto iter = syncPairs.cbegin(); iter != syncPairs.cend(); ++iter) {
        if (iter.key().endsWith(confKey)) {
            if (auto syncFunc = iter.value().toAppSet) {
                QVariant appSetVal;
                if (iter.value().set.type == SettingType::kAppAttr)
                    appSetVal = Application::appAttribute(static_cast<Application::ApplicationAttribute>(iter.value().set.val));
                else if (iter.value().set.type == SettingType::kGenAttr)
                    appSetVal = Application::genericAttribute(static_cast<Application::GenericAttribute>(iter.value().set.val));
                else
                    continue;

                if (auto isEqual = iter.value().isEqual) {
                    if (isEqual(var, appSetVal))
                        continue;
                }

                syncFunc(cfgPath, cfgKey, var);
                qDebug() << QString("%1:%2 is synced to DSetting by custom sync func.").arg(cfgPath).arg(cfgKey);
            } else {
                const auto &val = iter.value().set.val;
                switch (iter.value().set.type) {
                case kAppAttr: {
                    auto key = static_cast<Application::ApplicationAttribute>(val);
                    const auto &&appSetVal = Application::instance()->appAttribute(key);
                    if (appSetVal != var)
                        Application::instance()->setAppAttribute(key, var);
                    break;
                }
                case kGenAttr: {
                    auto key = static_cast<Application::GenericAttribute>(val);
                    const auto &&appSetVal = Application::instance()->genericAttribute(key);
                    if (appSetVal != var)
                        Application::instance()->setGenericAttribute(key, var);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}
