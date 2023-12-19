// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGBACKEND_H
#define SETTINGBACKEND_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/application/application.h>

#include <dsettingsbackend.h>

#include <QList>
#include <QVariant>

DCORE_USE_NAMESPACE

namespace dfmbase {
class SettingBackendPrivate;
class SettingBackend : public DSettingsBackend
{
    Q_OBJECT
public:
    static SettingBackend *instance();

    void setToSettings(DSettings *settings);

    QStringList keys() const;
    QVariant getOption(const QString &key) const;
    void doSync();

    using GetOptFunc = std::function<QVariant()>;
    using SaveOptFunc = std::function<void(const QVariant &)>;
    void addSettingAccessor(const QString &key, GetOptFunc get, SaveOptFunc set);
    void removeSettingAccessor(const QString &key);
    void addSettingAccessor(Application::ApplicationAttribute attr, SaveOptFunc set);
    void addSettingAccessor(Application::GenericAttribute attr, SaveOptFunc set);
    void addToSerialDataKey(const QString &key);
    void removeSerialDataKey(const QString &key);

Q_SIGNALS:
    void optionSetted(const QString &key, const QVariant &value);

public Q_SLOTS:
    void onOptionSetted(const QString &key, const QVariant &value);

protected:
    void doSetOption(const QString &key, const QVariant &value);
    void onValueChanged(int attribute, const QVariant &value);

    void initPresetSettingConfig();

    void initBasicSettingConfig();
    void initWorkspaceSettingConfig();
    void initAdvanceSettingConfig();

private:
    explicit SettingBackend(QObject *parent = nullptr);
    ~SettingBackend();

    QScopedPointer<SettingBackendPrivate> d;
};
}
#endif   // SETTINGBACKEND_H
