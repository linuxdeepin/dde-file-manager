#include "dbusfilemanager1.h"

#include <QProcess>

DBusFileManager1::DBusFileManager1(QObject *parent)
    : QObject(parent)
{

}

void DBusFileManager1::ShowFolders(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    QProcess::startDetached("dde-file-manager", URIs);
}

void DBusFileManager1::ShowItemProperties(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    QProcess::startDetached("dde-file-manager", QStringList() << "-p" << URIs);
}

void DBusFileManager1::ShowItems(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" <<  URIs);
}
