/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusfiledialogmanager.h"
#include "dbusfiledialoghandle.h"
#include "dfmstandardpaths.h"
#include "filedialog_adaptor.h"
#include "app/define.h"
#include "singleton.h"
#include "interfaces/dfmstandardpaths.h"
#include "dfilewatcher.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "views/windowmanager.h"
#include "dfileservices.h"
#include "bluetooth/bluetoothtransdialog.h"

#include <QDBusConnection>

DFM_USE_NAMESPACE

DBusFileDialogManager::DBusFileDialogManager(QObject *parent)
    : QObject(parent)
{

}

QDBusObjectPath DBusFileDialogManager::createDialog(QString key)
{
    if (key.isEmpty())
        key = QUuid::createUuid().toRfc4122().toHex();

    g_isFileDialogMode = true;

    if (!initJobDone) {
        DFMGlobal::initGvfsMountManager();
        DFMGlobal::initDeviceListener();

        /*init searchHistoryManager */
        DFMGlobal::initSearchHistoryManager();

        /*init bookmarkManager */
        DFMGlobal::initBookmarkManager();

        /*init fileMenuManager */
        DFMGlobal::initFileMenuManager();

        /*init fileSignalManger */
        DFMGlobal::initFileSiganlManager();

        /*init appController */
        DFMGlobal::initAppcontroller();

        /*init fileService */
        DFMGlobal::initFileService();

        /*init systemPathMnager */
        DFMGlobal::initSystemPathManager();

        /*init mimeTypeDisplayManager */
        DFMGlobal::initMimeTypeDisplayManager();

        /*init networkManager */
        DFMGlobal::initNetworkManager();

        /*init secretManger */
        DFMGlobal::initSecretManager();

        /*init userShareManager */
        DFMGlobal::initUserShareManager();

        /*init initDialogManager */
        DFMGlobal::initDialogManager();

        /*init controllers for different scheme*/
        fileService->initHandlersByCreators();

        WindowManager::instance();

        initJobDone = true;
    }

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
    return DFMApplication::instance()->genericAttribute(DFMApplication::GA_OverrideFileChooserDialog).toBool();
}

bool DBusFileDialogManager::canUseFileChooserDialog(const QString &group, const QString &executableFileName) const
{
    const QString &group_name = QStringLiteral("DBusFileDialog");
    const QVariantMap &black_map = DFMApplication::appObtuselySetting()->value(group_name, "disable").toMap();

    return !black_map.value(group).toStringList().contains(executableFileName);
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

QStringList DBusFileDialogManager::monitorFiles() const
{
    return DFileWatcher::getMonitorFiles();
}

void DBusFileDialogManager::showBluetoothTransDialog(const QString &id, const QStringList &URIs)
{
    BluetoothTransDialog *dlg = new BluetoothTransDialog(URIs, BluetoothTransDialog::DirectlySend, id);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void DBusFileDialogManager::onDialogDestroy()
{
    const QDBusObjectPath &path = m_dialogObjectMap.key(QObject::sender());

    if (!path.path().isEmpty())
        m_dialogObjectMap.remove(path);
}
