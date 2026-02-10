// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.OperationsStackManager")

public:
    explicit OperationsStackManagerDbus(QObject *parent = nullptr);

public slots:
    void SaveOperations(const QVariantMap &values);
    void CleanOperations();
    QVariantMap RevocationOperations();
    void SaveRedoOperations(const QVariantMap &values);
    QVariantMap RevocationRedoOperations();
    void CleanOperationsByUrl(const QStringList &urls);

private:
    QStack<QVariantMap> fileOperations;
    QStack<QVariantMap> redoFileOperations;
};

#endif   // OPERATIONSSTACKMANAGERDBUS_H
