// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
