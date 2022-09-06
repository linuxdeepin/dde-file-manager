// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grouppolicy.h"

#include <QDebug>
#include <QApplication>
#include <QFile>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

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
        if (auto sync = m_synchorinizers.value(key, nullptr))
            sync(getValue(key));
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

/*!
 * \brief GroupPolicy::addSyncFunc: this is used to add callback functions when DConfig's value changed, normally the function is used to sync config to dsettings.
 * \param key
 * \param func: how the config sync to dsetting.
 * \return the synchornizer is added.
 */
bool GroupPolicy::addSyncFunc(const QString &key, SyncFunc func)
{
    if (!m_synchorinizers.contains(key)) {
        m_synchorinizers.insert(key, func);
        return true;
    } else {
        qWarning() << "key: " << key << " is already binded a synchronizer";
        return false;
    }
}

/*!
 * \brief GroupPolicy::isConfigSetted: this function is for checking whether a dconfig item is setted by user, and returns it's value if it was setted.
 * \param key
 * \param settedValuej: if the key existed then the value will be setted to its current value.
 * \return true if item is setted
 */
bool GroupPolicy::isConfigSetted(const QString &key, QVariant *settedValue)
{
    auto confPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    qDebug() << "user config path: " << confPath;
    confPath += "/dsg/configs/org.deepin.dde.file-manager/org.deepin.dde.file-manager.json";

    QFile f(confPath);
    qDebug() << "user config exists at? " << confPath << f.exists();
    if (!f.exists())
        return false;

    f.open(QIODevice::ReadOnly);
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "cannot parse user config: " << confPath << err.errorString();
        return false;
    }

    auto obj = doc.object();
    if (obj.isEmpty()) {
        qWarning() << "no contents in config: " << confPath;
        return false;
    }

    auto contents = obj.value("contents").toObject();
    if (contents.contains(key)) {
        if (settedValue) {
            auto item = contents.value(key).toObject();
            *settedValue = item.value("value").toVariant();
        }
        return true;
    }
    return false;
}
