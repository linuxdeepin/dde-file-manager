// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCRINDEXCONTROLLER_H
#define OCRINDEXCONTROLLER_H

#include "abstractindexcontroller.h"

DAEMONPCORE_BEGIN_NAMESPACE

class OcrIndexController : public AbstractIndexController
{
    Q_OBJECT

public:
    explicit OcrIndexController(QObject *parent = nullptr);
    ~OcrIndexController() override;
};

DAEMONPCORE_END_NAMESPACE

#endif   // OCRINDEXCONTROLLER_H
