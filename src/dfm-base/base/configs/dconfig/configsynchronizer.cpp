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
#include "dconfigmanager.h"
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
    return true;
}

ConfigSynchronizerPrivate::ConfigSynchronizerPrivate(ConfigSynchronizer *qq)
    : q(qq)
{
    initConn();
}

void ConfigSynchronizerPrivate::initConn()
{
    auto appIns = Application::instance();
    q->connect(appIns, &Application::appAttributeChanged, q, [this](Application::ApplicationAttribute aa, const QVariant &var) {
        onAppAttrChanged(aa, var);
    });
    q->connect(appIns, &Application::genericAttributeChanged, q, [this](Application::GenericAttribute ga, const QVariant &var) {
        onGenAttrChanged(ga, var);
    });
    q->connect(DConfigManager::instance(), &DConfigManager::valueChanged, q, [this](const QString &path, const QString &key) {
        onDConfChanged(path, key);
    });
}

void ConfigSynchronizerPrivate::onAppAttrChanged(Application::ApplicationAttribute aa, const QVariant &var)
{
    syncToDConf(kAppAttr, aa, var);
}

void ConfigSynchronizerPrivate::onGenAttrChanged(Application::GenericAttribute ga, const QVariant &var)
{
    syncToDConf(kGenAttr, ga, var);
}

void ConfigSynchronizerPrivate::syncToDConf(SettingType type, int attr, const QVariant &var)
{
    const auto &setKey = SyncPair::serialize({ type, attr }, {});
    if (setKey.isEmpty())
        return;

    for (auto iter = syncPairs.cbegin(); iter != syncPairs.cend(); ++iter) {
        if (iter.key().startsWith(setKey)) {
            if (auto syncFunc = iter.value().toDconf) {
                auto currDconfVar = DConfigManager::instance()->value(iter.value().cfg.path, iter.value().cfg.key);
                if (auto compairFunc = iter.value().compair) {
                    if (compairFunc(currDconfVar, var))
                        continue;
                }
                syncFunc(attr, var);
                qDebug() << QString("%1:%2 is synced to DConfig by custom sync func.").arg(type).arg(attr);
            } else {
                const auto &cfgPath = iter.value().cfg.path;
                const auto &cfgKey = iter.value().cfg.key;
                if (!cfgPath.isEmpty() && !cfgKey.isEmpty()) {
                    const auto &&currVar = DConfigManager::instance()->value(cfgPath, cfgKey);
                    // the value of dconfig and dsetting is wrote to local file by async, and when the local
                    // file changed, the changed signal is emitted.
                    // so before set to dconfig/dsetting, make sure that the value is different from current.
                    if (currVar != var)
                        DConfigManager::instance()->setValue(cfgPath, cfgKey, var);
                }
            }
        }
    }
}

void ConfigSynchronizerPrivate::onDConfChanged(const QString &cfgPath, const QString &cfgKey)
{
    auto newVal = DConfigManager::instance()->value(cfgPath, cfgKey);   // assume that when the signal is emitted, the value is updated, not very sure, need verify
    syncToSetting(cfgPath, cfgKey, newVal);
}

void ConfigSynchronizerPrivate::syncToSetting(const QString &cfgPath, const QString &cfgKey, const QVariant &var)
{
    const auto &confKey = SyncPair::serialize({}, { cfgPath, cfgKey });
    if (confKey.isEmpty())
        return;

    for (auto iter = syncPairs.cbegin(); iter != syncPairs.cend(); ++iter) {
        if (iter.key().endsWith(confKey)) {
            if (auto syncFunc = iter.value().toAttr) {
                QVariant currDSetVal;
                if (iter.value().set.type == SettingType::kAppAttr)
                    currDSetVal = Application::appAttribute(static_cast<Application::ApplicationAttribute>(iter.value().set.val));
                else if (iter.value().set.type == SettingType::kGenAttr)
                    currDSetVal = Application::genericAttribute(static_cast<Application::GenericAttribute>(iter.value().set.val));
                else
                    continue;

                if (auto compairFunc = iter.value().compair) {
                    if (compairFunc(var, currDSetVal))
                        continue;
                }

                syncFunc(cfgPath, cfgKey, var);
                qDebug() << QString("%1:%2 is synced to DSetting by custom sync func.").arg(cfgPath).arg(cfgKey);
            } else {
                const auto &val = iter.value().set.val;
                switch (iter.value().set.type) {
                case kAppAttr: {
                    auto key = static_cast<Application::ApplicationAttribute>(val);
                    const auto &&currVar = Application::instance()->appAttribute(key);
                    if (currVar != var)
                        Application::instance()->setAppAttribute(key, var);
                    break;
                }
                case kGenAttr: {
                    auto key = static_cast<Application::GenericAttribute>(val);
                    const auto &&currVar = Application::instance()->genericAttribute(key);
                    if (currVar != var)
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
