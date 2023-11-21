// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/dfm_base_global.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

using namespace dfmbase;

SettingJsonGenerator::SettingJsonGenerator()
{
}

SettingJsonGenerator *SettingJsonGenerator::instance()
{
    static SettingJsonGenerator gen;
    return &gen;
}

QByteArray SettingJsonGenerator::genSettingJson()
{
    mergeGroups();

    QJsonArray groups;
    auto iter = topGroups.cbegin();
    while (iter != topGroups.cend()) {
        groups.append(constructTopGroup(iter.key()));
        iter++;
    }

    QJsonObject obj;
    obj.insert("groups", groups);

    QJsonDocument doc;
    doc.setObject(obj);
    return doc.toJson(QJsonDocument::Indented);
}

bool SettingJsonGenerator::hasGroup(const QString &key) const
{
    return configGroups.contains(key);
}

bool SettingJsonGenerator::hasConfig(const QString &key) const
{
    return configs.contains(key);
}

bool SettingJsonGenerator::addGroup(const QString &key, const QString &name)
{
    if (key.count(".") > 1) {
        qCWarning(logDFMBase) << "max group level is 2, inputed: " << key.count(".") << key;
        return false;
    }
    if (key.startsWith(".") || key.endsWith(".")) {
        qCWarning(logDFMBase) << "the dot must not be start or end of the key." << key;
        return false;
    }

    if (key.contains(".")) {
        if (configGroups.contains(key)) {
            qCWarning(logDFMBase) << "same name key has been added:" << key << ":" << configGroups.value(key);
            return false;
        }
        configGroups.insert(key, name);

        // check if top group exist.
        QString topGroup = key.split(".").first();
        if (!topGroups.contains(topGroup)) {
            qCWarning(logDFMBase) << "no top group exist: " << topGroup;
            tmpTopGroups.insert(topGroup, topGroup);
        }
    } else {
        if (topGroups.contains(key)) {
            qCWarning(logDFMBase) << "same name key has been added: " << key << ":" << topGroups.value(key);
            return false;
        }
        topGroups.insert(key, name);
    }

    return true;
}

bool SettingJsonGenerator::removeGroup(const QString &key)
{
    if (key.count(".") > 1) {
        qCWarning(logDFMBase) << "max group level is 2, inputed: " << key.count(".") << key;
        return false;
    }
    if (key.startsWith(".") || key.endsWith(".")) {
        qCWarning(logDFMBase) << "the dot must not be start or end of the key." << key;
        return false;
    }

    if (key.contains(".")) {
        if (configGroups.remove(key) == 0) {
            qCWarning(logDFMBase) << "remove failed: " << key;
            return false;
        }
    } else {
        if (topGroups.remove(key) == 0) {
            qCWarning(logDFMBase) << "remove failed: " << key;
            return false;
        }
    }

    return true;
}

bool SettingJsonGenerator::addConfig(const QString &key, const QVariantMap &config)
{
    if (key.count(".") != 2) {
        qCWarning(logDFMBase) << "config can only be inserted in level 2:" << key;
        return false;
    }
    if (key.startsWith(".") || key.endsWith(".")) {
        qCWarning(logDFMBase) << "the dot must not be start or end of the key." << key;
        return false;
    }
    if (key.contains("..")) {
        qCWarning(logDFMBase) << "cannot insert config into empty group: " << key;
        return false;
    }
    if (configs.contains(key)) {
        qCWarning(logDFMBase) << "a same name key is already added." << key << ":" << configs.value(key);
        return false;
    }

    QStringList frags = key.split(".");
    if (frags.count() != 3)
        return false;
    QString configKey = frags.at(2);
    if (config.value("key").toString() != configKey) {
        qCWarning(logDFMBase) << "config is not valid with key" << key << config;
        return false;
    }

    frags.removeLast();
    tmpConfigGroups.insert(frags.join("."), frags.last());
    frags.removeLast();
    tmpTopGroups.insert(frags.first(), frags.first());

    configs.insert(key, config);
    return true;
}

bool SettingJsonGenerator::removeConfig(const QString &key)
{
    if (key.count(".") != 2) {
        qCWarning(logDFMBase) << "config can only be inserted in level 2:" << key;
        return false;
    }
    if (key.startsWith(".") || key.endsWith(".")) {
        qCWarning(logDFMBase) << "the dot must not be start or end of the key." << key;
        return false;
    }
    if (key.contains("..")) {
        qCWarning(logDFMBase) << "cannot remove config into empty group: " << key;
        return false;
    }

    QStringList frags = key.split(".");
    if (frags.count() != 3)
        return false;

    frags.removeLast();
    bool ret1 { tmpConfigGroups.remove(frags.join(".")) > 0 };
    frags.removeLast();
    bool ret2 { tmpTopGroups.remove(frags.first()) > 0 };
    bool ret3 { configs.remove(key) > 0 };
    return ret1 && ret2 && ret3;
}

bool SettingJsonGenerator::addCheckBoxConfig(const QString &key, const QString &text, bool defaultVal)
{
    QVariantMap config {
        { "key", key.mid(key.lastIndexOf(".") + 1) },
        { "text", text },
        { "type", "checkbox" },
        { "default", defaultVal }
    };
    return addConfig(key, config);
}

bool SettingJsonGenerator::addComboboxConfig(const QString &key, const QString &name, const QStringList &options, int defaultVal)
{
    QVariantMap config {
        { "key", key.mid(key.lastIndexOf(".") + 1) },
        { "name", name },
        { "items", options },
        { "type", "combobox" },
        { "default", defaultVal }
    };
    return addConfig(key, config);
}

bool SettingJsonGenerator::addComboboxConfig(const QString &key, const QString &name, const QVariantMap &options, QVariant defaultVal)
{
    QVariantMap config {
        { "key", key.mid(key.lastIndexOf(".") + 1) },
        { "name", name },
        { "items", options },
        { "type", "combobox" },
        { "default", defaultVal }
    };
    return addConfig(key, config);
}

void SettingJsonGenerator::mergeGroups()
{
    auto merge = [](const QMap<QString, QString> &from, QMap<QString, QString> &into) {
        auto iter = from.cbegin();
        while (iter != from.cend()) {
            if (!into.contains(iter.key()))
                into.insert(iter.key(), iter.value());
            iter++;
        }
    };
    merge(tmpConfigGroups, configGroups);
    merge(tmpTopGroups, topGroups);
}

QJsonObject SettingJsonGenerator::constructTopGroup(const QString &key)
{
    qCDebug(logDFMBase) << "construct top group:" << key;
    QJsonObject obj;
    obj.insert("key", key);
    obj.insert("name", topGroups.value(key, "Unknown"));

    QJsonArray groups;
    auto iter = configGroups.cbegin();
    while (iter != configGroups.cend()) {
        if (iter.key().startsWith(key))
            groups.append(constructConfigGroup(iter.key()));
        iter++;
    }

    obj.insert("groups", groups);
    return obj;
}

QJsonObject SettingJsonGenerator::constructConfigGroup(const QString &key)
{
    qCDebug(logDFMBase) << "construct config group:" << key;
    QJsonObject obj;
    obj.insert("key", key.mid(key.indexOf(".") + 1));
    obj.insert("name", configGroups.value(key, "Unknown"));

    QJsonArray options;
    auto iter = configs.cbegin();
    while (iter != configs.cend()) {
        if (iter.key().startsWith(key))
            options.append(constructConfig(iter.key()));
        iter++;
    }

    obj.insert("options", options);
    return obj;
}

QJsonObject SettingJsonGenerator::constructConfig(const QString &key)
{
    qCDebug(logDFMBase) << "construct item: " << key;
    QVariantMap config = configs.value(key, QVariantMap());
    return QJsonObject::fromVariantMap(config);
}
