// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
