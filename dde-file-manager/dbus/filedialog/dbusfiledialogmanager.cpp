#include "dbusfiledialogmanager.h"
#include "dbusfiledialoghandle.h"
#include "dfmstandardpaths.h"
#include "filedialog_adaptor.h"
#include "app/define.h"
#include "singleton.h"
#include "interfaces/dfmsetting.h"
#include "interfaces/dfmstandardpaths.h"

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

    const QDBusObjectPath path("/com/deepin/filemanager/filedialog/" + key);

    if (m_dialogObjectMap.contains(path)) {
        return path;
    }

    if (!QDBusConnection::sessionBus().registerObject(path.path(), handle)) {
        qWarning("Cannot register to the D-Bus object.\n");
        handle->deleteLater();

        return QDBusObjectPath();
    }

    m_dialogObjectMap[path] = handle;
    connect(handle, &DBusFileDialogHandle::destroyed, this, &DBusFileDialogManager::onDialogDestroy);

    return path;
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

bool DBusFileDialogManager::isUseFileChooserDialog() const
{
    return globalSetting->isDefaultChooserDialog();
}

bool DBusFileDialogManager::canUseFileChooserDialog(const QString &group, const QString &executableFileName) const
{
#ifdef QT_NO_DEBUG
    QSettings blackList(DFMStandardPaths::standardLocation(DFMStandardPaths::DbusFileDialogConfPath), QSettings::NativeFormat);
#else
    QSettings blackList(QString("%1/dbus/filedialog/%2").arg(PRO_FILE_PWD).arg("dbus_filedialog_blacklist.conf"), QSettings::NativeFormat);
#endif

    blackList.beginGroup(group);

    return !blackList.value(executableFileName, false).toBool();
}

QStringList DBusFileDialogManager::globPatternsForMime(const QString &mimeType) const
{
    QMimeDatabase db;
    QMimeType mime(db.mimeTypeForName(mimeType));
    if (mime.isValid()) {
        if (mime.isDefault()) {
            return QStringList(QStringLiteral("*"));
        } else {
            return mime.globPatterns();
        }
    }
    return QStringList();
}

void DBusFileDialogManager::onDialogDestroy()
{
    const QDBusObjectPath &path = m_dialogObjectMap.key(QObject::sender());

    if (!path.path().isEmpty())
        m_dialogObjectMap.remove(path);
}
