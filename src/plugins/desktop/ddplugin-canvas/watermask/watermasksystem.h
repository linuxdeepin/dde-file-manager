// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WATERMASKSYSTEM_H
#define WATERMASKSYSTEM_H

#include "ddplugin_canvas_global.h"
#include "deepinlicensehelper.h"

#include <QWidget>

class QLabel;

namespace ddplugin_canvas {
class WatermaskSystem : public QObject
{
    Q_OBJECT
public:
    explicit WatermaskSystem(QWidget *parent = nullptr);
    static bool isEnable();
    static bool usingCn();
    static bool showLicenseState();
    static void getEditonResource(const QString &root, QString *logo, QString *text);
    static QPixmap maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio);
    void refresh();
    void stackUnder(QWidget *);
    void updatePosition();
signals:
    void showedOn(const QPoint &pos);
public slots:
protected:
    static void getResource(const QString &root, const QString &lang, QString *logo, QString *text);
    static void findResource(const QString &dirPath, const QString &lang, QString *logo, QString *text);
protected slots:
    void stateChanged(int state, int prop);
protected:
    inline QWidget *parentWidget() {
        return qobject_cast<QWidget *>(parent());
    }
    void loadConfig();
protected:
    QLabel *logoLabel = nullptr;
    QLabel *textLabel = nullptr;
};

}

#endif // WATERMASKSYSTEM_H
