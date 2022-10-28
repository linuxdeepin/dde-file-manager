/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#ifndef SETTINGBACKEND_H
#define SETTINGBACKEND_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/base/application/application.h"

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

    QStringList keys() const;
    QVariant getOption(const QString &key) const;
    void doSync();

    using GetOptFunc = std::function<QVariant()>;
    using SaveOptFunc = std::function<void(const QVariant &)>;
    void addSettingAccessor(const QString &key, GetOptFunc get, SaveOptFunc set);
    void addSettingAccessor(Application::ApplicationAttribute attr, SaveOptFunc set);
    void addSettingAccessor(Application::GenericAttribute attr, SaveOptFunc set);
    void addToSerialDataKey(const QString &key);

protected:
    void doSetOption(const QString &key, const QVariant &value);
    void onValueChanged(int attribute, const QVariant &value);

private:
    explicit SettingBackend(QObject *parent = nullptr);
    ~SettingBackend();

    QScopedPointer<SettingBackendPrivate> d;
};
}
#endif   // SETTINGBACKEND_H
