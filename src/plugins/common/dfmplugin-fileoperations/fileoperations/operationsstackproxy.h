// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPERATIONSSTACK_H
#define OPERATIONSSTACK_H

#include "dfmplugin_fileoperations_global.h"

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include "operationsstackmanager_interface.h"
#else
#    include "operationsstackmanager_interface_qt6.h"
#endif

#include <QObject>

using OperationsStackManagerInterface = OrgDeepinFilemanagerDaemonOperationsStackManagerInterface;

DPFILEOPERATIONS_BEGIN_NAMESPACE

class OperationsStackProxy : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationsStackProxy)

public:
    static OperationsStackProxy &instance();

    void saveOperations(const QVariantMap &values);
    void cleanOperations();
    QVariantMap revocationOperations();
    void SaveRedoOperations(const QVariantMap &values);
    QVariantMap RevocationRedoOperations();
    void CleanOperationsByUrl(const QStringList &urls);

private:
    explicit OperationsStackProxy(QObject *parent = nullptr);
    void initialize();

private:
    bool dbusValid { false };
    std::unique_ptr<OperationsStackManagerInterface> operationsStackDbus;
    QStack<QVariantMap> fileOperations;
    QStack<QVariantMap> redoFileOperations;
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // OPERATIONSSTACK_H
