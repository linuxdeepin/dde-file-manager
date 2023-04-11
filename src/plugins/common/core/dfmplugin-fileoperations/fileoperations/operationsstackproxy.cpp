// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operationsstackproxy.h"

DPFILEOPERATIONS_BEGIN_NAMESPACE

static constexpr uint8_t kMaxStep { 2 };   // BUG: 116699
static const char *kOperationsStackService { "org.deepin.filemanager.server" };
static const char *kOperationsStackPath { "/org/deepin/filemanager/server/OperationsStackManager" };

OperationsStackProxy &OperationsStackProxy::instance()
{
    static OperationsStackProxy ins;
    return ins;
}

void OperationsStackProxy::saveOperations(const QVariantMap &values)
{
    if (dbusValid) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = operationsStackDbus->SaveOperations(values);
        reply.waitForFinished();
        if (!reply.isValid()) {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return;
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return;
    }

    while (fileOperations.size() >= kMaxStep)
        fileOperations.pop_front();

    fileOperations.push(values);
}

void OperationsStackProxy::cleanOperations()
{
    if (dbusValid) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        operationsStackDbus->CleanOperations();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return;
    }

    fileOperations.clear();
}

QVariantMap OperationsStackProxy::revocationOperations()
{
    if (dbusValid) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = operationsStackDbus->RevocationOperations();
        reply.waitForFinished();
        if (!reply.isValid()) {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return {};
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;

        return reply.value();
    }

    if (fileOperations.count() > 0)
        return fileOperations.pop();

    return {};
}

OperationsStackProxy::OperationsStackProxy(QObject *parent)
    : QObject(parent)
{
    initialize();
}

void OperationsStackProxy::initialize()
{
    // use DBus if `OperationsStackManager` service exists,
    // otherwise use local cache
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    if (!interface || !interface->isServiceRegistered(kOperationsStackService).value())
        return;

    qInfo() << "Start initilize dbus: `OperationsStackManagerInterface`";
    operationsStackDbus.reset(new OperationsStackManagerInterface(kOperationsStackService,
                                                                  kOperationsStackPath,
                                                                  QDBusConnection::sessionBus(),
                                                                  this));
    if (operationsStackDbus && operationsStackDbus->isValid()) {
        dbusValid = true;
        operationsStackDbus->setTimeout(3000);
    }
    qInfo() << "Finish initilize dbus: `OperationsStackManagerInterface`";
}

DPFILEOPERATIONS_END_NAMESPACE
