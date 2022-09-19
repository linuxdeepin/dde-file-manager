// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "committhread.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>

CommitLog::CommitLog(QObject *parent)
    : QObject(parent)
{
}

CommitLog::~CommitLog()
{
    if (m_library.isLoaded())
        m_library.unload();
    qInfo() << " - destroyed";
}

void CommitLog::commit(const QVariant &args)
{
    if (args.isNull() || !args.isValid())
        return;
    const QJsonObject &dataObj = QJsonObject::fromVariantHash(args.toHash());
    QJsonDocument doc(dataObj);
    const QByteArray &sendData = doc.toJson(QJsonDocument::Compact);
    m_writeEventLog(sendData.data());
}

bool CommitLog::init()
{
    m_library.setFileName("deepin-event-log");
    if (!m_library.load()) {
        qWarning() << "Load library failed";
        return false;
    }

    m_initEventLog = reinterpret_cast<InitEventLog>(m_library.resolve("Initialize"));
    m_writeEventLog = reinterpret_cast<WriteEventLog>(m_library.resolve("WriteEventLog"));

    if (!m_initEventLog || !m_writeEventLog) {
        qWarning() << "Library init failed";
        return false;
    }

    if (!m_initEventLog(QApplication::applicationName().toStdString(), false)) {
        qWarning() << "Initialize called failed";
        return false;
    }

    return true;
}
