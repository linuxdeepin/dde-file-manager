#ifndef SCREENMANAGER_P_H
#define SCREENMANAGER_P_H

#include "abstractscreenmanager.h"

#include <QTimer>
#include <QMultiMap>

class ScreenManagerPrivate
{
public:
    explicit ScreenManagerPrivate(AbstractScreenManager *p) : q(p){}
    ~ScreenManagerPrivate();
    void readyShot(int wait = 50);
public:
    QTimer *m_eventShot = nullptr;      //延迟处理定时器
    QMultiMap<AbstractScreenManager::Event,qint64> m_events;    //事件池
    AbstractScreenManager *q;
};

#endif // SCREENMANAGER_P_H
