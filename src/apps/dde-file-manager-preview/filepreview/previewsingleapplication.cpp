// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewsingleapplication.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/finallyutil.h>

#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <linux/limits.h>

DFMBASE_USE_NAMESPACE
// DFM_LOG_REISGER_CATEGORY(DFMPREVIEW_LOG_CG)

PreviewSingleApplication::PreviewSingleApplication(int &argc, char **argv, int)
    : DApplication(argc, argv), localServer(new QLocalServer)
{
    initConnect();
}

PreviewSingleApplication::~PreviewSingleApplication()
{
    closeServer();
}

void PreviewSingleApplication::handleNewClient(const QString &uniqueKey)
{
    QByteArray data { nullptr };

    for (const QString &arg : arguments()) {
        data.append(arg.toLocal8Bit().toBase64());
        data.append(' ');
    }

    if (!data.isEmpty())
        data.chop(1);

    QLocalSocket *socket = getNewClientConnect(uniqueKey, data);
    if (socket) {
        socket->close();
        socket->deleteLater();
    }
}

void PreviewSingleApplication::processArgs(const QStringList &list)
{
    // parse args
    if (list.count() < 2)
        return;

    // Read data from temporary file
    QFile file(list[1]);
    if (!file.open(QIODevice::ReadOnly)) {
        fmWarning() << "Failed to open temporary file for preview data";
        return;
    }

    // Parse JSON data
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject()) {
        fmWarning() << "Invalid JSON data in temporary file";
        return;
    }

    QJsonObject data = doc.object();
    quint64 winId = data["windowId"].toString().toULongLong();

    // 正确处理JSON数组转换为QUrl列表
    const QJsonArray &selectUrlsArray = data["selectUrls"].toArray();
    QStringList selectUrlStrs;
    for (const QJsonValue &value : selectUrlsArray) {
        selectUrlStrs << value.toString();
    }
    QList<QUrl> selectUrls = QUrl::fromStringList(selectUrlStrs);

    const QJsonArray &dirUrlsArray = data["dirUrls"].toArray();
    QStringList dirUrlStrs;
    for (const QJsonValue &value : dirUrlsArray) {
        dirUrlStrs << value.toString();
    }
    QList<QUrl> dirUrls = QUrl::fromStringList(dirUrlStrs);

    // Delete temporary file
    QFile::remove(list[1]);

    static PreviewLibrary lib;
    if (!lib.load())
        return;

    lib.showPreview(winId, selectUrls, dirUrls);
}

void PreviewSingleApplication::initConnect()
{
    connect(localServer, &QLocalServer::newConnection, this, &PreviewSingleApplication::handleConnection);
}

QLocalSocket *PreviewSingleApplication::getNewClientConnect(const QString &key, const QByteArray &message)
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
        fmDebug() << localSocket->errorString();
    }

    return localSocket;
}

QString PreviewSingleApplication::userServerName(const QString &key)
{
    QString userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation), key);
    if (userKey.isEmpty()) {
        userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation), key);
    }
    return userKey;
}

bool PreviewSingleApplication::setSingleInstance(const QString &key)
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

void PreviewSingleApplication::handleConnection()
{
    fmDebug() << "New connection is coming";
    QLocalSocket *nextPendingConnection = localServer->nextPendingConnection();
    connect(nextPendingConnection, SIGNAL(readyRead()), this, SLOT(readData()));
}

void PreviewSingleApplication::closeServer()
{
    if (localServer) {
        localServer->removeServer(localServer->serverName());
        localServer->close();
        delete localServer;
        localServer = nullptr;
    }
}

void PreviewSingleApplication::readData()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());

    if (!socket)
        return;

    QStringList arguments;
    for (const QByteArray &arg_base64 : socket->readAll().split(' ')) {
        const QByteArray &arg = QByteArray::fromBase64(arg_base64.simplified());

        if (arg.isEmpty())
            continue;

        QString argstr = QString::fromLocal8Bit(arg);
        arguments << argstr;
    }

    processArgs(arguments);
}
