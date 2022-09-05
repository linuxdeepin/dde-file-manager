// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSettings>
#include <QMutex>
#include <QTimer>
#include "../global/singleton.h"

class Config: public QObject, public DDEDesktop::Singleton<Config>
{
    Q_OBJECT
public:
    inline QMutex *mutex() {return &m_mtxLock;}
    inline QSettings *settings()  {return m_settings;}
    QVariant getConfig(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());

    static const QString groupGeneral;
    static const QString keyProfile;
    static const QString keySortBy;
    static const QString keySortOrder;
    static const QString keyAutoAlign;
    static const QString keyIconLevel;
    static const QString keyQuickHide;
    static const QString keyAutoMerge;
    static const QString keyWaterMask;
    static const QString keyIconSizeMode;
public slots:
    void setConfig(const QString &group, const QString &key, const QVariant &value);
    void removeConfig(const QString &group, const QString &key);
    void setConfigList(const QString &group, const QStringList &keys, const QVariantList &values);
    void removeConfigList(const QString &group, const QStringList &keys);
    void sync();
private:
    explicit Config();
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;
    friend class DDEDesktop::Singleton<Config>;

    QMutex  m_mtxLock;
    QSettings *m_settings = nullptr;
    QTimer m_syncTimer;
};
