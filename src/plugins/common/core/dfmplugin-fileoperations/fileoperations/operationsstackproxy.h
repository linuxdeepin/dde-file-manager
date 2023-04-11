// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPERATIONSSTACK_H
#define OPERATIONSSTACK_H

#include "dfmplugin_fileoperations_global.h"
#include "operationsstackmanager_interface.h"

#include <QObject>

using OperationsStackManagerInterface = OrgDeepinFilemanagerServerOperationsStackManagerInterface;

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

private:
    explicit OperationsStackProxy(QObject *parent = nullptr);
    void initialize();

private:
    bool dbusValid { false };
    std::unique_ptr<OperationsStackManagerInterface> operationsStackDbus;
    QStack<QVariantMap> fileOperations;
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // OPERATIONSSTACK_H
