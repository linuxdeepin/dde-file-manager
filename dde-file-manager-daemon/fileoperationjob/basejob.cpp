#include "basejob.h"
#include "app/policykithelper.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>

BaseJob::BaseJob(QObject *parent) : QObject(parent)
{

}

BaseJob::~BaseJob()
{
    QDBusConnection::systemBus().unregisterObject(m_objectPath);
}

qint64 BaseJob::getClientPid()
{
    qint64 pid = 0;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if(c.isConnected())
    {
       pid = c.interface()->servicePid(message().service()).value();
    }
    return pid;
}

bool BaseJob::checkAuthorization(const QString &actionId, qint64 applicationPid)
{
    bool isAuthenticationSucceeded(false);
    qint64 pid = getClientPid();
    if (pid){
        isAuthenticationSucceeded = PolicyKitHelper::instance()->checkAuthorization(actionId, applicationPid);
    }
    return isAuthenticationSucceeded;
}

QString BaseJob::objectPath() const
{
    return m_objectPath;
}

void BaseJob::setObjectPath(const QString &objectPath)
{
    m_objectPath = objectPath;
}

bool BaseJob::registerObject()
{
    bool result = QDBusConnection::systemBus().registerObject(objectPath(), this);
    if (!result){
        deleteLater();
    }
    return result;
}


