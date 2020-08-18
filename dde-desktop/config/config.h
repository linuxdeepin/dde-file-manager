/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#pragma once

#include <QObject>
#include <QSettings>
#include <QMutex>
#include <QTimer>
#include "../global/singleton.h"

class Config: public QObject, public Singleton<Config>
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

public slots:
    void setConfig(const QString &group, const QString &key, const QVariant &value);
    void removeConfig(const QString &group, const QString &key);
    void setConfigList(const QString &group, const QStringList &keys, const QVariantList &values);
    void removeConfigList(const QString &group, const QStringList &keys);
    void sync();
private:
    explicit Config();
    Q_DISABLE_COPY(Config)
    friend class Singleton<Config>;

    QMutex  m_mtxLock;
    QSettings *m_settings = nullptr;
    QTimer m_syncTimer;
};
