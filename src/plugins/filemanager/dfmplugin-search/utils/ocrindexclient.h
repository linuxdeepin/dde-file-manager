// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCRINDEXCLIENT_H
#define OCRINDEXCLIENT_H

#include "abstractindexclient.h"

DPSEARCH_BEGIN_NAMESPACE

class OcrIndexClient : public AbstractIndexClient
{
    Q_OBJECT

public:
    using TaskType = AbstractIndexClient::TaskType;
    using ServiceStatus = AbstractIndexClient::ServiceStatus;

    static OcrIndexClient *instance();

private:
    explicit OcrIndexClient(QObject *parent = nullptr);
    ~OcrIndexClient() override;
};

DPSEARCH_END_NAMESPACE

#endif   // OCRINDEXCLIENT_H
