// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPERATIONSSTACKMANAGERDBUS_H
#define OPERATIONSSTACKMANAGERDBUS_H

#include <QDBusVariant>
#include <QVariantMap>
#include <QStack>
#include <QObject>

class OperationsStackManagerDbus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.filemanager.server.OperationsStackManager")

public:
    explicit OperationsStackManagerDbus(QObject *parent = nullptr);

public slots:
    void SaveOperations(const QVariantMap &values);
    void CleanOperations();
    QVariantMap RevocationOperations();

private:
    QStack<QVariantMap> fileOperations;
};

#endif   // OPERATIONSSTACKMANAGERDBUS_H
