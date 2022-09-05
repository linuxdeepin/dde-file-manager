// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENMANAGER_P_H
#define SCREENMANAGER_P_H

#include "abstractscreenmanager.h"

#include <QTimer>
#include <QMultiMap>

class ScreenManagerPrivate
{
public:
    explicit ScreenManagerPrivate(AbstractScreenManager *p);
    ~ScreenManagerPrivate();
    void readyShot(int wait = 50);
public:
    QTimer m_eventShot;      //延迟处理定时器
    QMultiMap<AbstractScreenManager::Event,qint64> m_events;    //事件池
    AbstractScreenManager *q;    
};

#endif // SCREENMANAGER_P_H
