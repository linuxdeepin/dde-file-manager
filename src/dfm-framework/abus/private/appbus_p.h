#ifndef APPBUS_P_H
#define APPBUS_P_H

#include "dfm-framework/abus/appbus.h"
#include "dfm-framework/service/pluginservice.h"
#include "dfm-framework/definitions/globaldefinitions.h"

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QCoreApplication>

DPF_BEGIN_NAMESPACE

class AppBusPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(AppBus)
    AppBus * const q_ptr;

public:
    explicit AppBusPrivate(AppBus * dd);
    void importService(PluginService* service);
    void exportService(PluginService* service);

private Q_SLOTS:
    void procNewConnection();

private:
    QLocalServer server;

    QString tryPingString;
    QString appServerName;
    QHash<QString, QLocalSocket*> onlineServers;

    QStringList scanfUseBusApp();

    bool tryPing(const QString &serverName);
    bool isTryPing(const QByteArray &array);
    bool isKeepAlive(const QByteArray &array);
    bool isMethodCall(const QByteArray &array);
};

DPF_END_NAMESPACE

#endif // APPBUS_P_H
