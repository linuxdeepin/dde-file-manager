/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "checknetwork.h"
#include <QApplication>
#include <QNetworkReply>

CheckNetwork::CheckNetwork(QObject *parent) : QObject(parent)
{

}

bool CheckNetwork::isHostAndPortConnect(const QString &host, const QString &port)
{
    //判断ip和port是否在查询，在就等待
    QNetworkAccessManager manager;
    QEventLoop loop;
    QMutex mutex;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(2000);
    bool bvist = true;
    bool bready = false;

    connect(&manager, &QNetworkAccessManager::finished, this, [&](QNetworkReply * reply) {
        QMutexLocker lk(&mutex);
        bready = true;
        qDebug() << reply->error() << reply->errorString();
        bvist = QNetworkReply::UnknownNetworkError < reply->error() ||  QNetworkReply::NoError >= reply->error();
        loop.exit();
        timer.stop();
    });

    connect(&manager, &QNetworkAccessManager::sslErrors, this, [&](QNetworkReply * reply,const QList<QSslError> &errors) {
        QMutexLocker lk(&mutex);
        bready = true;
        qDebug() << reply->error() << reply->errorString() << errors;
        bvist = QNetworkReply::UnknownNetworkError < reply->error() ||  QNetworkReply::NoError >= reply->error();
        loop.exit();
        timer.stop();
    });

    manager.connectToHost(host,port.toUShort() == 0 ? 21 : port.toUShort());

    connect(&timer,&QTimer::timeout,this,[&](){
        QMutexLocker lk(&mutex);
        bvist = bready ? bvist : false;
        loop.exit();
        timer.stop();
    });

    timer.start();
    loop.exec();

    QMutexLocker lk(&mutex);

    return bvist;
}
