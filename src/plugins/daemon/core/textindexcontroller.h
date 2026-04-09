// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXCONTROLLER_H
#define TEXTINDEXCONTROLLER_H

#include "abstractindexcontroller.h"

DAEMONPCORE_BEGIN_NAMESPACE

class TextIndexController : public AbstractIndexController
{
    Q_OBJECT

public:
    explicit TextIndexController(QObject *parent = nullptr);
    ~TextIndexController() override;
};

DAEMONPCORE_END_NAMESPACE

#endif   // TEXTINDEXCONTROLLER_H
