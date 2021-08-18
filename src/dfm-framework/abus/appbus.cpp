#include "appbus.h"
#include "private/appbus_p.h"

DPF_USE_NAMESPACE

AppBus::AppBus(QObject *parent)
    : QObject(parent),
      d(new AppBusPrivate(this))
{

}

AppBus::~AppBus()
{

}

QStringList AppBus::onlineServer()
{
    return d->onlineServers.keys();
}

bool AppBus::isMimeServer(const QString &serverName)
{
    return d->appServerName == serverName;
}

QString AppBus::mimeServer()
{
    return d->appServerName;
}
