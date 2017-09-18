/*
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
    DTK_CORE_NAMESPACE::DLogManager::registerConsoleAppender();
    DTK_CORE_NAMESPACE::DLogManager::registerFileAppender();
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
