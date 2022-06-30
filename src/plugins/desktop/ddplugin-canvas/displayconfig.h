/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
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
#ifndef DISPLAYCONFIG_H
#define DISPLAYCONFIG_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QMutex>
#include <QVariant>

class QSettings;
class QTimer;
class QThread;

namespace ddplugin_canvas {

class DisplayConfig : public QObject
{
    Q_OBJECT
public:
    static DisplayConfig *instance();
    QList<QString> profile();
    bool setProfile(const QList<QString> &profile);
    QHash<QString, QPoint> coordinates(const QString &key);
    bool setCoordinates(const QString &key, const QHash<QString, QPoint> &pos);
    void sortMethod(int &role, Qt::SortOrder &order);
    bool setSortMethod(const int &role, const Qt::SortOrder &order);
    bool autoAlign();
    void setAutoAlign(bool align);
    int iconLevel();
    bool setIconLevel(int lv);
    bool customWaterMask();
public slots:
    void sync();
protected:
    explicit DisplayConfig(QObject *parent = nullptr);
    ~DisplayConfig();

    QVariant value(const QString &group, const QString &key, const QVariant &defaultVar);
    void setValues(const QString &group, const QHash<QString, QVariant> &values);
    void remove(const QString &group, const QString &key);
    void remove(const QString &group, const QStringList &keys);
    QString path() const;
private:
    static bool covertPostion(const QString &strPos, QPoint &pos);
    static QString covertPostion(const QPoint &pos);
private:
    Q_DISABLE_COPY(DisplayConfig)

    QMutex mtxLock;
    QSettings *settings = nullptr;
    QTimer *syncTimer = nullptr;
    QThread *workThread = nullptr;
};

#define DispalyIns DisplayConfig::instance()

}

#endif   // DISPLAYCONFIG_H
