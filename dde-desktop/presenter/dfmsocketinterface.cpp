/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dfmsocketinterface.h"

#include <QDebug>
#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "DDesktopServices"

DUTIL_USE_NAMESPACE

using namespace std;

int getProcIdByName(string procName)
{
    int pid = -1;

    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != NULL) {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp))) {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0) {
                // Read contents of virtual /proc/{pid}/cmdline file
                string cmdPath = string("/proc/") + dirp->d_name + "/cmdline";
                ifstream cmdFile(cmdPath.c_str());
                string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty()) {
                    // Keep first cmdline item which contains the program path
                    size_t pos = cmdLine.find('\0');
                    if (pos != string::npos) {
                        cmdLine = cmdLine.substr(0, pos);
                    }
                    // Keep program name only, removing the path
                    pos = cmdLine.rfind('/');
                    if (pos != string::npos) {
                        cmdLine = cmdLine.substr(pos + 1);
                    }
                    // Compare against requested process name
                    if (procName == cmdLine) {
                        pid = id;
                    }
                }
            }
        }
    }

    closedir(dp);

    return pid;
}
extern void startProcessDetached(const QString &program,
                                 const QStringList &arguments = QStringList(),
                                 QIODevice::OpenMode mode = QIODevice::ReadWrite);

class DFMSocketInterfacePrivate
{
public:
    DFMSocketInterfacePrivate(DFMSocketInterface *parent) : q_ptr(parent) {}

    QLocalSocket *socket = nullptr;

    DFMSocketInterface *q_ptr;
    Q_DECLARE_PUBLIC(DFMSocketInterface);
};

DFMSocketInterface::DFMSocketInterface(QObject *parent) : QObject(parent), d_ptr(new DFMSocketInterfacePrivate(this))
{
    Q_D(DFMSocketInterface);

    d->socket = new QLocalSocket();
    QString socketPath = qgetenv("XDG_RUNTIME_DIR") + "/dde-file-manager";
    socketPath = QString("/var/run/user/%1/dde-file-manager").arg(getuid());
    qDebug() << "connect to socket" << socketPath;
    connect(d->socket, static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
    this, [ = ](QLocalSocket::LocalSocketError socketError) {
        d->socket->close();
        d->socket->deleteLater();
        d->socket = new QLocalSocket();
        d->socket->connectToServer(socketPath);
        qCritical() << "Connect error" << socketError << d->socket->errorString();
    });

    d->socket->connectToServer(socketPath);
}

DFMSocketInterface::~DFMSocketInterface()
{

}

//json="{\"paths\":[$paths],\"isShowPropertyDialogRequest\":true}";
// echo $json |socat - $XDG_RUNTIME_DIR/dde-file-manager;
void DFMSocketInterface::showProperty(const QStringList &paths)
{
    Q_D(DFMSocketInterface);

    DDesktopServices::showFileItemProperties(paths);
}
