// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGJSONGENERATOR_H
#define SETTINGJSONGENERATOR_H

#include <QVariantMap>

/*
 * the root group:
 * {
 *      "groups": [(top level group)]
 * }
 *
 * a top level group:
 * {
 *      "key": (string),
 *      "name": (string),
 *      "groups": [(config group)]
 * }
 *
 * a config group:
 * {
 *      "key": (string),
 *      "name": (string),
 *      "options": [(config item)]
 * }
 *
 * config item: // option item
 * {
 *      "key": (string),
 *      "text": (string),
 *      "type": (string),
 *      "default": (variant),
 *      "hide": (bool),
 *      "custom_info": (variant)
 * }
 * */

namespace dfmbase {

class SettingJsonGenerator
{
public:
    static SettingJsonGenerator *instance();

    QByteArray genSettingJson();

    bool hasGroup(const QString &key) const;
    bool hasConfig(const QString &key) const;

    bool addGroup(const QString &key, const QString &name);
    bool removeGroup(const QString &key);
    bool addConfig(const QString &key, const QVariantMap &config);
    bool removeConfig(const QString &key);
    bool addCheckBoxConfig(const QString &key, const QString &text, bool defaultVal = true);
    bool addComboboxConfig(const QString &key, const QString &name, const QStringList &options, int defaultVal = 0);
    bool addComboboxConfig(const QString &key, const QString &name, const QVariantMap &options, QVariant defaultVal = QVariant());
    bool addPathComboboxConfig(const QString &key, const QString &name, const QVariantMap &options, QVariant defaultVal = QVariant());
    bool addSliderConfig(const QString &key, const QString &name, int maxVal, int minVal, int defaultVal = 0);
    bool addSliderConfig(const QString &key, const QString &name, const QString &leftIcon, const QString &rightIcon, int maxVal, int minVal, int defaultVal = 0);
    bool addSliderConfig(const QString &key, const QString &name, const QString &leftIcon, const QString &rightIcon, int maxVal, int minVal, QVariantList valueList, int defaultVal = 0);

protected:
    SettingJsonGenerator();

    void mergeGroups();
    QJsonObject constructTopGroup(const QString &key);
    QJsonObject constructConfigGroup(const QString &key);
    QJsonObject constructConfig(const QString &key);

private:
    QMap<QString, QString> topGroups;
    QMap<QString, QString> tmpTopGroups;
    QMap<QString, QString> configGroups;
    QMap<QString, QString> tmpConfigGroups;
    QMap<QString, QVariantMap> configs;
};

}
#endif   // SETTINGJSONGENERATOR_H
