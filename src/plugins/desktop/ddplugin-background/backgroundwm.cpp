// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundwm.h"

#include <QGSettings>

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

void BackgroundWM::onAppearanceValueChanged(const QString &key)
{
    if (QStringLiteral("backgroundUris") == key) {
        fmInfo() << "appearance background changed...";
        emit backgroundChanged();
    }
}

QString BackgroundWM::getBackgroundFromWm(const QString &screen)
{
    QString path;
    if (screen.isEmpty())
        return path;

    if (!isWMActive()) {
        fmWarning() << "wm is not registered on dbus";
        return path;
    }

    fmInfo() << "Get background by wm GetCurrentWorkspaceBackgroundForMonitor and sc:" << screen;
    QDBusPendingReply<QString> reply = wmInter->GetCurrentWorkspaceBackgroundForMonitor(screen);
    reply.waitForFinished();

    if (reply.error().type() != QDBusError::NoError) {
        fmWarning() << "Get background failed by wmDBus"
                    << reply.error().type() << reply.error().name() << reply.error().message();
    } else {
        path = reply.argumentAt<0>();
    }

    return path;
}

QString BackgroundWM::getBackgroundFromConfig(const QString &screen)
{
    QString path;
    QString configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    QFile wmFile(configPath + "/deepinwmrc");
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

bool BackgroundWM::isWMActive() const
{
    if (auto ifs = QDBusConnection::sessionBus().interface()) {
        return ifs->isServiceRegistered("com.deepin.wm");
    }
    return false;
}

QString BackgroundWM::background(const QString &screen)
{
    QString path;

    if (!screen.isEmpty()) {
        // 1.Get the background from wm
        path = getBackgroundFromWm(screen);
        fmInfo() << "getBackgroundFromWm  path :" << path << "screen" << screen;

        if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
            // 2.Parse background from config file
            path = getBackgroundFromConfig(screen);
            fmWarning() << "getBackgroundFormConfig path :" << path << "screen" << screen;

            if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
                // 3.Use the default background
                path = getDefaultBackground();
                fmCritical() << "getDefaultBackground path :" << path << "screen" << screen;
            }
        }
    } else {
        fmInfo() << "Get background path terminated screen:" << screen << wmInter;
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
                fmInfo() << "default background path:" << path;
                break;
            }
        }
    }

    if (defaultPath.isEmpty()) {
        defaultPath = BackgroundService::getDefaultBackground();
    }

    return defaultPath;
}
