// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWSETTINGUTIL_H
#define VIEWSETTINGUTIL_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QTimer>

class QKeyEvent;
class QMouseEvent;
namespace ddplugin_canvas {

class ViewSettingUtil : public QObject
{
    Q_OBJECT
public:
    explicit ViewSettingUtil(QObject *parent = nullptr);
    void checkTouchDrag(QMouseEvent *event);
    bool isDelayDrag() const;
signals:

public slots:
private:
    QTimer touchDragTimer; // delay to start drag when touch moving
};

}

#endif // VIEWSETTINGUTIL_H
