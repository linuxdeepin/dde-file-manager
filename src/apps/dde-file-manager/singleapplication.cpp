// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleapplication.h"
#include "commandparser.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/finallyutil.h>

#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QProcess>

#include <linux/limits.h>

Q_DECLARE_LOGGING_CATEGORY(logAppFileManager)

DFMBASE_USE_NAMESPACE

SingleApplication::SingleApplication(int &argc, char **argv, int)
    : DApplication(argc, argv), localServer(new QLocalServer)
{
    initConnect();
}

SingleApplication::~SingleApplication()
{
    closeServer();
}

void SingleApplication::initConnect()
{
    connect(localServer, &QLocalServer::newConnection, this, &SingleApplication::handleConnection);
    connect(this, &QApplication::aboutToQuit, this, &SingleApplication::closeServer);
}

QLocalSocket *SingleApplication::getNewClientConnect(const QString &key, const QByteArray &message)
{
    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(userServerName(key));
    if (localSocket->waitForConnected(1000)) {
        if (localSocket->state() == QLocalSocket::ConnectedState) {
            if (localSocket->isValid()) {
                localSocket->write(message);
                localSocket->flush();
            }
        }
    } else {
        qCWarning(logAppFileManager) << "SingleApplication::getNewClientConnect: Failed to connect to server:" << localSocket->errorString();
    }

    return localSocket;
}

QString SingleApplication::userServerName(const QString &key)
{
    QString userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation), key);
    if (userKey.isEmpty()) {
        userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation), key);
    }
    return userKey;
}

void SingleApplication::openAsAdmin()
{
    if (WindowUtils ::isWayLand()) {
        QString cmd = "xhost";
        QStringList args;
        args << "+";
        QProcess p;
        p.start(cmd, args);
        p.waitForFinished();
    }

    QStringList args = arguments().mid(1);
    args.removeAll(QStringLiteral("-r"));
    args.removeAll(QStringLiteral("--root"));
    args.removeAll(QStringLiteral("-w"));
    args.removeAll(QStringLiteral("--working-dir"));
    QProcess::startDetached("dde-file-manager-pkexec", args, QDir::currentPath());
}

void SingleApplication::handleNewClient(const QString &uniqueKey)
{
    QByteArray data { nullptr };
    bool isSetGetMonitorFiles = false;

    for (const QString &arg : arguments()) {
        if (arg == "--get-monitor-files")
            isSetGetMonitorFiles = true;

        if (!arg.startsWith("-") && QFile::exists(arg))
            data.append(QDir(arg).absolutePath().toLocal8Bit().toBase64());
        else
            data.append(arg.toLocal8Bit().toBase64());

        data.append(' ');
    }

    if (!data.isEmpty())
        data.chop(1);

    QLocalSocket *socket = SingleApplication::getNewClientConnect(uniqueKey, data);
    if (socket) {
        socket->waitForReadyRead();

        for (const QByteArray &i : socket->readAll().split(' '))
            qInfo(logAppFileManager) << QString::fromLocal8Bit(QByteArray::fromBase64(i));

        socket->close();
        socket->deleteLater();
    }
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

    localServer->removeServer(userKey);

    bool f = localServer->listen(userKey);

    return f;
}

void SingleApplication::handleConnection()
{
    qCDebug(logAppFileManager) << "SingleApplication::handleConnection: New client connection received";
    QLocalSocket *nextPendingConnection = localServer->nextPendingConnection();
    connect(nextPendingConnection, SIGNAL(readyRead()), this, SLOT(readData()));
}

void SingleApplication::readData()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());

    if (!socket) {
        qCWarning(logAppFileManager) << "SingleApplication::readData: Invalid socket sender";
        return;
    }

    QStringList arguments;
    for (const QByteArray &arg_base64 : socket->readAll().split(' ')) {
        const QByteArray &arg = QByteArray::fromBase64(arg_base64.simplified());

        if (arg.isEmpty())
            continue;

        QString argstr = QString::fromLocal8Bit(arg);
        arguments << argstr;
    }

    qCDebug(logAppFileManager) << "SingleApplication::readData: Processing" << arguments.size() << "arguments from client";
    CommandParser::instance().process(arguments);

    FinallyUtil release([&] {
        if (socket) {
            socket->close();
            socket->deleteLater();
        }
    });

    if (CommandParser::instance().isSet("get-monitor-files")) {
        qCDebug(logAppFileManager) << "SingleApplication::readData: Processing get-monitor-files request";
        //Todo(yanghao&lxs): get-monitor-files
        return;
    }

    CommandParser::instance().processCommand();
}

void SingleApplication::closeServer()
{
    if (localServer) {
        qCDebug(logAppFileManager) << "SingleApplication::closeServer: Closing local server";
        localServer->removeServer(localServer->serverName());
        localServer->close();
        delete localServer;
        localServer = nullptr;
    }
}

void SingleApplication::handleQuitAction()
{
    // fix bug-272373
    // When exiting the process, if a copy task exists (@abstractjob.cpp),
    // this task will block the main thread, causing the UI to be unresponsive.
    // So here is a rewrite of the exit implementation that closes all the windows
    // of the filemanager
    qCInfo(logAppFileManager) << "SingleApplication::handleQuitAction: Closing all file manager windows";
    WindowUtils::closeAllFileManagerWindows();
}
