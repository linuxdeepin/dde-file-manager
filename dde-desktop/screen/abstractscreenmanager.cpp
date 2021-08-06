#include "abstractscreenmanager.h"
#include "abstractscreenmanager_p.h"

#include <QDebug>

ScreenManagerPrivate::~ScreenManagerPrivate()
{
    if (m_eventShot)
        delete m_eventShot;
    m_eventShot = nullptr;
}

void ScreenManagerPrivate::readyShot(int wait)
{
    if (m_eventShot != nullptr) {
        m_eventShot->stop();
        delete m_eventShot;
        m_eventShot = nullptr;
        qDebug() << "shotting later " << wait;
    }

    if (wait < 1) {
        wait = 1;
    }

    m_eventShot = new QTimer;
    QObject::connect(m_eventShot,&QTimer::timeout,q,[=]() {
        m_eventShot->stop();
        //事件优先级。由上往下，背景和画布模块在处理上层的事件已经处理过下层事件的涉及的改变，因此直接忽略
        if (m_events.contains(AbstractScreenManager::Mode)) {
            emit q->sigDisplayModeChanged();
        }
        else if (m_events.contains(AbstractScreenManager::Screen)) {
            emit q->sigScreenChanged();
        }
        else if (m_events.contains(AbstractScreenManager::Geometry)) {
            emit q->sigScreenGeometryChanged();
        }
        else if (m_events.contains(AbstractScreenManager::AvailableGeometry)) {
            emit q->sigScreenAvailableGeometryChanged();
        }
        m_events.clear();
    });

    m_eventShot->start(wait);
}

AbstractScreenManager::AbstractScreenManager(QObject *parent)
    : QObject(parent)
    ,d(new ScreenManagerPrivate(this))
{

}

AbstractScreenManager::~AbstractScreenManager()
{
    delete d;
}

void AbstractScreenManager::appendEvent(AbstractScreenManager::Event e)
{
    qDebug() << "append event" << e << "current size" << (d->m_events.size() + 1);
    //收集短时间内爆发出的事件，合并处理，优化响应速度
    d->m_events.insert(e,0);
    d->readyShot(100);
}
