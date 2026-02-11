// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedialogmanagerdbus.h"
#include "dbus/filedialoghandledbus.h"
#include "filedialogadaptor.h"
#include "utils/appexitcontroller.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/event/event.h>

#include <QApplication>
#include <QDBusConnection>
#include <QUuid>

DFMBASE_USE_NAMESPACE
DIALOGCORE_USE_NAMESPACE

FileDialogManagerDBus::FileDialogManagerDBus(QObject *parent)
    : QObject(parent)
{
    connect(qApp, &QApplication::lastWindowClosed, this, [this]() {
        lastWindowClosed = true;
        onAppExit();
    });
}

QDBusObjectPath FileDialogManagerDBus::createDialog(QString key)
{
    if (key.isEmpty())
        key = QUuid::createUuid().toRfc4122().toHex();

    FileDialogHandleDBus *handle = new FileDialogHandleDBus();
    Q_UNUSED(new FiledialogAdaptor(handle));

    const QDBusObjectPath path("/com/deepin/filemanager/filedialog/" + key);

    if (curDialogObjectMap.contains(path)) {
        return path;
    }

    if (!QDBusConnection::sessionBus().registerObject(path.path(), handle)) {
        fmCritical("File Dialog: Cannot register to the D-Bus object.\n");
        handle->deleteLater();

        return QDBusObjectPath();
    }

    curDialogObjectMap[path] = handle;
    connect(handle, &FileDialogHandleDBus::destroyed, this, &FileDialogManagerDBus::onDialogDestroy);
    DIALOGCORE_NAMESPACE::AppExitController::instance().dismiss();
    initEventsFilter();
    return path;
}

void FileDialogManagerDBus::destroyDialog(const QDBusObjectPath &path)
{
    QObject *object = curDialogObjectMap.value(path);

    if (object)
        object->deleteLater();
}

QList<QDBusObjectPath> FileDialogManagerDBus::dialogs() const
{
    return curDialogObjectMap.keys();
}

/*!
 * \brief deprecated: Just compatibility
 */
QString FileDialogManagerDBus::errorString() const
{
    return {};
}

bool FileDialogManagerDBus::isUseFileChooserDialog() const
{
    return Application::instance()->genericAttribute(Application::kOverrideFileChooserDialog).toBool();
}

bool FileDialogManagerDBus::canUseFileChooserDialog(const QString &group, const QString &executableFileName) const
{
    const QString &groupName = QStringLiteral("DBusFileDialog");
    const QVariantMap &blackMap = Application::appObtuselySetting()->value(groupName, "disable").toMap();

    return !blackMap.value(group).toStringList().contains(executableFileName);
}

QStringList FileDialogManagerDBus::globPatternsForMime(const QString &mimeType) const
{
    DFMBASE_NAMESPACE::DMimeDatabase db;
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

/*!
 * \brief deprecated: Just compatibility
 */
QStringList FileDialogManagerDBus::monitorFiles() const
{
    return {};
}

void FileDialogManagerDBus::showBluetoothTransDialog(const QString &id, const QStringList &URIs)
{
    dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_SendFiles", URIs, id);
}

void FileDialogManagerDBus::onDialogDestroy()
{
    const QDBusObjectPath &path = curDialogObjectMap.key(QObject::sender());

    if (!path.path().isEmpty())
        curDialogObjectMap.remove(path);

    onAppExit();
}

void FileDialogManagerDBus::onAppExit()
{
    if (lastWindowClosed && curDialogObjectMap.size() == 0) {
        // after 1 minutes app exit if don't use filedialog
        DIALOGCORE_NAMESPACE::AppExitController::instance().readyToExit(60, [this]() {
            // last confirm exit
            if (lastWindowClosed && curDialogObjectMap.size() == 0)
                return true;
            return false;
        });
    }
}

void FileDialogManagerDBus::initEventsFilter()
{
    dpfSignalDispatcher->installGlobalEventFilter(this, [this](DPF_NAMESPACE::EventType type, const QVariantList &params) -> bool {
        if (type == GlobalEventType::kOpenFiles) {
            quint64 eventWinId { params.at(0).toULongLong() };
            QMap<QDBusObjectPath, QObject *>::iterator it = curDialogObjectMap.begin();
            for (; it != curDialogObjectMap.end(); ++it) {
                FileDialogHandleDBus *handle = qobject_cast<FileDialogHandleDBus *>(it.value());
                if (!handle)
                    continue;
                if (handle->winId() == eventWinId) {
                    FileDialog *dialog = qobject_cast<FileDialog *>(handle->widget());
                    if (!dialog)
                        continue;
                    dialog->onAcceptButtonClicked();
                    break;
                }
            }
            return true;
        }

        static QList<DPF_NAMESPACE::EventType> filterTypeGroup { GlobalEventType::kOpenNewTab,
                                                                 GlobalEventType::kOpenNewWindow,
                                                                 GlobalEventType::kOpenAsAdmin,
                                                                 GlobalEventType::kOpenFilesByApp,
                                                                 GlobalEventType::kCreateSymlink,
                                                                 GlobalEventType::kOpenInTerminal,
                                                                 GlobalEventType::kHideFiles };
        if (filterTypeGroup.contains(type))
            return true;

        return false;
    });
}
