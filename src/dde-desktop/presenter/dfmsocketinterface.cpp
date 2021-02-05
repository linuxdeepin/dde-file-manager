/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dfmsocketinterface.h"
#include "dfmsocketinterface_p.h"

#include <QDebug>
#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class DFMSocketInterfacePrivate
{
public:
    explicit DFMSocketInterfacePrivate(DFMSocketInterface *parent) : q_ptr(parent) {}

    QLocalSocket *socket = nullptr;

    DFMSocketInterface *q_ptr;
    Q_DECLARE_PUBLIC(DFMSocketInterface)
};

DFMSocketInterface::DFMSocketInterface(QObject *parent) : QObject(parent), d_ptr(new DFMSocketInterfacePrivate(this))
{
    Q_D(DFMSocketInterface);

    d->socket = new QLocalSocket();
    QString socketPath = QString("/var/run/user/%1/dde-file-manager").arg(getuid());
    qInfo() << "connect to socket" << socketPath;
    connect(d->socket, static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
    this, [ = ](QLocalSocket::LocalSocketError socketError) {
        d->socket->close();
        d->socket->deleteLater();
        d->socket = new QLocalSocket();
        d->socket->connectToServer(socketPath);
        qCritical() << "Connect error" << socketError << d->socket->errorString();
    });

    d->socket->connectToServer(socketPath);
}

DFMSocketInterface::~DFMSocketInterface()
{

}

//json="{\"paths\":[$paths],\"isShowPropertyDialogRequest\":true}";
// echo $json |socat - $XDG_RUNTIME_DIR/dde-file-manager;
void DFMSocketInterface::showProperty(const QStringList &paths)
{
    //Q_D(DFMSocketInterface);

    DDesktopServices::showFileItemProperties(paths);
}
