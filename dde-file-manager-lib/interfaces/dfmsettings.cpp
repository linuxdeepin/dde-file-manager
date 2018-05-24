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
#include "dfmsettings.h"

#include "dfilewatcher.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTimer>

DFM_BEGIN_NAMESPACE

class DFMSettingsPrivate
{
public:
    DFMSettingsPrivate(DFMSettings *qq);

    bool autoSync = false;
    bool watchChanges = false;
    bool settingFileIsDirty = false;

    QTimer *syncTimer = nullptr;

    QString fallbackFile;
    QString settingFile;
    DFileWatcher *settingFileWatcher = nullptr;

    DFMSettings *q_ptr;

    struct Data {
        QHash<QString, QVariantHash> values;

        QVariant value(const QString &group, const QString &key, const QVariant &dv = QVariant()) const
        {
            return values.value(group).value(key, dv);
        }

        void setValue(const QString group, const QString &key, const QVariant &value)
        {
            if (!values.contains(group)) {
                values.insert(group, {{key, value}});

                return;
            }

            values[group][key] = value;
        }
    };

    Data defaultData;
    Data fallbackData;
    Data writableData;

    void fromJsonFile(const QString &fileName, Data *data);
    void fromJson(const QByteArray &json, Data *data);
    QByteArray toJson(const Data &data);

    void makeSettingFileToDirty(bool dirty)
    {
        if (settingFileIsDirty == dirty)
            return;

        settingFileIsDirty = dirty;

        if (!autoSync)
            return;

        Q_ASSERT(syncTimer);

        if (QThread::currentThread() == syncTimer->thread()) {
            if (dirty) {
                syncTimer->start();
            } else {
                syncTimer->stop();
            }
        } else {
            syncTimer->metaObject()->invokeMethod(syncTimer, dirty ? "start" : "stop", Qt::QueuedConnection);
        }
    }

    void _q_onFileChanged(const DUrl &url);
};

DFMSettingsPrivate::DFMSettingsPrivate(DFMSettings *qq)
    : q_ptr(qq)
{

}

void DFMSettingsPrivate::fromJsonFile(const QString &fileName, Data *data)
{
    QFile file(fileName);

    if (!file.exists())
        return;

    if (!file.open(QFile::ReadOnly)) {
        qWarning() << file.errorString();

        return;
    }

    const QByteArray &json = file.readAll();

    if (json.isEmpty())
        return;

    fromJson(json, data);
}

void DFMSettingsPrivate::fromJson(const QByteArray &json, Data *data)
{
    QJsonParseError error;
    const QJsonDocument &doc = QJsonDocument::fromJson(json, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << error.errorString();
        return;
    }

    if (!doc.isObject()) {
        qWarning() << QString();
        return;
    }

    const QJsonObject &groups_object = doc.object();

    for (auto begin = groups_object.constBegin(); begin != groups_object.constEnd(); ++begin) {
        const QJsonValue &value = begin.value();

        if (!value.isObject()) {
            qWarning() << QString();
            continue;
        }

        const QJsonObject &value_object = value.toObject();
        QVariantHash hash;

        for (auto begin = value_object.constBegin(); begin != value_object.constEnd(); ++begin) {
            hash[begin.key()] = begin.value().toVariant();
        }

        data->values[begin.key()] = hash;
    }
}

QByteArray DFMSettingsPrivate::toJson(const Data &data)
{
    QJsonObject root_object;

    for (auto begin = data.values.constBegin(); begin != data.values.constEnd(); ++begin) {
        root_object.insert(begin.key(), QJsonValue(QJsonObject::fromVariantHash(begin.value())));
    }

    return QJsonDocument(root_object).toJson();
}

void DFMSettingsPrivate::_q_onFileChanged(const DUrl &url)
{
    if (url.toLocalFile() != settingFile) {
        return;
    }

    const auto old_values = writableData.values;

    writableData.values.clear();
    fromJsonFile(settingFile, &writableData);
    makeSettingFileToDirty(false);

    for (auto begin = writableData.values.constBegin(); begin != writableData.values.constEnd(); ++begin) {
        for (auto i = begin.value().constBegin(); i != begin.value().constEnd(); ++i) {
            if (old_values.value(begin.key()).contains(i.key())) {
                if (old_values.value(begin.key()).value(i.key()) == i.value())
                    continue;
            } else {
                if (fallbackData.values.value(begin.key()).contains(i.key())) {
                    if (fallbackData.values.value(begin.key()).value(i.key()) == i.value())
                        continue;
                }

                if (defaultData.values.value(begin.key()).value(i.key()) == i.value())
                    continue;
            }

            Q_EMIT q_ptr->valueChanged(begin.key(), i.key(), i.value());
        }
    }

    for (auto begin = old_values.constBegin(); begin != old_values.constEnd(); ++begin) {
        for (auto i = begin.value().constBegin(); i != begin.value().constEnd(); ++i) {
            if (writableData.values.value(begin.key()).contains(i.key()))
                continue;

            const QVariant &new_value = q_ptr->value(begin.key(), i.key());

            if (new_value != old_values.value(begin.key()).value(i.key()))
                Q_EMIT q_ptr->valueChanged(begin.key(), i.key(), new_value);
        }
    }
}

DFMSettings::DFMSettings(const QString &defaultFile, const QString &fallbackFile, const QString &settingFile, QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMSettingsPrivate(this))
{
    d_ptr->fallbackFile = fallbackFile;
    d_ptr->settingFile = settingFile;

    d_ptr->fromJsonFile(defaultFile, &d_ptr->defaultData);
    d_ptr->fromJsonFile(fallbackFile, &d_ptr->fallbackData);
    d_ptr->fromJsonFile(settingFile, &d_ptr->writableData);
}

static QString getConfigFilePath(QStandardPaths::StandardLocation type, const QString &fileName, bool writable)
{
    if (writable) {
        QString path = QStandardPaths::writableLocation(type);

        if (path.isEmpty()) {
            path = QDir::home().absoluteFilePath(QString(".config/%1/%2").arg(qApp->organizationName()).arg(qApp->applicationName()));
        }

        return path.append(QString("/%1.json").arg(fileName));
    }

    const QStringList &list = QStandardPaths::standardLocations(type);

    QString path = list.isEmpty() ? QString("/etc/xdg/%1/%2").arg(qApp->organizationName()).arg(qApp->applicationName()) : list.last();

    return path.append(QString("/%1.json").arg(fileName));
}

DFMSettings::DFMSettings(const QString &name, ConfigType type, QObject *parent)
    : DFMSettings(QString(":/config/%2.json").arg(name),
                  getConfigFilePath(type == AppConfig
                                    ? QStandardPaths::AppConfigLocation
                                    : QStandardPaths::GenericConfigLocation,
                                    name, false),
                  getConfigFilePath(type == AppConfig
                                    ? QStandardPaths::AppConfigLocation
                                    : QStandardPaths::GenericConfigLocation,
                                    name, true),
                  parent)
{

}

DFMSettings::~DFMSettings()
{
    Q_D(DFMSettings);

    if (d->syncTimer) {
        d->syncTimer->stop();
    }

    if (d->settingFileIsDirty)
        sync();
}

bool DFMSettings::contains(const QString &group, const QString &key) const
{
    Q_D(const DFMSettings);

    if (key.isEmpty()) {
        if (d->writableData.values.contains(group))
            return true;

        if (d->fallbackData.values.contains(group))
            return true;

        return d->defaultData.values.contains(group);
    }

    if (d->writableData.values.value(group).contains(key))
        return true;

    if (d->fallbackData.values.value(group).contains(key))
        return true;

    return d->defaultData.values.value(group).contains(key);
}

QSet<QString> DFMSettings::groups() const
{
    Q_D(const DFMSettings);

    QSet<QString> groups;

    groups.reserve(d->writableData.values.size() + d->fallbackData.values.size() + d->defaultData.values.size());

    for (auto begin = d->writableData.values.constBegin(); begin != d->writableData.values.constEnd(); ++begin) {
        groups << begin.key();
    }

    for (auto begin = d->fallbackData.values.constBegin(); begin != d->fallbackData.values.constEnd(); ++begin) {
        groups << begin.key();
    }

    for (auto begin = d->defaultData.values.constBegin(); begin != d->defaultData.values.constEnd(); ++begin) {
        groups << begin.key();
    }

    return groups;
}

QSet<QString> DFMSettings::keys(const QString &group) const
{
    Q_D(const DFMSettings);

    QSet<QString> keys;

    const auto &&wg = d->writableData.values.value(group);
    const auto &&fg = d->fallbackData.values.value(group);
    const auto &&dg = d->defaultData.values.value(group);

    keys.reserve(wg.size() + fg.size() + dg.size());

    for (auto begin = wg.constBegin(); begin != wg.constEnd(); ++begin) {
        keys << begin.key();
    }

    for (auto begin = fg.constBegin(); begin != fg.constEnd(); ++begin) {
        keys << begin.key();
    }

    for (auto begin = dg.constBegin(); begin != dg.constEnd(); ++begin) {
        keys << begin.key();
    }

    return keys;
}

QVariant DFMSettings::value(const QString &group, const QString &key, const QVariant &defaultValue) const
{
    Q_D(const DFMSettings);

    QVariant value = d->writableData.values.value(group).value(key, QVariant::Invalid);

    if (value.isValid())
        return value;

    value = d->fallbackData.values.value(group).value(key, QVariant::Invalid);

    if (value.isValid())
        return value;

    return d->defaultData.values.value(group).value(key, defaultValue);
}

void DFMSettings::setValue(const QString &group, const QString &key, const QVariant &value)
{
    Q_D(DFMSettings);

    bool emit_signal = false;

    if (isRemovable(group, key)) {
        if (d->writableData.value(group, key) == value)
            return;

        emit_signal  = true;
    } else {
        emit_signal = this->value(group, key, value) == value;
    }

    d->writableData.setValue(group, key, value);
    d->makeSettingFileToDirty(true);

    if (emit_signal)
        Q_EMIT valueChanged(group, key, value);
}

void DFMSettings::removeGroup(const QString &group)
{
    Q_D(DFMSettings);

    if (!d->writableData.values.contains(group))
        return;

    const QVariantHash &group_values = d->writableData.values.take(group);

    d->makeSettingFileToDirty(true);

    for (auto begin = group_values.constBegin(); begin != group_values.constEnd(); ++begin) {
        const QVariant &new_value = value(group, begin.key());

        if (new_value != begin.value())
            Q_EMIT valueChanged(group, begin.key(), new_value);
    }
}

bool DFMSettings::isRemovable(const QString &group, const QString &key)
{
    Q_D(DFMSettings);

    return d->writableData.values.value(group).contains(key);
}

void DFMSettings::remove(const QString &group, const QString &key)
{
    Q_D(DFMSettings);

    if (!d->writableData.values.value(group).contains(key))
        return;

    const QVariant &old_value = d->writableData.values[group].take(key);
    d->makeSettingFileToDirty(true);

    const QVariant &new_value = value(group, key);

    if (old_value == new_value)
        return;

    Q_EMIT valueChanged(group, key, new_value);
}

void DFMSettings::clear()
{
    Q_D(DFMSettings);

    if (d->writableData.values.isEmpty())
        return;

    const QHash<QString, QVariantHash> old_values = d->writableData.values;

    d->writableData.values.clear();
    d->makeSettingFileToDirty(true);

    for (auto begin = old_values.constBegin(); begin != old_values.constEnd(); ++begin) {
        const QVariantHash &values = begin.value();

        for (auto i = values.constBegin(); i != values.constEnd(); ++i) {
            const QVariant &new_value = value(begin.key(), i.key());

            if (new_value != i.value()) {
                Q_EMIT valueChanged(begin.key(), i.key(), new_value);
            }
        }
    }
}

bool DFMSettings::sync()
{
    Q_D(DFMSettings);

    const QByteArray &json = d->toJson(d->writableData);

    QFile file(d->settingFile);

    if (!file.open(QFile::WriteOnly)) {
        return false;
    }

    bool ok = file.write(json) == json.size();

    if (ok) {
        d->makeSettingFileToDirty(false);
    }

    return ok;
}

bool DFMSettings::autoSync() const
{
    Q_D(const DFMSettings);

    return d->autoSync;
}

bool DFMSettings::watchChanges() const
{
    Q_D(const DFMSettings);

    return d->watchChanges;
}

void DFMSettings::setAutoSync(bool autoSync)
{
    Q_D(DFMSettings);

    if (d->autoSync == autoSync)
        return;

    d->autoSync = autoSync;

    if (autoSync) {
        if (d->settingFileIsDirty)
            sync();

        if (!d->syncTimer) {
            d->syncTimer = new QTimer(this);
            d->syncTimer->moveToThread(thread());
            d->syncTimer->setSingleShot(true);
            d->syncTimer->setInterval(1000);

            qDebug() << thread() << d->syncTimer->thread();

            connect(d->syncTimer, &QTimer::timeout, this, &DFMSettings::sync);
        }
    } else {
        if (d->syncTimer) {
            d->syncTimer->stop();
            d->syncTimer->deleteLater();
            d->syncTimer = nullptr;
        }
    }
}

void DFMSettings::setWatchChanges(bool watchChanges)
{
    Q_D(DFMSettings);

    if (d->watchChanges == watchChanges)
        return;

    d->watchChanges = watchChanges;

    if (watchChanges) {
        {
            QFileInfo info(d->settingFile);

            if (!info.exists()) {
                if (info.absoluteDir().mkpath(info.absolutePath())) {
                    QFile file(d->settingFile);

                    file.open(QFile::WriteOnly);
                }
            }
        }

        d->settingFileWatcher = new DFileWatcher(d->settingFile, this);
        d->settingFileWatcher->moveToThread(thread());

        connect(d->settingFileWatcher, SIGNAL(fileModified(DUrl)), this, SLOT(_q_onFileChanged(DUrl)));

        d->settingFileWatcher->startWatcher();
    } else {
        if (d->settingFileWatcher) {
            d->settingFileWatcher->deleteLater();
            d->settingFileWatcher = nullptr;
        }
    }
}

DFM_END_NAMESPACE

#include "moc_dfmsettings.cpp"
