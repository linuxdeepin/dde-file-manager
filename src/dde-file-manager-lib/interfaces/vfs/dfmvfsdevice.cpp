/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "private/dfmvfsdevice_p.h"

#include <QDebug>
#include <QJsonObject>
#include <QScopedPointer>
#include <QLoggingCategory>
#include <QThread>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(vfsDevice, "vfs.device")
#else
Q_LOGGING_CATEGORY(vfsDevice, "vfs.device", QtInfoMsg)
#endif

DFM_BEGIN_NAMESPACE

DFMVfsDevicePrivate::DFMVfsDevicePrivate(const QUrl &url, void *gmountObjectPtr, DFMVfsDevice *qq)
    : q_ptr(qq)
{
    m_setupUrl = url.scheme() == "device" ? url.path() : url.toString();
    c_GMount.reset((GMount *)gmountObjectPtr);
}

DFMVfsDevicePrivate::DFMVfsDevicePrivate(const QUrl &url, DFMVfsDevice *qq)
    : m_setupUrl(url.scheme() == "device" ? url.path() : url.toString())
    , q_ptr(qq)
{

}

DFMVfsDevicePrivate::~DFMVfsDevicePrivate()
{
    if (m_handler) {
        delete m_handler;
    }
}

GMount *DFMVfsDevicePrivate::createGMount() const
{
    QByteArray urlBa = m_setupUrl.toLatin1();
    const char *urlCStr = urlBa.data();

    GError *error = nullptr;
    DFMGFile file(g_file_new_for_uri(urlCStr));
    DFMGMount mount(g_file_find_enclosing_mount(file.data(), nullptr, &error));
    if (!mount || error) {
        qWarning() << "Error when creating GMount from GFile with this url: " << m_setupUrl;
        qWarning() << "Reason: " << QString::fromLocal8Bit(error->message);
        g_error_free(error);
    }

    if (!mount) {
        throw "DFMVfsDevicePrivate::createGMount() Method get called but create failed.";
    }

    return mount.take();
}

QUrl DFMVfsDevice::defaultUri() const
{
    Q_D(const DFMVfsDevice);

    DFMGFile defaultUriFile(g_mount_get_default_location(d->getGMount()));
    DFMGCChar defaultUriCStr(g_file_get_uri(defaultUriFile.data()));
    return QUrl(QString::fromLocal8Bit(defaultUriCStr.data()));
}


// caller should free the return value by calling `g_object_unref` or using `DFMGFile`.
GFile *DFMVfsDevicePrivate::createRootFile() const
{
    DFMGFile file(g_mount_get_root(getGMount()));
    if (!file) {
        throw "DFMVfsDevicePrivate::getRootFile() Method get called which requires DFMVfsDevice is attached but it's not attached";
    }
    return file.take();
}

// caller should free the return value by calling `g_object_unref` or using `DFMGFileInfo`.
GFileInfo *DFMVfsDevicePrivate::createRootFileInfo() const
{
    GError *error = nullptr;
    DFMGFileInfo fileInfo(g_file_query_filesystem_info(getGFile(), "filesystem::*", nullptr, &error));
    if (!fileInfo || error) {
        qWarning() << "Error when creating DFMVfsDevicePrivate::m_GFileInfo";
        qWarning() << "Reason: " << QString::fromLocal8Bit(error->message);
        g_error_free(error);
    }
    if (!fileInfo) {
        throw "DFMVfsDevicePrivate::createRootFileInfo() Method get called but fileinfo create failed.";
    }

    return fileInfo.take();
}

GMount *DFMVfsDevicePrivate::getGMount() const
{
    if (!c_GMount) {
        c_GMount.reset(createGMount());
    }

    return c_GMount.data();
}

GFile *DFMVfsDevicePrivate::getGFile() const
{
    if (!c_GFile) {
        c_GFile.reset(createRootFile());
    }

    return c_GFile.data();
}

GFileInfo *DFMVfsDevicePrivate::getGFileInfo() const
{
    if (!c_GFileInfo) {
        c_GFileInfo.reset(createRootFileInfo());
    }

    return c_GFileInfo.data();
}

QStringList DFMVfsDevicePrivate::getThemedIconName(GThemedIcon *icon)
{
    QStringList iconNames;
    if (icon) {
        char **names = nullptr;
        char **iter = nullptr;
        g_object_get(icon, "names", &names, NULL);
        for (iter = names; *iter; iter++) {
            iconNames.append(QString(*iter));
        }
        g_strfreev(names);
    }
    return iconNames;
}

// FIXME: who takes the ownership of the return value, should we `g_object_unref` it?
GMountOperation *DFMVfsDevicePrivate::GMountOperationNewMountOp(DFMVfsDevice *devicePtr)
{
    GMountOperation *op;

    op = g_mount_operation_new();

    g_signal_connect(op, "ask_password", (GCallback)&DFMVfsDevicePrivate::GMountOperationAskPasswordCb, devicePtr);
    g_signal_connect(op, "ask_question", (GCallback)&DFMVfsDevicePrivate::GMountOperationAskQuestionCb, devicePtr);

    /* dragondjf: we *should* also connect to the "aborted" signal but since the
     *            main thread is blocked handling input we won't get that signal
     *            anyway...
     */
    return op;
}

void DFMVfsDevicePrivate::GMountOperationAskPasswordCb(GMountOperation *op, const char *message, const char *default_user,
                                                       const char *default_domain, GAskPasswordFlags flags, gpointer vfsDevicePtr)
{
    DFMVfsDevice *device = static_cast<DFMVfsDevice *>(vfsDevicePtr);
    bool anonymous = g_mount_operation_get_anonymous(op);
    GPasswordSave passwordSave = g_mount_operation_get_password_save(op);

    const char *default_password = g_mount_operation_get_password(op);

    QJsonObject obj;
    obj.insert("message", message);
    obj.insert("anonymous", anonymous);
    obj.insert("username", default_user);
    obj.insert("domain", default_domain);
    obj.insert("password", default_password);
    obj.insert("GAskPasswordFlags", flags);
    obj.insert("passwordSave", passwordSave);

    qCDebug(vfsDevice()) << "GMountOperationAskPasswordCb() Default fields data" << obj;

    QJsonObject loginObj;
    if (device && device->eventHandler()) {
        DFMVfsAbstractEventHandler *handler = static_cast<DFMVfsAbstractEventHandler *>(device->eventHandler());
        loginObj = handler->handleAskPassword(obj);
    } else {
        qCDebug(vfsDevice()) << "GMountOperationAskPasswordCb(): No event handler registered to DFMVfsManager, use the default action.";
    }

    if (!loginObj.isEmpty()) {
        anonymous = loginObj.value("anonymous").toBool();
        QString username = loginObj.value("username").toString();
        QString domain = loginObj.value("domain").toString();
        QString password = loginObj.value("password").toString();
        GPasswordSave passwordsaveFlag =  static_cast<GPasswordSave>(loginObj.value("passwordSave").toInt());

        if ((flags & G_ASK_PASSWORD_ANONYMOUS_SUPPORTED) && anonymous) {
            g_mount_operation_set_anonymous(op, TRUE);
        } else {
            if (flags & G_ASK_PASSWORD_NEED_USERNAME) {
                g_mount_operation_set_username(op, username.toLocal8Bit().constData());
            }

            if (flags & G_ASK_PASSWORD_NEED_DOMAIN) {
                g_mount_operation_set_domain(op, domain.toLocal8Bit().constData());
            }

            if (flags & G_ASK_PASSWORD_NEED_PASSWORD) {
                g_mount_operation_set_password(op, password.toLocal8Bit().constData());
            }

            if (flags & G_ASK_PASSWORD_SAVING_SUPPORTED) {
                g_mount_operation_set_password_save(op, passwordsaveFlag);
            }
        }

        /* Only try anonymous access once. */
        if (anonymous) {
            g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        } else {
            g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
        }

    } else {
        qCDebug(vfsDevice()) << "cancel connect";
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
    }
}

// blumia: This callback is mainly for sftp fingerprint identity dialog, but should works on any ask-question signal.
//         ref: https://www.freedesktop.org/software/gstreamer-sdk/data/docs/latest/gio/GMountOperation.html#GMountOperation-ask-question
void DFMVfsDevicePrivate::GMountOperationAskQuestionCb(GMountOperation *op, const char *message, const GStrv choices, gpointer vfsDevicePtr)
{
    char **ptr = choices;
    int choice = 0;
    QStringList choiceList;
    DFMVfsDevice *device = static_cast<DFMVfsDevice *>(vfsDevicePtr);

    QString oneMessage(message);
    qCDebug(vfsDevice()) << "GMountOperationAskQuestionCb() message: " << message;

    while (ptr && *ptr) {
        QString oneOption = QString::asprintf("%s", *ptr++);
        qCDebug(vfsDevice()) << "GMountOperationAskQuestionCb()  - option(s): " << oneOption;
        choiceList << oneOption;
    }

    if (device && device->eventHandler()) {
        DFMVfsAbstractEventHandler *handler = static_cast<DFMVfsAbstractEventHandler *>(device->eventHandler());
        choice = handler->handleAskQuestion(oneMessage, choiceList);
    } else {
        qCDebug(vfsDevice()) << "GMountOperationAskQuestionCb(): No event handler registered to DFMVfsManager, use the default action.";
    }

    qCDebug(vfsDevice()) << "GMountOperationAskQuestionCb() user choice(start at 0): " << choice;

    // check if choose is invalid
    if (choice < 0 && choice >= choiceList.count()) {
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    g_mount_operation_set_choice(op, choice);
    g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);

    return;
}

void DFMVfsDevicePrivate::GFileMountDoneCb(GObject *object, GAsyncResult *res, gpointer vfsDevicePtr)
{
    gboolean succeeded;
    GError *error = NULL;
    DFMVfsDevice *device = static_cast<DFMVfsDevice *>(vfsDevicePtr);

    succeeded = g_file_mount_enclosing_volume_finish(G_FILE(object), res, &error);

    if (!succeeded && error) {
        Q_ASSERT(error->domain == G_IO_ERROR);

        int errorCode = error->code;
        QString errorMsg(error->message);

        if (device->eventHandler()) {
            DFMVfsAbstractEventHandler *handler = static_cast<DFMVfsAbstractEventHandler *>(device->eventHandler());
            handler->handleMountError(errorCode, errorMsg);
        } else {
            qCDebug(vfsDevice()) << "GFileMountDoneCb(): No event handler registered to DFMVfsManager, use the default action.";

            switch (error->code) {
            case G_IO_ERROR_FAILED_HANDLED: // Operation failed and a helper program has already interacted with the user. Do not display any error dialog.
                break;
            default:
                qCDebug(vfsDevice()) << "GFileMountDoneCb() mount failed. reason: " << errorMsg;
                break;
            }
        }

        g_error_free(error);
    }

    if (device->d_ptr->m_eventLoop) {
        device->d_ptr->m_eventLoop->exit(succeeded ? 0 : -1);
    }
}

void DFMVfsDevicePrivate::GFileUnmountDoneCb(GObject *object, GAsyncResult *res, gpointer vfsDevicePtr)
{
    gboolean succeeded;
    GError *error = NULL;
    DFMVfsDevice *device = static_cast<DFMVfsDevice *>(vfsDevicePtr);

    succeeded = g_mount_unmount_with_operation_finish(G_MOUNT(object), res, &error);

    if (!succeeded && error) {
        if (error->domain != G_IO_ERROR) return;

        int errorCode = error->code;
        QString errorMsg(error->message);

        if (device->eventHandler()) {
            DFMVfsAbstractEventHandler *handler = static_cast<DFMVfsAbstractEventHandler *>(device->eventHandler());
            handler->handleUnmountError(errorCode, errorMsg);
        } else {
            qCDebug(vfsDevice()) << "GFileUnmountDoneCb(): No event handler registered to DFMVfsManager, use the default action.";
            qCDebug(vfsDevice()) << "GFileUnmountDoneCb() unmount failed. reason: " << errorMsg;
        }

        g_error_free(error);
    }
}

/*! \class DFMVfsDevice

    \brief DFMVfsDevice allowed you query and manage a VFS mountpoint.

    Once a virtual filesystem (i.e. a remote/network device) got mounted. You can then manage this VFS by
    creating a new DFMVfsDevice instance. Since we use the mount URI as an identifier, it may failed when
    creating DFMVfsDevice instance, so using DFMVfsDevice::create() for creating and don't forget to check
    if create failed.

    DFMVfsManager::getVfsList() will return a full list with all attached(i.e. mounted) VFSes, using url
    from that function's return value is the recommend way to create a DFMVfsDevice instance.

    DFMVfsDevice is actually a GMount and GFileInfo wrapper for a GMount mountpoint, but it's designed for
    virtual filesystem here. For local filesystem management, use DDiskManager instead.

    \sa DFMVfsManager, DDiskManager
 */

DFMVfsDevice::DFMVfsDevice(const QUrl &url, void *gmountObjectPtr, QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMVfsDevicePrivate(url, gmountObjectPtr, this))
{

}

/*!
 * \brief Create a new DFMVfsDevice instance.
 *
 * Create a DFMVfsDevice instance with the given mountpoint URI. like `sftp://blumia@127.0.0.1/` or similar.
 * You can use DFMVfsManager::getVfsList to get a list of mounted VFS devices' URI which is ready for use to
 * create DFMVfsDevice instance.
 *
 * \param url mountpoint URI.
 *
 * \param parent useless for now, probably will be used later.
 *
 * \return the created DFMVfsDevice instance, or nullptr if failed.
 *
 * \sa DFMVfsManager::getVfsList
 */
DFMVfsDevice *DFMVfsDevice::create(const QUrl &url, QObject *parent)
{
    if (!url.isValid() || url.scheme() == "file" || url.scheme().isEmpty()) {
        return nullptr;
    }

    QString deviceId = url.scheme() == "device" ? url.path() : url.toString();

    QByteArray ba = deviceId.toLatin1();
    const char *urlCStr = ba.data();

    GError *error = nullptr;
    DFMGFile file(g_file_new_for_uri(urlCStr));
    DFMGMount mount(g_file_find_enclosing_mount(file.data(), nullptr, &error));
    if (!mount || error) {
        qWarning() << "Error when creating DFMVfsDevice with this url: " << url;
        qWarning() << "Reason: " << QString::fromLocal8Bit(error->message);
        g_error_free(error);
        return nullptr;
    }

    return new DFMVfsDevice(url, mount.take(), parent);
}

/*!
 * \brief Create a new DFMVfsDevice instance.
 *
 * This method create a DFMVfsDevice instance without checking if there is already a
 * attached vfs device. This can be used to create a vfs device to attach a new vfs
 * mountpoint to local.
 *
 * \note it's dangerous if you are creating a unmounted device and calling functions
 * which requires device existed and mounted. You can use createUnsafe() to create a
 * DFMVfsDevice instance even if the given url is not related to a devive (e.g.
 * `smb://10.0.12.161:445/` is not a url for device, `smb://10.0.12.161:445/share/`
 * is). It's useful for mounting a GFile with uri like this.
 *
 * \param url mountpoint URI.
 *
 * \param parent parent useless for now, probably will be used later.
 *
 * \return the created DFMVfsDevice instance, or nullptr if \a url is a file scheme url.
 */
DFMVfsDevice *DFMVfsDevice::createUnsafe(const QUrl &url, QObject *parent)
{
    if (url.scheme() == "file" || url.scheme().isEmpty()) {
        return nullptr;
    }

    if (url.scheme() != "device") {
        QUrl newUrl;
        newUrl.setScheme("device");
        newUrl.setPath(url.toString());
        return new DFMVfsDevice(newUrl, parent);
    }

    return new DFMVfsDevice(url, parent);
}

DFMVfsDevice::~DFMVfsDevice()
{

}

/*!
 * \brief Attach the device.
 *
 * This method should be called by DFMVfsManager in the most case, and it's okay to call
 * even if the SFMVfsDevice instance is not yet attached. (of course, this function is
 * to do the attach job).
 *
 * You can also use this function to mount uri like `smb://10.0.12.161:445/` , but this
 * kinds of mount won't trigger a DFMVfsManager::vfsAttached() (i.e. vfs device mounted)
 * signal.
 *
 * \return
 */
bool DFMVfsDevice::attach()
{
    Q_D(DFMVfsDevice);

    QPointer<QEventLoop> oldEventLoop = d->m_eventLoop;
    QEventLoop eventLoop;

    d->m_eventLoop = &eventLoop;

    DFMGFile file(g_file_new_for_uri(d->m_setupUrl.toUtf8().constData()));
    if (!file) {
        return false;
    }

    GMountOperation *op = DFMVfsDevicePrivate::GMountOperationNewMountOp(this);
    g_file_mount_enclosing_volume(file.data(), static_cast<GMountMountFlags>(0),
                                  op, nullptr, (GAsyncReadyCallback)&DFMVfsDevicePrivate::GFileMountDoneCb, this);

    int ret = d->m_eventLoop->exec();
    if (oldEventLoop) {
        oldEventLoop->exit(ret);
    }

    return ret == 0;
}

/*!
 * \brief Async detach device.
 *
 * \return if we start to do detach. will be false if it's not detachable.
 */
bool DFMVfsDevice::detachAsync()
{
    Q_D(DFMVfsDevice);

    if (!canDetach()) {
        return false;
    }

    GMountOperation *op = DFMVfsDevicePrivate::GMountOperationNewMountOp(this);
    g_mount_unmount_with_operation(d->getGMount(), G_MOUNT_UNMOUNT_NONE,
                                   op, nullptr, (GAsyncReadyCallback)&DFMVfsDevicePrivate::GFileUnmountDoneCb, this);

    return true;
}

DFMVfsAbstractEventHandler *DFMVfsDevice::eventHandler() const
{
    Q_D(const DFMVfsDevice);

    return d->m_handler;
}

void DFMVfsDevice::setEventHandler(DFMVfsAbstractEventHandler *handler, QThread *threadOfHandler)
{
    Q_D(DFMVfsDevice);

    d->m_handler = handler;
    d->m_threadOfEventHandler = threadOfHandler;
}

bool DFMVfsDevice::isReadOnly() const
{
    Q_D(const DFMVfsDevice);

    return g_file_info_get_attribute_boolean(d->getGFileInfo(), G_FILE_ATTRIBUTE_FILESYSTEM_READONLY);
}

bool DFMVfsDevice::canDetach() const
{
    Q_D(const DFMVfsDevice);

    return g_mount_can_unmount(d->getGMount());
}

quint64 DFMVfsDevice::totalBytes() const
{
    Q_D(const DFMVfsDevice);

    quint64 result;

    try {
        result = g_file_info_get_attribute_uint64(d->getGFileInfo(), G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
    } catch (const char *) {
        result = 0;
    }

    return result;
}

quint64 DFMVfsDevice::usedBytes() const
{
    Q_D(const DFMVfsDevice);

    quint64 result;

    try {
        result = g_file_info_get_attribute_uint64(d->getGFileInfo(), G_FILE_ATTRIBUTE_FILESYSTEM_USED);
    } catch (const char *) {
        result = 0;
    }

    return result;
}

quint64 DFMVfsDevice::freeBytes() const
{
    Q_D(const DFMVfsDevice);

    quint64 result;

    try {
        result = g_file_info_get_attribute_uint64(d->getGFileInfo(), G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
    } catch (const char *) {
        result = 0;
    }

    return result;
}

/*!
 * \brief Get the root path of the mount point
 *
 * e.g. "/run/user/1002/gvfs/smb-share:server=10.0.12.161,share=share"
 */
QString DFMVfsDevice::rootPath() const
{
    Q_D(const DFMVfsDevice);

    DFMGCChar rootUriCStr(g_file_get_path(d->getGFile()));
    return QString::fromLocal8Bit(rootUriCStr.data());
}

/*!
 * \brief Get the root uri of the mount point
 *
 * e.g. QUrl("smb://10.0.12.161/share/")
 */
QUrl DFMVfsDevice::rootUri() const
{
    Q_D(const DFMVfsDevice);

    DFMGCChar rootUriCStr(g_file_get_uri(d->getGFile()));
    return QUrl(QString::fromLocal8Bit(rootUriCStr.data()));
}




QString DFMVfsDevice::defaultPath() const
{
    Q_D(const DFMVfsDevice);

    DFMGFile defaultUriFile(g_mount_get_default_location(d->getGMount()));
    DFMGCChar defaultUriCStr(g_file_get_path(defaultUriFile.data()));
    return QString::fromLocal8Bit(defaultUriCStr.data());
}

QString DFMVfsDevice::name() const
{
    Q_D(const DFMVfsDevice);

    DFMGCChar vfsName(g_mount_get_name(d->getGMount()));
    return QString::fromLocal8Bit(vfsName.data());
}

/*!
 * \brief Get icon theme name with a fallback list
 */
QStringList DFMVfsDevice::iconList() const
{
    Q_D(const DFMVfsDevice);

    DFMGIcon icon(g_mount_get_icon(d->getGMount()));

    if (icon && G_IS_THEMED_ICON(icon.data())) {
        // blumia: QIcon::fromTheme accept the full name and will do the fallback when needed, so
        //         maybe we doesn't need such a list.
        return DFMVfsDevicePrivate::getThemedIconName(G_THEMED_ICON(icon.data()));
    }

    // use `gchar * g_icon_to_string ()` can get a path if it's not a GThemedIcon.
    // if it is, the result of `g_icon_to_string()` is simply the name (such as network-server).
    return {};
}

/*!
 * \brief Get symbolic icon name with a fallback list
 *
 * TODO: What's the differences between `g_mount_get_icon` and `g_mount_get_symbolic_icon`?
 */
QStringList DFMVfsDevice::symbolicIconList() const
{
    Q_D(const DFMVfsDevice);

    DFMGIcon icon(g_mount_get_symbolic_icon(d->getGMount()));

    if (icon && G_IS_THEMED_ICON(icon.data())) {
        return DFMVfsDevicePrivate::getThemedIconName(G_THEMED_ICON(icon.data()));
    }

    return {};
}

DFM_END_NAMESPACE
