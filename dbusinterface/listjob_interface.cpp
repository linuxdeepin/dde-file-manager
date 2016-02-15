#include "listjob_interface.h"

ListJobInterface::ListJobInterface(const QString &service, const QString &path,
                                   const QString &interface, QObject *parent) :
    QDBusAbstractInterface(service, path, interface.toLatin1().constData(),
                           QDBusConnection::sessionBus(), parent)
{
    qDBusRegisterMetaType<FileItemInfoList>();
}

ListJobInterface::~ListJobInterface()
{
    Abort();
}
