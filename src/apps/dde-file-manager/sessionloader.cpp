// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionloader.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

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

void SessionBusiness::savePath(const QString &path)
{
    if (!sessionAPI.isInitialized()) {
        qCWarning(logAppFileManager) << "failed to save path caused no usm session api init";
        return;
    }
    KConfig config(sessionAPI.filename());
    KConfigGroup general = config.group("General");
    general.writeEntry("Path", path);
}

bool SessionBusiness::readPath(QString *path)
{
    if (!sessionAPI.isInitialized()) {
        qCWarning(logAppFileManager) << "failed to read path caused no usm session api init";
        return false;
    }

    if (!path) {
        qCWarning(logAppFileManager) << "path is null";
        return false;
    }

    KConfig config(sessionAPI.filename());
    KConfigGroup general = config.group("General");
    *path = general.readEntry("Path", "");
    qCInfo(logAppFileManager) << "get general path:" << *path;
    return true;
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
        getAPI()->setWindowProperty(window->winId());
    }
}

void SessionBusiness::onCurrentUrlChanged(quint64 windId, const QUrl &url)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "WindowMonitor", "Cannot find window by id");

    if (getAPI()->isInitialized()) {
        getAPI()->setWindowProperty(window->winId());
    }

    savePath(url.path());
}
