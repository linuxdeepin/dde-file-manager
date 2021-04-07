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
