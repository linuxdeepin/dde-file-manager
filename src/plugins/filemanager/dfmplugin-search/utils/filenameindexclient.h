// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMEINDEXCLIENT_H
#define FILENAMEINDEXCLIENT_H

#include "abstractindexclient.h"

DPSEARCH_BEGIN_NAMESPACE

class FileNameIndexClient : public AbstractIndexClient
{
    Q_OBJECT

public:
    using TaskType = AbstractIndexClient::TaskType;
    using ServiceStatus = AbstractIndexClient::ServiceStatus;

    static FileNameIndexClient *instance();

private:
    explicit FileNameIndexClient(QObject *parent = nullptr);
    ~FileNameIndexClient() override;
};

DPSEARCH_END_NAMESPACE

#endif   // FILENAMEINDEXCLIENT_H
