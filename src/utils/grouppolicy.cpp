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

#include <QDebug>
#include <QApplication>

class GroupPolicyGlobal : public GroupPolicy{};
Q_GLOBAL_STATIC(GroupPolicyGlobal, groupPolicyGlobal)

GroupPolicy *GroupPolicy::instance()
{
    groupPolicyGlobal->moveToThread(qApp->thread());
    return groupPolicyGlobal;
}

GroupPolicy::GroupPolicy(QObject *parent) : QObject(parent)
{
    if (!DTK_POLICY_SUPPORT) {
        qWarning() << QString("The current version of dtkcore(%1) does not support group policies").arg(DTK_VERSION);
    }

#if (DTK_POLICY_SUPPORT)
    m_config = Dtk::Core::DConfig::create("org.deepin.dde.file-manager", "org.deepin.dde.file-manager", "", this);

    // 判断配置是否有效
    if (!m_config->isValid()) {
        qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(m_config->name(), m_config->subpath());
        return;
    }

    connect(m_config, &Dtk::Core::DConfig::valueChanged, this, [this](const QString &key){
        emit this->valueChanged(key);
    });
#endif
}

QStringList GroupPolicy::getKeys()
{
    if (!DTK_POLICY_SUPPORT)
        return QStringList();
#if (DTK_POLICY_SUPPORT)
    if (!m_config || !m_config->isValid()) {
        qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(m_config->name(), m_config->subpath());
        return {};
    }
    return m_config->keyList();
#endif
    return QStringList();
}

bool GroupPolicy::containKey(const QString &key)
{
    return getKeys().contains(key);
}

QVariant GroupPolicy::getValue(const QString &key, const QVariant &fallback)
{
    if (!DTK_POLICY_SUPPORT)
        return QVariant();

#if (DTK_POLICY_SUPPORT)
    // 判断配置是否有效
    if (!m_config || !m_config->isValid()) {
        qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(m_config->name(), m_config->subpath());
        return QVariant();
    }
    return m_config->value(key, fallback);
#endif
}

void GroupPolicy::setValue(const QString &key, const QVariant &value)
{
    if (!DTK_POLICY_SUPPORT)
        return;

#if (DTK_POLICY_SUPPORT)
    // 判断配置是否有效
    if (!m_config || !m_config->isValid()) {
        qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(m_config->name(), m_config->subpath());
        return;
    }
    m_config->setValue(key,value);
#endif
}

