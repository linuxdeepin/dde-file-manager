#ifndef CHECKNETWORK_H
#define CHECKNETWORK_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QEventLoop>
#include <QNetworkAccessManager>

#include <durl.h>

class CheckNetwork : public QObject
{
    Q_OBJECT
public:
    explicit CheckNetwork(QObject *parent = nullptr);
    bool isHostAndPortConnect(const QString &host, const QString &port);
    static bool isHostAndPortConnectV2(const QString &host, quint16 port, int timeout = 1200, bool fast = true);
    static void clearUp();
signals:

public slots:

private:
    //save host and port
};

#endif // CHECKNETWORK_H
