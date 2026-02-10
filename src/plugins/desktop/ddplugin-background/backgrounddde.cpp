// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgrounddde.h"

#include <DConfig>

#include <QJsonDocument>

DDP_BACKGROUND_USE_NAMESPACE
DCORE_USE_NAMESPACE

BackgroundDDE::BackgroundDDE(QObject *parent)
    : BackgroundService(parent)
{
    fmDebug() << "create org.deepin.dde.Appearance1";
    interface = new InterFace("org.deepin.dde.Appearance1", "/org/deepin/dde/Appearance1",
                              QDBusConnection::sessionBus(), this);
    interface->setTimeout(200);
    fmDebug() << "create org.deepin.dde.Appearance1 end";

    apperanceConf = DConfig::create("org.deepin.dde.appearance", "org.deepin.dde.appearance", "", this);
    connect(apperanceConf, &DConfig::valueChanged, this, &BackgroundDDE::onAppearanceValueChanged);
}

BackgroundDDE::~BackgroundDDE()
{
    if (interface) {
        interface->deleteLater();
        interface = nullptr;
    }
}

QString BackgroundDDE::getBackgroundFromDDE(const QString &screen)
{
    QString path;
    if (screen.isEmpty())
        return path;

    fmDebug() << "Get background by DDE GetCurrentWorkspaceBackgroundForMonitor and sc:" << screen;
    QDBusPendingReply<QString> reply = interface->GetCurrentWorkspaceBackgroundForMonitor(screen);
    reply.waitForFinished();

    if (reply.error().type() != QDBusError::NoError) {
        fmWarning() << "Get background failed by DDE_DBus"
                    << reply.error().type() << reply.error().name() << reply.error().message();
    } else {
        path = reply.argumentAt<0>();
    }

    return path;
}

QString BackgroundDDE::getBackgroundFromConfig(const QString &screen)
{
    QString path;
    QString configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    QFile ddeFile(configPath + "/dde-appearance/config.json");
    if (!ddeFile.open(QFile::ReadOnly | QIODevice::Text)) {
        fmWarning() << "config file doesn't exist";
        return path;
    }

    //Judge whether the configuration file is valid
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(ddeFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        fmCritical() << "config file is invailid :" << error.errorString();
        return path;
    }

    //Find the background path based on the workspace and screen name
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (int i = 0; i < arr.size(); i++) {
            if (arr.at(i).isObject()) {
                QJsonValue type = arr.at(i).toObject().value("type");
                QJsonValue info = arr.at(i).toObject().value("wallpaperInfo");
                if (type.toString() == ("index+monitorName") && info.isArray()) {
                    QJsonArray val = info.toArray();
                    for (int j = 0; j < val.size(); j++) {
                        if (val.at(j).isObject()) {
                            QString wpIndex = val.at(j).toObject().value("wpIndex").toString();
                            int index = wpIndex.indexOf("+");
                            if (index <= 0) {
                                continue;
                            }

                            int workspaceIndex = wpIndex.left(index).toInt();
                            QString screenName = wpIndex.mid(index + 1);
                            if (workspaceIndex != currentWorkspaceIndex || screenName != screen) {
                                continue;
                            }

                            path = val.at(j).toObject().value("uri").toString();
                            break;
                        }
                    }
                }
            }
        }
    }
    ddeFile.close();

    return path;
}

void BackgroundDDE::onAppearanceValueChanged(const QString &key)
{
    if (key == QString("Wallpaper_Uris")) {
        fmDebug() << "appearance Wallpaper_Uris changed...";
        emit backgroundChanged();
    }
}

QString BackgroundDDE::background(const QString &screen)
{
    QString path;

    if (!screen.isEmpty()) {

        //1.Get the background from DDE
        path = getBackgroundFromDDE(screen);
        fmDebug() << "getBackgroundFromDDE path :" << path << "screen" << screen;

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
        fmDebug() << "Get background path terminated screen:" << screen << interface;
    }

    return path;
}

QString BackgroundDDE::getDefaultBackground()
{
    return BackgroundService::getDefaultBackground();
}
