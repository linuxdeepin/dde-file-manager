// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QUrl>
#include <QVariantHash>

namespace dfmbase {

class SettingsPrivate;
class Settings : public QObject
{
    Q_OBJECT
    friend class SettingsPrivate;
    Q_PROPERTY(bool autoSync READ autoSync WRITE setAutoSync)
    Q_PROPERTY(bool watchChanges READ watchChanges WRITE setWatchChanges)
public:
    enum ConfigType {
        kAppConfig,
        kGenericConfig
    };

    explicit Settings(const QString &defaultFile, const QString &fallbackFile, const QString &settingFile, QObject *parent = nullptr);
    explicit Settings(const QString &name, ConfigType type = kAppConfig, QObject *parent = nullptr);
    ~Settings();

    bool contains(const QString &group, const QString &key) const;
    QSet<QString> groups() const;
    QSet<QString> keys(const QString &group) const;
    QStringList keyList(const QString &group) const;
    QStringList defaultConfigkeyList(const QString &group) const;
    static QUrl toUrlValue(const QVariant &url);
    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) const;
    QVariant value(const QString &group, const QUrl &key, const QVariant &defaultValue = QVariant()) const;
    QVariant defaultConfigValue(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) const;
    QVariant defaultConfigValue(const QString &group, const QUrl &key, const QVariant &defaultValue = QVariant()) const;
    QUrl urlValue(const QString &group, const QString &key, const QUrl &defaultValue = QUrl()) const;
    QUrl urlValue(const QString &group, const QUrl &key, const QUrl &defaultValue = QUrl()) const;
    void setValue(const QString &group, const QString &key, const QVariant &value);
    void setValue(const QString &group, const QUrl &key, const QVariant &value);
    bool setValueNoNotify(const QString &group, const QString &key, const QVariant &value);
    bool setValueNoNotify(const QString &group, const QUrl &key, const QVariant &value);
    void removeGroup(const QString &group);
    bool isRemovable(const QString &group, const QString &key) const;
    bool isRemovable(const QString &group, const QUrl &key) const;
    void remove(const QString &group, const QString &key);
    void remove(const QString &group, const QUrl &key);
    void clear();
    void reload();
    bool sync();
    bool autoSync() const;
    bool watchChanges() const;
    void autoSyncExclude(const QString &group, bool sync = false);
public Q_SLOTS:
    void setAutoSync(bool autoSync);
    void setWatchChanges(bool watchChanges);

Q_SIGNALS:
    void valueChanged(const QString &group, const QString &key, const QVariant &value);
    void valueEdited(const QString &group, const QString &key, const QVariant &value);

private:
    QScopedPointer<SettingsPrivate> d;
    void onFileChanged(const QUrl &);
};

}

#endif   // SETTINGS_H
