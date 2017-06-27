#include "dbusfilemanager1.h"
#include "dfilewatcher.h"

#include <QProcess>

DBusFileManager1::DBusFileManager1(QObject *parent)
    : QObject(parent)
{

}

void DBusFileManager1::ShowFolders(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    if (QProcess::startDetached("file-manager.sh", URIs))
        return;

    QProcess::startDetached("dde-file-manager", URIs);
}

void DBusFileManager1::ShowItemProperties(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    if (QProcess::startDetached("file-manager.sh", QStringList() << "-p" << URIs))
        return;

    QProcess::startDetached("dde-file-manager", QStringList() << "-p" << URIs);
}

void DBusFileManager1::ShowItems(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--show-item" <<  URIs))
        return;

    QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" <<  URIs);
}

QStringList DBusFileManager1::GetMonitorFiles() const
{
    return DFileWatcher::getMonitorFiles();
}
