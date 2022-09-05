// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

