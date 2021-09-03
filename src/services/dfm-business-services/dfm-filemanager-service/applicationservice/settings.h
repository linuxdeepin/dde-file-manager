/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef SETTINGS_H
#define SETTINGS_H

#include "services/dfm-business-services/dfm-filemanager-service/dfm_filemanager_service_global.h"

#include <QObject>
#include <QUrl>
#include <QVariantHash>

DSB_FM_BEGIN_NAMESPACE
class SettingsPrivate;
class Settings : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Settings)
    Q_PROPERTY(bool autoSync READ autoSync WRITE setAutoSync)
    Q_PROPERTY(bool watchChanges READ watchChanges WRITE setWatchChanges)
public:
    enum ConfigType {
        AppConfig,
        GenericConfig
    };

    explicit Settings(const QString &defaultFile, const QString &fallbackFile, const QString &settingFile, QObject *parent = nullptr);
    explicit Settings(const QString &name, ConfigType type = AppConfig, QObject *parent = nullptr);
    ~Settings();

    bool contains(const QString &group, const QString &key) const;
    QSet<QString> groups() const;
    QSet<QString> keys(const QString &group) const;
    QStringList keyList(const QString &group) const;
    static QUrl toUrlValue(const QVariant &url);
    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) const;
    QVariant value(const QString &group, const QUrl &key, const QVariant &defaultValue = QVariant()) const;
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
public Q_SLOTS:
    void setAutoSync(bool autoSync);
    void setWatchChanges(bool watchChanges);

Q_SIGNALS:
    void valueChanged(const QString &group, const QString &key, const QVariant &value);
    void valueEdited(const QString &group, const QString &key, const QVariant &value);

private:
    QScopedPointer<SettingsPrivate> d_ptr;
    void onFileChanged(const QUrl &);
};

DSB_FM_END_NAMESPACE

#endif // SETTINGS_H
