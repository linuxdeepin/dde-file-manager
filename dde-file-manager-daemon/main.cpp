#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include "fileoperation.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QDBusConnection connection = QDBusConnection::systemBus();
    QString service = "com.deepin.filemanager.daemon";
    QString objectPath = "/com/deepin/filemanager/daemon/Operations";
    if (!connection.interface()->isServiceRegistered(service)){
        qDebug() << connection.registerService(service) << "register" << service << "success";
        FileOperation fileOperation(service, objectPath);
        return a.exec();
    }else{
        qDebug() << "dde-file-manager-daemon is runing";
//        RenameJobInterface renameJobInterface("com.deepin.filemanager.daemon", "/com/deepin/filemanager/daemon/RenameJob", connection);
//        renameJobInterface.Execute();
        return 0;
    }
}
