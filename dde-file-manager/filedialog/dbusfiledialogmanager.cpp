#include "dbusfiledialogmanager.h"
#include "dbusfiledialoghandle.h"
#include "filedialog_adaptor.h"

#include <QDBusConnection>

DBusFileDialogManager::DBusFileDialogManager(QObject *parent)
    : QObject(parent)
{

}

QDBusObjectPath DBusFileDialogManager::createDialog(QString key)
{
    if (key.isEmpty())
        key = QUuid::createUuid().toRfc4122().toHex();

    DBusFileDialogHandle *handle = new DBusFileDialogHandle();
    Q_UNUSED(new FiledialogAdaptor(handle));

    const QString &path = "/com/deepin/filemanager/filedialog/" + key;

    if (!QDBusConnection::sessionBus().registerObject(path, handle)) {
        qWarning("Cannot register to the D-Bus object.\n");
        handle->deleteLater();

        return QDBusObjectPath();
    }

    m_dialogObjectMap[QDBusObjectPath(path)] = handle;
    connect(handle, &DBusFileDialogHandle::destroyed, this, &DBusFileDialogManager::onDialogDestroy);

    return QDBusObjectPath(path);
}

void DBusFileDialogManager::destroyDialog(const QDBusObjectPath &path)
{
    QObject *object = m_dialogObjectMap.value(path);

    if (object)
        object->deleteLater();
}

QList<QDBusObjectPath> DBusFileDialogManager::dialogs() const
{
    return m_dialogObjectMap.keys();
}

QString DBusFileDialogManager::errorString() const
{
    return m_errorString;
}

void DBusFileDialogManager::onDialogDestroy()
{
    const QDBusObjectPath &path = m_dialogObjectMap.key(QObject::sender());

    if (!path.path().isEmpty())
        m_dialogObjectMap.remove(path);
}
