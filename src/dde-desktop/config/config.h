/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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
