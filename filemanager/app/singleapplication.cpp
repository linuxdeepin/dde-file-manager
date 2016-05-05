#include "singleapplication.h"
#include "widgets/commandlinemanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "global.h"
#include "filesignalmanager.h"



SingleApplication::SingleApplication(int &argc, char **argv, int): QApplication(argc, argv)
{
    m_localServer = new QLocalServer;
    initConnect();
}

SingleApplication::~SingleApplication()
{
    if (m_localServer){
        m_localServer->removeServer(m_localServer->serverName());
        m_localServer->close();
    }
}

void SingleApplication::initConnect()
{
    connect(m_localServer, &QLocalServer::newConnection, this, &SingleApplication::handleConnection);
}

void SingleApplication::newClientProcess(const QString &key)
{
    qDebug() << "The dde-file-manager is running!";
    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(userServerName(key));
    if (localSocket->waitForConnected(1000)){
        if (localSocket->state() == QLocalSocket::ConnectedState){
            if (localSocket->isValid()){
                qDebug() << "start write";
                QString commandlineUrl;
                if (CommandLineManager::instance()->positionalArguments().count() > 0){
                    commandlineUrl = CommandLineManager::instance()->positionalArguments().at(0);
                }
                QJsonObject message;
                message.insert("url", commandlineUrl);
                QJsonDocument  obj(message);
                localSocket->write(obj.toJson().data());
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
    QProcess userID;
    userID.start("id", QStringList() << "-u");
    userID.waitForFinished();
    QByteArray id = userID.readAll();

    QString userKey = QString("%1/%2/%3").arg("/var/run/user", QString(id).trimmed(), key);
    return userKey;
}



bool SingleApplication::setSingleInstance(const QString &key)
{
    QString userKey = userServerName(key);

    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(userKey);

    // if connect success, another instance is running.
    bool result = localSocket->waitForConnected(1000);
    localSocket->deleteLater();

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
    qDebug() << sender();
    qDebug() << static_cast<QLocalSocket*>(sender())->bytesAvailable();

    QJsonParseError* error = new QJsonParseError();
    QJsonObject messageObj = QJsonDocument::fromJson(QByteArray(static_cast<QLocalSocket*>(sender())->readAll()), error).object();
    qDebug() << messageObj << error->errorString();

    DUrl url = DUrl::fromLocalFile(QDir::homePath());
    if (messageObj.contains("url")){
        QString _url = messageObj.value("url").toString();
        if (!_url.isEmpty()){
            url = DUrl::fromLocalFile(_url);
        }
    }
    qDebug() << url;
    emit fileSignalManager->requestOpenNewWindowByUrl(url, false);
}
