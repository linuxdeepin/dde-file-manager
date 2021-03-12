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
