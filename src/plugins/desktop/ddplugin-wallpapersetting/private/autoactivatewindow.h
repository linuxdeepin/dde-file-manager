// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTOACTIVATEWINDOW_H
#define AUTOACTIVATEWINDOW_H

#include "ddplugin_wallpapersetting_global.h"

#include <QObject>

namespace ddplugin_wallpapersetting {

class AutoActivateWindowPrivate;
class AutoActivateWindow : public QObject
{
    Q_OBJECT
public:
    explicit AutoActivateWindow(QObject *parent = nullptr);
    void setWatched(QWidget *);
    bool start();
    void stop();
signals:

public slots:
private:
    AutoActivateWindowPrivate *d;
};

}

#endif // AUTOACTIVATEWINDOW_H
