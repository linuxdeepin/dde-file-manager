// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMVFSDEVICE_P_H
#define DFMVFSDEVICE_P_H

#include "dfmvfsdevice.h"
#include "private/dfmgiowrapper_p.h"

DFM_BEGIN_NAMESPACE

class DFMVfsDevicePrivate
{
    Q_DECLARE_PUBLIC(DFMVfsDevice)

public:
    explicit DFMVfsDevicePrivate(const QUrl &url, void *gmountObjectPtr, DFMVfsDevice *qq);
    explicit DFMVfsDevicePrivate(const QUrl &url, DFMVfsDevice *qq);
    ~DFMVfsDevicePrivate();

    GMount* createGMount() const;
    GFile* createRootFile() const;
    GFileInfo* createRootFileInfo() const;

    GMount* getGMount() const; // promise will always return a value, or throw an exception
    GFile* getGFile() const; // promise will always return a value, or throw an exception
    GFileInfo* getGFileInfo() const; // promise will always return a value, or throw an exception

    static QStringList getThemedIconName(GThemedIcon *icon);

    static GMountOperation *GMountOperationNewMountOp(DFMVfsDevice *devicePtr);
    static void GMountOperationAskPasswordCb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags, gpointer vfsDevicePtr);
    static void GMountOperationAskQuestionCb(GMountOperation *op, const char *message, const GStrv choices, gpointer vfsDevicePtr);
    static void GFileMountDoneCb(GObject *object, GAsyncResult *res, gpointer vfsDevicePtr);
    static void GFileUnmountDoneCb(GObject *object, GAsyncResult *res, gpointer vfsDevicePtr);

    QString m_setupUrl; // url used to setup this DFMVfsDevice.
    QPointer<QEventLoop> m_eventLoop; // to make async event sync.

    DFMVfsAbstractEventHandler *m_handler = nullptr;
    QPointer<QThread> m_threadOfEventHandler;

    DFMVfsDevice *q_ptr = nullptr;

private:
    mutable DFMGMount c_GMount; // cached value, default null, mountpoint file.
    mutable DFMGFile c_GFile; // cached value, default null, mountpoint file.
    mutable DFMGFileInfo c_GFileInfo; // cached value, default null, mountpoint fileinfo.
};

DFM_END_NAMESPACE

#endif // DFMVFSDEVICE_P_H
