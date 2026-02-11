// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
        qCInfo(logAppFileManager) << "UsmSessionAPI::~UsmSessionAPI: Disconnected from session manager";
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
    return false;   // seesion bussion is needless this time

    if (initialized)
        return true;
    libUsm.setFileName("usm");
    if (!libUsm.load()) {
        qCCritical(logAppFileManager) << "UsmSessionAPI::init: Cannot load libusm.so";
        return false;
    }

    connectSM = reinterpret_cast<FnConnectSM>(libUsm.resolve("connectSM"));
    disconnectSM = reinterpret_cast<FnDisconnectSM>(libUsm.resolve("disconnectSM"));
    requestSaveYourself = reinterpret_cast<FnRequestSaveYourself>(libUsm.resolve("requestSaveYourself"));
    parseArguments = reinterpret_cast<FnParseArguments>(libUsm.resolve("parseArguments"));
    setWindowProperty = reinterpret_cast<FnSetWindowProperty>(libUsm.resolve("setWindowProperty"));
    filename = reinterpret_cast<FnFilename>(libUsm.resolve("filename"));

    initialized = (connectSM && disconnectSM && requestSaveYourself && parseArguments && setWindowProperty && filename);

    qCInfo(logAppFileManager) << "UsmSessionAPI::init: Session API initialized:" << initialized;
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

void SessionBusiness::process(const QStringList &args)
{
    arguments = args;
}

char **SessionBusiness::parseArguments(int &argc)
{
    if (arguments.isEmpty())
        return nullptr;
    argc = arguments.count();
    char **argv_new = new char *[argc];
    for (int i = 0; i < argc; ++i) {
        QByteArray ba = arguments.at(i).toLocal8Bit();
        argv_new[i] = new char[ba.size() + 1];
        memset(argv_new[i], 0, ba.size() + 1);
        strncpy(argv_new[i], ba.data(), ba.size());
    }
    return argv_new;
}

void SessionBusiness::releaseArguments(int argc, char **argv_new)
{
    if (argc < 1 || argv_new == nullptr)
        return;

    for (int i = 0; i < argc; ++i) {
        delete[] argv_new[i];
    }
    delete[] argv_new;
}

void SessionBusiness::savePath(quint64 wid, const QString &path)
{
    if (!sessionAPI.isInitialized()) {
        qCWarning(logAppFileManager) << "SessionBusiness::savePath: Failed to save path, session API not initialized, PID:" << qApp->applicationPid();
        return;
    }
    QString filePath = QString("%1/.config/%2").arg(QDir::homePath()).arg(sessionAPI.filename(wid));

    QJsonObject jsonObj;
    jsonObj["path"] = path;
    QJsonDocument doc(jsonObj);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCCritical(logAppFileManager) << "SessionBusiness::savePath: Failed to write session data to file:" << filePath 
                                      << "for window:" << wid << "path:" << path;
        return;
    }

    file.write(doc.toJson());
    file.close();

    qCInfo(logAppFileManager) << "SessionBusiness::savePath: Successfully saved session data for window:" << wid 
                              << "to file:" << filePath;
}

bool SessionBusiness::readPath(const QString &fileName, QString *data)
{
    if (!data) {
        qCCritical(logAppFileManager) << "SessionBusiness::readPath: Output data pointer is null";
        return false;
    }

    QString filePath = QString("%1/.config/session/%2_%3").arg(QDir::homePath()).arg(QCoreApplication::applicationName()).arg(fileName);
    QString &path = *data;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(logAppFileManager) << "SessionBusiness::readPath: Cannot read session data from file:" << filePath;
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        qCWarning(logAppFileManager) << "SessionBusiness::readPath: Invalid JSON data in session file:" << filePath;
        return false;
    }

    QJsonObject jsonObj = doc.object();
    path = jsonObj["path"].toString();
    qCInfo(logAppFileManager) << "SessionBusiness::readPath: Successfully read session data from file:" << filePath 
                              << "path:" << path;

    return !path.isEmpty();
}

void SessionBusiness::bindEvents()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &SessionBusiness::onWindowClosed, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &SessionBusiness::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::currentUrlChanged, this, &SessionBusiness::onCurrentUrlChanged, Qt::DirectConnection);
}

const static QString windowInit = "inited";
void SessionBusiness::connectToUsmSever(quint64 wid)
{
    auto inited = windowStatus.value(wid);
    if (inited.contains(windowInit))
        return;

    if (SessionBusiness::instance()->getAPI()->init()) {
        int argc = 0;
        char **argv = parseArguments(argc);
        getAPI()->parseArguments(argc, argv);
        releaseArguments(argc, argv);

        getAPI()->connectSM(wid);
        getAPI()->setWindowProperty(wid);
        windowStatus[wid] = windowInit;
        qCInfo(logAppFileManager) << "SessionBusiness::connectToUsmSever: Connected to session manager for window:" << wid 
                                  << "with arguments:" << arguments;
    }
}

void SessionBusiness::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "SessionBusiness::onWindowOpened", "Cannot find window by id");

    if (!window->isHidden()) {
        qCDebug(logAppFileManager) << "SessionBusiness::onWindowOpened: Window opened, connecting to session manager, window ID:" << windId;
        connectToUsmSever(window->winId());
    }
}

void SessionBusiness::onWindowClosed(quint64 windId)
{
    qCDebug(logAppFileManager) << "SessionBusiness::onWindowClosed: Window closed, removing from status, window ID:" << windId;
    windowStatus.remove(windId);
}

void SessionBusiness::onCurrentUrlChanged(quint64 windId, const QUrl &url)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "SessionBusiness::onCurrentUrlChanged", "Cannot find window by id");

    if (!window->isHidden()) {
        qCDebug(logAppFileManager) << "SessionBusiness::onCurrentUrlChanged: URL changed for window:" << windId 
                                   << "new URL:" << url.toString();
        connectToUsmSever(window->winId());
        savePath(window->winId(), url.toString());
    }
}
