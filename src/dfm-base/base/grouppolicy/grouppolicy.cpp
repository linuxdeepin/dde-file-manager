/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
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
#include "grouppolicy.h"

#include <DConfig>

#include <QDebug>
#include <QApplication>

using namespace dfmbase;
DCORE_USE_NAMESPACE

static constexpr char kCfgAppId[] { "dde-file-manager" };
static constexpr char kCfgName[] { "org.deepin.dde.file-manager" };

class GroupPolicyGlobal : public GroupPolicy
{
};
Q_GLOBAL_STATIC(GroupPolicyGlobal, groupPolicyGlobal)

GroupPolicy *GroupPolicy::instance()
{
#if DTK_VERSION >= DTK_VERSION_CHECK(5, 5, 29, 0)
    return groupPolicyGlobal;
#endif
    return nullptr;
}

GroupPolicy::GroupPolicy(QObject *parent)
    : QObject(parent)
{
#if DTK_VERSION >= DTK_VERSION_CHECK(5, 5, 29, 0)
    config = DConfig::create(kCfgAppId, kCfgName, "", this);

    // 判断配置是否有效
    if (!isValidConfig())
        return;

    connect(config, &DConfig::valueChanged, this, &GroupPolicy::valueChanged);
#endif
}

bool GroupPolicy::isValidConfig()
{
    if (config && config->isValid())
        return true;

    qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(config->name(), config->subpath());
    return false;
}

QStringList GroupPolicy::keys()
{
    if (!isValidConfig())
        return {};

    return config->keyList();
}

bool GroupPolicy::contains(const QString &key)
{
    return keys().contains(key);
}

QVariant GroupPolicy::value(const QString &key, const QVariant &fallback)
{
    if (!isValidConfig())
        return {};

    return config->value(key, fallback);
}

void GroupPolicy::setValue(const QString &key, const QVariant &value)
{
    if (!isValidConfig())
        return;

    config->setValue(key, value);
}
