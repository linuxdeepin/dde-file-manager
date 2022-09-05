// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSETTINGS_H
#define DFMSETTINGS_H

#include <QObject>
#include <QVariantHash>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMSettingsPrivate;
class DFMSettings : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMSettings)

    Q_PROPERTY(bool autoSync READ autoSync WRITE setAutoSync)
#ifndef DFM_NO_FILE_WATCHER
    Q_PROPERTY(bool watchChanges READ watchChanges WRITE setWatchChanges)
#endif

public:
    enum ConfigType {
        AppConfig,
        GenericConfig
    };

    explicit DFMSettings(const QString &defaultFile, const QString &fallbackFile, const QString &settingFile, QObject *parent = nullptr);
    explicit DFMSettings(const QString &name, ConfigType type = AppConfig, QObject *parent = nullptr);
    ~DFMSettings();

    bool contains(const QString &group, const QString &key) const;

    QSet<QString> groups() const;
    QSet<QString> keys(const QString &group) const;
    QStringList keyList(const QString &group) const;

    static DUrl toUrlValue(const QVariant &url);

    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) const;
    QVariant value(const QString &group, const DUrl &key, const QVariant &defaultValue = QVariant()) const;
    DUrl urlValue(const QString &group, const QString &key, const DUrl &defaultValue = DUrl()) const;
    DUrl urlValue(const QString &group, const DUrl &key, const DUrl &defaultValue = DUrl()) const;
    void setValue(const QString &group, const QString &key, const QVariant &value);
    void setValue(const QString &group, const DUrl &key, const QVariant &value);
    // if changed return true
    bool setValueNoNotify(const QString &group, const QString &key, const QVariant &value);
    bool setValueNoNotify(const QString &group, const DUrl &key, const QVariant &value);

    void removeGroup(const QString &group);
    bool isRemovable(const QString &group, const QString &key) const;
    bool isRemovable(const QString &group, const DUrl &key) const;
    void remove(const QString &group, const QString &key);
    void remove(const QString &group, const DUrl &key);
    void clear();
    void reload();

    bool sync();

    bool autoSync() const;
#ifndef DFM_NO_FILE_WATCHER
    bool watchChanges() const;
#endif

public Q_SLOTS:
    void setAutoSync(bool autoSync);
#ifndef DFM_NO_FILE_WATCHER
    void setWatchChanges(bool watchChanges);
#endif

Q_SIGNALS:
    void valueChanged(const QString &group, const QString &key, const QVariant &value);
    void valueEdited(const QString &group, const QString &key, const QVariant &value);

private:
    QScopedPointer<DFMSettingsPrivate> d_ptr;

#ifndef DFM_NO_FILE_WATCHER
    void onFileChanged(const DUrl &);
#endif
};

DFM_END_NAMESPACE

#endif // DFMSETTINGS_H
