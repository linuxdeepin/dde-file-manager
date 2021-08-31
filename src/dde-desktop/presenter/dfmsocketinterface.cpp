/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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
    ~DFMSocketInterfacePrivate() {
    }
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
        //连接出错可能造成死循环，屏蔽之
        d->socket->close();
        d->socket->deleteLater();
//        d->socket = new QLocalSocket();
//        d->socket->connectToServer(socketPath);
        qCritical() << "Connect error" << socketError << d->socket->errorString();
    });

    d->socket->connectToServer(socketPath);
}

DFMSocketInterface::~DFMSocketInterface()
{
    Q_D(DFMSocketInterface);
    if (d->socket) {
        d->socket->close();
        d->socket->deleteLater();
        d->socket = nullptr;
    }
}

//json="{\"paths\":[$paths],\"isShowPropertyDialogRequest\":true}";
// echo $json |socat - $XDG_RUNTIME_DIR/dde-file-manager;
void DFMSocketInterface::showProperty(const QStringList &paths)
{
    //Q_D(DFMSocketInterface);

    DDesktopServices::showFileItemProperties(paths);
}
