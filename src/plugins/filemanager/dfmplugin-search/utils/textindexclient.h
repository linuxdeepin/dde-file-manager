// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXCLIENT_H
#define TEXTINDEXCLIENT_H

#include "abstractindexclient.h"

DPSEARCH_BEGIN_NAMESPACE

class TextIndexClient : public AbstractIndexClient
{
    Q_OBJECT
public:
    using TaskType = AbstractIndexClient::TaskType;
    using ServiceStatus = AbstractIndexClient::ServiceStatus;

    static TextIndexClient *instance();

private:
    explicit TextIndexClient(QObject *parent = nullptr);
    ~TextIndexClient() override;
};

DPSEARCH_END_NAMESPACE

#endif   // TEXTINDEXCLIENT_H
