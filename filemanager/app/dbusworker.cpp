#include "dbusworker.h"
#include "../app/global.h"
#include <QTimer>

DBusWorker::DBusWorker(QObject *parent) :
    QObject(parent)
{
    m_requestTimer = new QTimer(this);
    m_requestTimer->setInterval(500);
    connect(m_requestTimer, SIGNAL(timeout()),
            this, SLOT(loadDesktopItems()));
    connect(signalManager, SIGNAL(startRequest()),
            this, SLOT(start()));
    connect(signalManager, SIGNAL(stopRequest()),
            this, SLOT(stop()));
}

void DBusWorker::start(){
    qDebug() << "start request desktop items from dbus";
    loadDesktopItems();
    m_requestTimer->start();
}

void DBusWorker::stop(){
    disconnect(m_requestTimer, SIGNAL(timeout()),
            this, SLOT(loadDesktopItems()));
    m_requestTimer->stop();
    qDebug() << "request desktop items from dbus finished";

}

void DBusWorker::loadDesktopItems(){
    m_count += 1;
    qDebug() << "loadDesktopItems" << m_count;
    if (m_count < 100){
        dbusController->loadDesktopItems();
    }else{
        stop();
    }
}
