#include "appbus_p.h"

#include "log/frameworklog.h"

#include <QDataStream>
#include <QFileInfo>
#include <QtConcurrent>
#include <filesystem>

DPF_USE_NAMESPACE

AppBusPrivate::AppBusPrivate(AppBus *dd)
    : QObject(dd),
      q_ptr(dd),
      tryPingString("static void AppBusPrivate::tryPing(const QString &serverName)")
{
    //隐藏tmp目录下的套接字文件
    appServerName = "." + QCoreApplication::applicationName() + QString("-%0.%1")
            .arg(QCoreApplication::applicationPid())
            .arg(metaObject()->className());

    //当前master加入
    onlineServers[appServerName] = nullptr;

    for(auto val : scanfUseBusApp())
    {
        auto localSocket = new QLocalSocket;
        localSocket->setServerName(val);
        onlineServers[val] = localSocket;

        localSocket->connectToServer(QIODevice::ReadWrite); //双向通信
        QByteArray writeArray;
        QDataStream oStream(&writeArray, QIODevice::WriteOnly);
        QList<QString> onlineServerKeys = onlineServers.keys();
        oStream << onlineServerKeys;
        localSocket->write(writeArray,writeArray.size());
        localSocket->waitForBytesWritten(10);
    }

    server.setMaxPendingConnections(INT_MAX);
    //允许跨用户组交换数据
    server.setSocketOptions(QLocalServer::WorldAccessOption);

    server.listen(appServerName);

    QObject::connect(&server, &QLocalServer::newConnection,
                     this, &AppBusPrivate::procNewConnection,
                     Qt::ConnectionType::DirectConnection);

    qCCritical(FrameworkLog()) << "now online servers: "
                               << onlineServers.keys();

    if (!server.isListening())
        qCCritical(FrameworkLog) << server.errorString();
    else
        qCCritical(FrameworkLog) << server.serverName()
                                 << "Master listening..." ;
}

void AppBusPrivate::procNewConnection()
{
    qCCritical(FrameworkLog) << "new connect socket";
    QLocalSocket *newSocket = server.nextPendingConnection();
    newSocket->waitForReadyRead();
    QByteArray array = newSocket->readAll();
    if (isTryPing(array)) {
        newSocket->write(tryPingString.toLocal8Bit());
        newSocket->waitForBytesWritten();
        newSocket->close();
        return;
    }

    if (isKeepAlive(array)) {

        QByteArray tmpArray = array;
        QStringList onlines;
        QDataStream stream(&tmpArray,QIODevice::ReadOnly);
        stream >> onlines;

        for (auto val : onlines) {
            //接收到的路由表里面等于当前服务
            if (val == server.serverName())
                continue;

            //新增项直接保存
            if (onlineServers[val] == nullptr) {
                onlineServers[val] = newSocket;
                qCCritical(FrameworkLog) << "insert new server"
                                         << val << onlineServers[val];
                q_ptr->newCreateAppBus(val);
            }
        }
        qCDebug(FrameworkLog) << "now onlineServers: "
                                 << onlineServers;

        qCCritical(FrameworkLog) << "now online servers:"
                                 << onlineServers.keys();

        qCCritical(FrameworkLog) << "now online servers count: "
                                 << onlineServers.count();
        return;
    }


    newSocket->close();
}

QStringList AppBusPrivate::scanfUseBusApp()
{
    QString tmp = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];
    //异步迭代
    auto controller = QtConcurrent::run([=]()
    {
        QStringList fileNames;
        std::filesystem::directory_iterator itera(tmp.toStdString());
        for(auto &emle: itera)
        {
            QString fileName = QString(emle.path().filename().c_str());

            if (QFileInfo(tmp + "/" + fileName).suffix()
                    != metaObject()->className()) {
                continue;
            }

            if (fileName == appServerName)
            {
                qCCritical(FrameworkLog) << "mime: " << fileName;
                continue;
            }

            if (!tryPing(fileName)) {
                continue;
            }

            fileNames << fileName;
        }
        return fileNames;
    });
    controller.waitForFinished();
    return controller.result(); //数据同步
}

bool AppBusPrivate::tryPing(const QString &serverName)
{
    qCCritical(FrameworkLog) << "try ping: " << serverName;
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.isOpen()) {
        socket.write(tryPingString.toLocal8Bit());
        socket.waitForBytesWritten(); //等待数据发送完毕
        if (socket.waitForReadyRead(10)) { //等待数据接收
            QByteArray readArray = socket.readAll();
            qCCritical(FrameworkLog) << readArray;
            if (tryPingString == readArray) { //数据认证成功
                return true;
            }
        }
    }

    QString tmp = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];
    QFile::remove(tmp + "/" + serverName);
    return false;
}

bool AppBusPrivate::isTryPing(const QByteArray &array)
{
    return tryPingString == array;
}

bool AppBusPrivate::isKeepAlive(const QByteArray &array)
{
    QByteArray tmpArray = array;
    QList<QString> onlines;
    QDataStream stream(&tmpArray, QIODevice::ReadOnly);
    stream >> onlines;
    return onlines.contains(server.serverName());
}

