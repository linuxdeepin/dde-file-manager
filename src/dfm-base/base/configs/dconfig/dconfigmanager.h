// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONFIGMANAGER_H
#define DCONFIGMANAGER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/configs/dconfig/global_dconf_defines.h>

#include <QObject>
#include <QVariant>

DFMBASE_BEGIN_NAMESPACE

class DConfigManagerPrivate;
class DConfigManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DConfigManager)

public:
    static DConfigManager *instance();

    bool addConfig(const QString &config, QString *err = nullptr);
    bool removeConfig(const QString &config, QString *err = nullptr);

    QStringList keys(const QString &config) const;
    bool contains(const QString &config, const QString &key) const;
    QVariant value(const QString &config, const QString &key, const QVariant &fallback = QVariant()) const;
    void setValue(const QString &config, const QString &key, const QVariant &value);

    bool validateConfigs(QStringList &invalidConfigs) const;

Q_SIGNALS:
    void valueChanged(const QString &config, const QString &key);

private:
    explicit DConfigManager(QObject *parent = nullptr);
    virtual ~DConfigManager() override;

private:
    QScopedPointer<DConfigManagerPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // DCONFIGMANAGER_H
