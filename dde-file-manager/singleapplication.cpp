#include "singleapplication.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "durl.h"

#include "widgets/commandlinemanager.h"
#include "widgets/singleton.h"
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

void SingleApplication::newClientProcess(const QString &key, const DUrlList& urllist)
{
    qDebug() << "The dde-file-manager is running!";
    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(userServerName(key));
    if (localSocket->waitForConnected(1000)){
        if (localSocket->state() == QLocalSocket::ConnectedState){
            if (localSocket->isValid()){
                qDebug() << "start write";
                bool isNewWindow = false;
                bool isShowPropertyDialogRequest = false;

                //Prehandle for none url arguments command on requesting opening new window
                QStringList paths;

                foreach (DUrl url, urllist) {
                    paths << url.toString();
                }

                if (paths.size() > 0){
                    isShowPropertyDialogRequest = CommandLineManager::instance()->isSet("p");
                }else{
                    paths << QDir::homePath();
                }

                isNewWindow = CommandLineManager::instance()->isSet("n");

                QJsonArray jsPaths;
                foreach (QString path, paths) {
                    jsPaths.append(QJsonValue(path));
                }

                QJsonObject message;
                message.insert("isNewWindow", isNewWindow);
                message.insert("isShowPropertyDialogRequest",isShowPropertyDialogRequest);
                message.insert("paths",jsPaths);

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
        userKey = QString("%1/%2").arg(getenv("XDG_RUNTIME_DIR"), key);
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

    QJsonParseError error;
    QJsonObject messageObj = QJsonDocument::fromJson(socket->readAll(), &error).object();
    qDebug() << messageObj << error.errorString();

    DUrl url = DUrl::fromLocalFile(QDir::homePath());

    bool isShowPropertyDialogRequest = false;
    if(messageObj.contains("isShowPropertyDialogRequest"))
        isShowPropertyDialogRequest = messageObj.value("isShowPropertyDialogRequest").toBool();

    QJsonArray jsPaths;
    if(messageObj.contains("paths"))
        jsPaths = messageObj.value("paths").toArray();
    QStringList paths;
    foreach (QJsonValue val, jsPaths) {
        paths << val.toString();
    }

    if(isShowPropertyDialogRequest){
        fileManagerApp->showPropertyDialog(paths);
        return;
    }

    bool isNewWindow = false;

    if (messageObj.contains("isNewWindow")){
        if (messageObj.value("isNewWindow").toBool()){
            isNewWindow = true;
        }
    }

    DFMEvent event;
    DUrlList urlList;
    foreach (QString path, paths) {
        if (!path.isEmpty()){
            url = DUrl::fromUserInput(path);
            urlList << url;
        }
    }
    event << urlList;
    event << urlList.first();

    qDebug() << event << isNewWindow;
    fileService->openUrl(event, isNewWindow);
}

void SingleApplication::closeServer()
{
    if (m_localServer){
        m_localServer->removeServer(m_localServer->serverName());
        m_localServer->close();
    }
}

