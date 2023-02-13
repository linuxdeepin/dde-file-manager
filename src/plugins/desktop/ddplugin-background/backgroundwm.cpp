// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundwm.h"

DDP_BACKGROUND_USE_NAMESPACE

BackgroundWM::BackgroundWM(QObject *parent)
    : BackgroundService(parent),
      gsettings(new QGSettings("com.deepin.dde.appearance", "", this))
{
    connect(gsettings, &QGSettings::changed, this, &BackgroundWM::onAppearanceValueChanged);
}

BackgroundWM::~BackgroundWM()
{
    if (gsettings) {
        gsettings->deleteLater();
        gsettings = nullptr;
    }
}

void BackgroundWM::onAppearanceValueChanged(const QString& key)
{
    if (QStringLiteral("backgroundUris") == key) {
        qInfo() << "appearance background changed...";
        emit backgroundChanged();
    }
}

QString BackgroundWM::getBackgroundFromWm(const QString &screen)
{
    QString path;
    if (screen.isEmpty())
        return path;

    int retry = 5;
    static const int timeOut = 200;
    int oldTimeOut = wmInter->timeout();
    wmInter->setTimeout(timeOut);

    while (retry--) {
        qInfo() << "Get background by wm GetCurrentWorkspaceBackgroundForMonitor and sc:" << screen;
        QDBusPendingReply<QString> reply = wmInter->GetCurrentWorkspaceBackgroundForMonitor(screen);
        reply.waitForFinished();

        if (reply.error().type() != QDBusError::NoError) {
            qWarning() << "Get background failed by wmDBus and times:" << (5-retry)
                       << reply.error().type() << reply.error().name() << reply.error().message();
        } else {
            path = reply.argumentAt<0>();
            qInfo() << "Get background path succeed:" << path << "screen" << screen << "   times:" << (5 - retry);
            break;
        }
    }
    wmInter->setTimeout(oldTimeOut);

    if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile()))
        qCritical() << "get background fail path :" << path << "screen" << screen;
    else
        qInfo() << "getBackgroundFromWm path :" << path << "screen" << screen;
    return path;
}

QString BackgroundWM::getBackgroundFromConfig(const QString &screen)
{
    QString path;
    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    QFile wmFile(homePath + "/.config/deepinwmrc");
    if (wmFile.open(QIODevice::ReadOnly | QIODevice::Text)) {

        // Find the background path based on the workspace and screen name
        while (!wmFile.atEnd()) {
            QString line = wmFile.readLine();
            int index = line.indexOf("@");
            int indexEQ = line.indexOf("=");
            if (index <= 0 || indexEQ <= index + 1) {
                continue;
            }

            int workspaceIndex = line.left(index).toInt();
            QString screenName = line.mid(index + 1, indexEQ - index - 1);
            if (workspaceIndex != currentWorkspaceIndex || screenName != screen) {
                continue;
            }

            path = line.mid(indexEQ + 1).trimmed();
            break;
        }

        wmFile.close();
    }

    return path;
}

QString BackgroundWM::background(const QString &screen)
{
    QString path;

    if (!screen.isEmpty()) {
        //1.Get the background from wm
        path = getBackgroundFromWm(screen);
        qInfo() << "getBackgroundFromWm GetCurrentWorkspaceBackgroundForMonitor path :" << path << "screen" << screen;

        if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
            // 2.Parse background from config file
            path = getBackgroundFromConfig(screen);
            qInfo() << "getBackgroundFormConfig path :" << path << "screen" << screen;

            if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
                // 3.Use the default background
                path = getDefaultBackground();
                qInfo() << "getDefaultBackground path :" << path << "screen" << screen;
            }
        }
    } else {
        qInfo() << "Get background path terminated screen:" << screen << wmInter;
    }

    return path;
}

QString BackgroundWM::getDefaultBackground()
{
    QString defaultPath;
    if (gsettings) {
        for (const QString &path : gsettings->get("background-uris").toStringList()) {
            if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
                continue;
            } else {
                defaultPath = path;
                qInfo() << "default background path:" << path;
                break;
            }
        }
    }

    if (defaultPath.isEmpty()) {
        defaultPath = BackgroundService::getDefaultBackground();
    }

    return defaultPath;
}
