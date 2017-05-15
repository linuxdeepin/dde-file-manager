#include "singleapplication.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "durl.h"

#include "commandlinemanager.h"
#include "singleton.h"
#include "filemanagerapp.h"
#include "dfmevent.h"
#include "interfaces/dfileservices.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonArray>
#include <QJsonValue>

#define fileManagerApp FileManagerApp::instance()

QString SingleApplication::UserID = "1000";

SingleApplication::SingleApplication(int &argc, char **argv, int): DApplication(argc, argv)
{
    m_localServer = new QLocalServer;
    initConnect();
}

SingleApplication::~SingleApplication()
{
    closeServer();
}

void SingleApplication::initConnect()
{
    connect(m_localServer, &QLocalServer::newConnection, this, &SingleApplication::handleConnection);
}

void SingleApplication::initSources()
{
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(dui_theme_dark);
    Q_INIT_RESOURCE(dui_theme_light);
}

void SingleApplication::newClientProcess(const QString &key, const QByteArray &message)
{
    qDebug() << "The dde-file-manager is running!";
    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(userServerName(key));
    if (localSocket->waitForConnected(1000)){
        if (localSocket->state() == QLocalSocket::ConnectedState){
            if (localSocket->isValid()) {
                localSocket->write(message);
                localSocket->flush();
            }
        }
    }else{
        qDebug() << localSocket->errorString();
    }
    qDebug() << "The dde-file-manager is running end!";
}

QString SingleApplication::userServerName(const QString &key)
{
    QString userKey;
    if (getUserID() == "0"){
        userKey = QString("%1/%2").arg("/tmp", key);
    }else{
        userKey = QString("%1/%2").arg(getenv("XDG_RUNTIME_DIR"), key);
    }

    return userKey;
}

QString SingleApplication::userId()
{
    return UserID;
}

QString SingleApplication::getUserID()
{
    return QString::number(DFMGlobal::getUserId());
}

bool SingleApplication::setSingleInstance(const QString &key)
{
    QString userKey = userServerName(key);

    QLocalSocket localSocket;
    localSocket.connectToServer(userKey);

    // if connect success, another instance is running.
    bool result = localSocket.waitForConnected(1000);

    if (result)
        return false;

    m_localServer->removeServer(userKey);

    bool f = m_localServer->listen(userKey);

    return f;
}

void SingleApplication::handleConnection()
{
    qDebug() << "new connection is coming";
    QLocalSocket* nextPendingConnection = m_localServer->nextPendingConnection();
    connect(nextPendingConnection, SIGNAL(readyRead()), this, SLOT(readData()));
}

void SingleApplication::readData()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());

    if (!socket)
        return;

    QStringList arguments;

    for (const QByteArray &arg_base64 : socket->read(1024).split(' ')) {
        const QByteArray &arg = QByteArray::fromBase64(arg_base64.simplified());

        if (arg.isEmpty())
            continue;

        arguments << QString::fromLocal8Bit(arg);
    }

    CommandLineManager::instance()->process(arguments);
    CommandLineManager::instance()->processCommand();
}

void SingleApplication::closeServer()
{
    if (m_localServer){
        m_localServer->removeServer(m_localServer->serverName());
        m_localServer->close();
    }
}

void SingleApplication::handleQuitAction()
{
    fileSignalManager->aboutToCloseLastActivedWindow(0);
}

