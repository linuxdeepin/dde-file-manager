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

#ifndef DFMVFSDEVICE_H
#define DFMVFSDEVICE_H

#include "dfmvfsabstracteventhandler.h"
#include <dfmglobal.h>

#include <QObject>
#include <QUrl>

DFM_BEGIN_NAMESPACE

class DFMVfsDevicePrivate;
class DFMVfsDevice : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isReadOnly READ isReadOnly)
    Q_PROPERTY(bool canDetach READ canDetach)
    Q_PROPERTY(quint64 totalBytes READ totalBytes)
    Q_PROPERTY(quint64 usedBytes READ usedBytes)
    Q_PROPERTY(quint64 freeBytes READ freeBytes)
    Q_PROPERTY(QUrl rootUri READ rootPath)
    Q_PROPERTY(QUrl defaultUri READ defaultPath)
    Q_PROPERTY(QString rootPath READ rootPath)
    Q_PROPERTY(QString defaultPath READ defaultPath)
    Q_PROPERTY(QString name READ name)

private:
    explicit DFMVfsDevice(const QUrl &url, void* gmountObjectPtr, QObject *parent = nullptr);

public:
    static DFMVfsDevice* create(const QUrl& url, QObject *parent = nullptr);
    static DFMVfsDevice* createUnsafe(const QUrl& url, QObject *parent = nullptr);
    ~DFMVfsDevice();

    bool attach(); // 挂载
    bool detachAsync(); // 卸载

    DFMVfsAbstractEventHandler *eventHandler() const;
    void setEventHandler(DFMVfsAbstractEventHandler *handler, QThread *threadOfHandle = nullptr);

    bool isReadOnly() const;
    bool canDetach() const;
    quint64 totalBytes() const;
    quint64 usedBytes() const;
    quint64 freeBytes() const;
    QUrl rootUri() const;
    QUrl defaultUri() const;
    QString rootPath() const;
    QString defaultPath() const;
    QString name() const;
    QStringList iconList() const;
    QStringList symbolicIconList() const;

    /* 各种属性：忘记密码？ */

private:
    QScopedPointer<DFMVfsDevicePrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMVfsDevice)
};

DFM_END_NAMESPACE

#endif // DFMVFSDEVICE_H
