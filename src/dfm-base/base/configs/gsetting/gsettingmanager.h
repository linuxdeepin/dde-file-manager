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
#ifndef GSETTINGMANAGER_H
#define GSETTINGMANAGER_H

#include "dfm_base_global.h"

#include <QObject>
#include <QVariant>

class QGSettings;

DFMBASE_BEGIN_NAMESPACE

class GSettingManagerPrivate;
class GSettingManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GSettingManager)

public:
    static GSettingManager *instance();

    static bool isSchemaInstalled(const QString &schemaId);

    bool addSettings(const QString &schemaId, const QString &path = QString(), QString *err = nullptr);
    bool removeSettings(const QString &schemaId, QString *err = nullptr);
    QGSettings *setting(const QString &schemaId) const;

    QVariant get(const QString &schemaId, const QString &key) const;
    void set(const QString &schemaId, const QString &key, const QVariant &value);
    bool trySet(const QString &schemaId, const QString &key, const QVariant &value);
    QStringList keys(const QString &schemaId) const;
    QVariantList choices(const QString &schemaId, const QString &key) const;
    void reset(const QString &schemaId, const QString &key);

Q_SIGNALS:
    void valueChanged(const QString &schemaId, const QString &key);

private:
    explicit GSettingManager(QObject *parent = nullptr);
    virtual ~GSettingManager() override;

private:
    QScopedPointer<GSettingManagerPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // GSETTINGMANAGER_H
