// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionloader.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

Q_DECLARE_LOGGING_CATEGORY(logAppFileManager)

using namespace dfmbase;
UsmSessionAPI::UsmSessionAPI()
{
}

UsmSessionAPI::~UsmSessionAPI()
{
    if (disconnectSM) {
        disconnectSM();
        qCInfo(logAppFileManager) << "disconnect SM";
    }

    libUsm.unload();
    initialized = false;
}

bool UsmSessionAPI::isInitialized() const
{
    return initialized;
}

bool UsmSessionAPI::init()
{
    if (initialized)
        return true;
    libUsm.setFileName("usm");
    if (!libUsm.load()) {
        qCWarning(logAppFileManager) << "cannot load libusm.so";
        return false;
    }

    connectSM = reinterpret_cast<FnConnectSM>(libUsm.resolve("connectSM"));
    disconnectSM = reinterpret_cast<FnDisconnectSM>(libUsm.resolve("disconnectSM"));
    requestSaveYourself = reinterpret_cast<FnRequestSaveYourself>(libUsm.resolve("requestSaveYourself"));
    parseArguments = reinterpret_cast<FnParseArguments>(libUsm.resolve("parseArguments"));
    setWindowProperty = reinterpret_cast<FnSetWindowProperty>(libUsm.resolve("setWindowProperty"));
    filename = reinterpret_cast<FnFilename>(libUsm.resolve("filename"));

    initialized = (connectSM && disconnectSM && requestSaveYourself && parseArguments && setWindowProperty && filename);

    qCInfo(logAppFileManager) << "usm session api initialized: " << initialized;
    return initialized;
}

SessionBusiness::SessionBusiness(QObject *parent)
    : QObject(parent)
{
    bindEvents();
}

SessionBusiness::~SessionBusiness()
{
}

SessionBusiness *SessionBusiness::instance()
{
    static SessionBusiness obj;
    return &obj;
}

UsmSessionAPI *SessionBusiness::getAPI()
{
    return &sessionAPI;
}

void SessionBusiness::savePath(unsigned long long wid, const QString &path)
{
    if (!sessionAPI.isInitialized()) {
        qCWarning(logAppFileManager) << "failed to save path caused no usm session api init";
        return;
    }
    QString filePath = QString("%1/.config/%2").arg(QDir::homePath()).arg(sessionAPI.filename(wid));

    QJsonObject jsonObj;
    jsonObj["path"] = path;
    QJsonDocument doc(jsonObj);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(logAppFileManager) << "Failed to write data:" << path << " to file:" << filePath;
        return;
    }

    file.write(doc.toJson());
    file.close();

    qCInfo(logAppFileManager) << "done to write data:" << path << " to file:" << filePath;
}

bool SessionBusiness::readPath(const QString &fileName, QString *data)
{
    if (!sessionAPI.isInitialized()) {
        qCWarning(logAppFileManager) << "failed to read path caused no usm session api init";
        return false;
    }

    if (!data) {
        qCWarning(logAppFileManager) << "path is null";
        return false;
    }

    QString filePath = QString("%1/.config/session/%2_%3").arg(QDir::homePath()).arg(QCoreApplication::applicationName()).arg(fileName);
    QString &path = *data;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(logAppFileManager) << "can't not read data from:" << filePath;
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        qCWarning(logAppFileManager) << "can't get the doc from:" << filePath;
        return false;
    }

    QJsonObject jsonObj = doc.object();
    path = jsonObj["path"].toString();
    qCInfo(logAppFileManager) << "get the data:" << path << " from:" << filePath;

    return !path.isEmpty();
}

void SessionBusiness::bindEvents()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &SessionBusiness::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::currentUrlChanged, this, &SessionBusiness::onCurrentUrlChanged, Qt::DirectConnection);
}

void SessionBusiness::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "WindowMonitor", "Cannot find window by id");

    if (getAPI()->isInitialized()) {
        getAPI()->connectSM(window->winId());
        getAPI()->setWindowProperty(window->winId());
    }
}

void SessionBusiness::onCurrentUrlChanged(quint64 windId, const QUrl &url)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "WindowMonitor", "Cannot find window by id");

    savePath(window->winId(), url.toString());
}
