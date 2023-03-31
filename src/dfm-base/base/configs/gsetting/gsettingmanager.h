// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GSETTINGMANAGER_H
#define GSETTINGMANAGER_H

#include <dfm-base/dfm_base_global.h>

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
