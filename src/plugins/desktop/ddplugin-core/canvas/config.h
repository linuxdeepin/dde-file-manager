/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>
#include <QMutex>
#include <QTimer>

class Config : public QObject
{
    Q_OBJECT

public:
    static inline Config *instance()
    {
        static Config config;
        return &config;
    }
    inline QMutex *mutex() { return &mtxLock; }
    inline QSettings *settings() { return settingsObj; }
    QVariant getConfig(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());

    static const QString kGroupGeneral;
    static const QString kKeyProfile;
    static const QString kKeySortBy;
    static const QString kKeySortOrder;
    static const QString kKeyAutoAlign;
    static const QString kKeyIconLevel;
    static const QString kKeyQuickHide;
    static const QString kKeyAutoMerge;
    static const QString kKeyWaterMask;

signals:

public slots:
    void setConfig(const QString &group, const QString &key, const QVariant &value);
    void removeConfig(const QString &group, const QString &key);
    void setConfigList(const QString &group, const QStringList &keys, const QVariantList &values);
    void removeConfigList(const QString &group, const QStringList &keys);
    void sync();

protected:
private:
    explicit Config();
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    QMutex mtxLock;
    QSettings *settingsObj { nullptr };
    QTimer syncTimer;
};

#endif   // CONFIG_H
