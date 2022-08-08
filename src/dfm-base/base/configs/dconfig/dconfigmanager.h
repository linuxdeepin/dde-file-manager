/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef DCONFIGMANAGER_H
#define DCONFIGMANAGER_H

#include "dfm_base_global.h"

#include <QObject>
#include <QVariant>

DFMBASE_BEGIN_NAMESPACE

inline constexpr char kDefaultCfgPath[] { "org.deepin.dde.file-manager" };
inline constexpr char kKeyHideDisk[] { "dfm.disk.hidden" };

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
