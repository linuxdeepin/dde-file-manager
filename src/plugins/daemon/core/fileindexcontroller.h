// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINDEXCONTROLLER_H
#define FILEINDEXCONTROLLER_H

#include "abstractindexcontroller.h"

DAEMONPCORE_BEGIN_NAMESPACE

class FileIndexController : public AbstractIndexController
{
    Q_OBJECT

public:
    explicit FileIndexController(QObject *parent = nullptr);
    ~FileIndexController() override;
};

DAEMONPCORE_END_NAMESPACE

#endif   // FILEINDEXCONTROLLER_H
