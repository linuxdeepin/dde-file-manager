#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <DLog>
#include "app/global.h"
#include "app/filemanagerdaemon.h"
#include "client/filemanagerclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QDBusConnection connection = QDBusConnection::systemBus();
    DTK_UTIL_NAMESPACE::DLogManager::registerConsoleAppender();
    DTK_UTIL_NAMESPACE::DLogManager::registerFileAppender();
    if (!connection.interface()->isServiceRegistered(DaemonServicePath)){
        qDebug() << connection.registerService(DaemonServicePath) << "register" << DaemonServicePath << "success";
        FileManagerDaemon* daemon = new FileManagerDaemon();
        qDebug() << daemon;
        return a.exec();
    }else{
        qDebug() << "dde-file-manager-daemon is running";
        FileManagerClient* client = new FileManagerClient();
        qDebug() << client << QCoreApplication::applicationPid();
        return a.exec();
    }
}
