// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "committhread.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>

using namespace dfmplugin_utils;

CommitLog::CommitLog(QObject *parent)
    : QObject(parent)
{
}

CommitLog::~CommitLog()
{
    if (logLibrary.isLoaded())
        logLibrary.unload();
    qInfo() << " - destroyed";
}

void CommitLog::commit(const QVariant &args)
{
    if (args.isNull() || !args.isValid())
        return;
    const QJsonObject &dataObj = QJsonObject::fromVariantHash(args.toHash());
    QJsonDocument doc(dataObj);
    const QByteArray &sendData = doc.toJson(QJsonDocument::Compact);
    writeEventLogFunc(sendData.data());
}

bool CommitLog::init()
{
    logLibrary.setFileName("deepin-event-log");
    if (!logLibrary.load()) {
        qWarning() << "Load library failed";
        return false;
    } else {
        qInfo() << "Load library success";
    }

    initEventLogFunc = reinterpret_cast<InitEventLog>(logLibrary.resolve("Initialize"));
    writeEventLogFunc = reinterpret_cast<WriteEventLog>(logLibrary.resolve("WriteEventLog"));

    if (!initEventLogFunc || !writeEventLogFunc) {
        qWarning() << "Library init failed";
        return false;
    }

    if (!initEventLogFunc(QApplication::applicationName().toStdString(), false)) {
        qWarning() << "Initialize called failed";
        return false;
    }

    return true;
}
