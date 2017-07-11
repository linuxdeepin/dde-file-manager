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
#include "../global/singleton.h"

class QScreen;
class DBusDisplay;
class Display: public QObject, public Singleton<Display>
{
    Q_OBJECT

    friend class Singleton<Display>;
public:
    explicit Display(QObject *parent = 0);

    QScreen *primaryScreen();

signals:
    void primaryScreenChanged(QScreen *screen);

private:
    DBusDisplay *m_display = nullptr;
};

