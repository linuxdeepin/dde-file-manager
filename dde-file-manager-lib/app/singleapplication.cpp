#include "singleapplication.h"
#include "global.h"
#include "filesignalmanager.h"
#include "durl.h"

#include "widgets/commandlinemanager.h"
#include "widgets/singleton.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>

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
                bool isNewWindow = false;
                if (CommandLineManager::instance()->positionalArguments().count() > 0){
                    commandlineUrl = CommandLineManager::instance()->positionalArguments().at(0);
                }else{
                    isNewWindow = CommandLineManager::instance()->isSet("n");
                }
                QJsonObject message;
                message.insert("url", commandlineUrl);
                message.insert("isNewWindow", isNewWindow);
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
    QString userKey;
    if (getUserID() == "0"){
        userKey = QString("%1/%2").arg("/tmp", key);
    }else{
        userKey = QString("%1/%2/%3").arg("/var/run/user", getUserID(), key);
    }
    qDebug() << userKey;
    return userKey;
}

QString SingleApplication::userId()
{
    return UserID;
}

QString SingleApplication::getUserID()
{
    QProcess userID;
    userID.start("id", QStringList() << "-u");
    userID.waitForFinished();
    QByteArray id = userID.readAll();
    UserID = QString(id).trimmed();
    return UserID;
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
    bool isNewWindow = false;
    if (messageObj.contains("url")){
        QString _url = messageObj.value("url").toString();
        if (!_url.isEmpty()){
            url = DUrl::fromUserInput(_url);
        }
    }
    if (messageObj.contains("isNewWindow")){
        if (messageObj.value("isNewWindow").toBool()){
            isNewWindow = true;
        }
    }
    qDebug() << url << isNewWindow;
    emit fileSignalManager->requestOpenNewWindowByUrl(url, isNewWindow);
}

void SingleApplication::closeServer()
{
    if (m_localServer){
        m_localServer->removeServer(m_localServer->serverName());
        m_localServer->close();
    }
}
