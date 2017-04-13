#include "singletonapp.h"
#include <QProcess>
#include <QLocalSocket>
#include <QDebug>

SingletonApp::SingletonApp(QObject *parent) : QObject(parent)
{
    initConnections();
}

void SingletonApp::initConnections()
{
    connect(&m_server, &QLocalServer::newConnection, this, &SingletonApp::handleConnection);
}

QString SingletonApp::getServerPathByKey(const QString &key)
{
    QString serverPath = QString("%1/%2").arg(getServerRootPath(), key);
    return serverPath;
}

QString SingletonApp::getUserId()
{
    QString cmd = "id";
    QProcess p;
    p.start(cmd, (QStringList() << "-u"));
    p.waitForFinished(-1);
    QString uid = p.readLine();
    return uid.trimmed();
}

QString SingletonApp::getServerRootPath()
{
    QString path;
    QString uid = getUserId();
    if(uid == "0")
        path = QString("/tmp");
    else
        path = QString("/run/user/%1").arg(uid);
    return path;
}

SingletonApp *SingletonApp::instance()
{
    static SingletonApp* app = new SingletonApp;
    return app;
}

bool SingletonApp::setSingletonApplication(const QString &key)
{
    m_key = key;
    QString serverPath = getServerPathByKey(key);
    QLocalSocket socket;
    socket.connectToServer(serverPath);
    bool ret = socket.waitForConnected(1000);
    if(ret){
        socket.write("MultiProcess");
        socket.flush();
        return false;
    }

    QLocalServer::removeServer(serverPath);
    ret = m_server.listen(serverPath);
    return ret;
}

void SingletonApp::readData()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    if(!socket)
        return;

    QByteArray data = socket->readAll();
    qDebug () << "New connection requested:" << data;
}

void SingletonApp::handleConnection()
{
    QLocalSocket* socket = m_server.nextPendingConnection();
    connect(socket, &QLocalSocket::readyRead, this, &SingletonApp::readData);
}

