/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
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

#include "singleapplication.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "durl.h"
#include "dfmeventdispatcher.h"
#include "dfilewatcher.h"

#include "commandlinemanager.h"
#include "singleton.h"
#include "filemanagerapp.h"
#include "dfmevent.h"
#include "interfaces/dfileservices.h"

#include <QProcess>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QTranslator>

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
    connect(fileSignalManager, &FileSignalManager::requestCloseListen, this, &SingleApplication::closeServer);
}

void SingleApplication::initSources()
{
    Q_INIT_RESOURCE(icons);
//    Q_INIT_RESOURCE(dui_theme_dark);
//    Q_INIT_RESOURCE(dui_theme_light);
}

QLocalSocket *SingleApplication::newClientProcess(const QString &key, const QByteArray &message)
{
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

QString SingleApplication::userId()
{
    return UserID;
}

bool SingleApplication::loadTranslator(QList<QLocale> localeFallback)
{
    DApplication::loadTranslator(localeFallback);

    QStringList translateDirs;

    const QStringList &dataDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    for (QString path : dataDirs) {
        translateDirs << path.append("/").append(applicationName()).append("/translations");
    }

    const QString name = QStringLiteral("dde-file-manager-app");

    for (const QLocale &locale : localeFallback) {
        QString translateFilename = QString("%1_%2").arg(name).arg(locale.name());
        for (QString path : translateDirs) {
            const QString &translatePath = path.append("/").append(translateFilename);
            if (QFile::exists(translatePath + ".qm")) {
                qDebug() << "load translate" << translatePath;
                QTranslator *translator = new QTranslator(this);
                translator->load(translatePath);
                installTranslator(translator);
                return true;
            }
        }

        QStringList parseLocalNameList = locale.name().split("_", QString::SkipEmptyParts);
        if (parseLocalNameList.length() > 0) {
            translateFilename = QString("%1_%2").arg(name)
                    .arg(parseLocalNameList.at(0));
            for (QString path : translateDirs) {
                const QString &translatePath = path.append("/").append(translateFilename);
                if (QFile::exists(translatePath + ".qm")) {
                    qDebug() << "translatePath after feedback:" << translatePath;
                    QTranslator *translator = new QTranslator(this);
                    translator->load(translatePath);
                    installTranslator(translator);
                    return true;
                }
            }
        }
    }

    return false;
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
    //socket->read(1024)中的1024会导致在文件夹名字达到最大长度时获取不完全，临时改为2048
    for (const QByteArray &arg_base64 : socket->read(2048).split(' ')) {
        const QByteArray &arg = QByteArray::fromBase64(arg_base64.simplified());

        if (arg.isEmpty())
            continue;

        arguments << QString::fromLocal8Bit(arg);
    }

    CommandLineManager::instance()->process(arguments);

    if (CommandLineManager::instance()->isSet("get-monitor-files")) {
        const QStringList &list = DFileWatcher::getMonitorFiles();
        QByteArray data;

        for (const QString &i : list)
            data.append(i.toLocal8Bit().toBase64()).append(' ');

        socket->write(data);
        socket->flush();
        return;
    }

    CommandLineManager::instance()->processCommand();
}

void SingleApplication::closeServer()
{
    if (m_localServer){
        m_localServer->removeServer(m_localServer->serverName());
        m_localServer->close();
        delete m_localServer;
        m_localServer = nullptr;
    }
}

void SingleApplication::handleQuitAction()
{
    fileSignalManager->aboutToCloseLastActivedWindow(0);
}

