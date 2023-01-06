// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEHANDLER_P_H
#define DFILEHANDLER_P_H

#include "dfilehandler.h"

DFM_BEGIN_NAMESPACE

class DFileHandlerPrivate
{
public:
    explicit DFileHandlerPrivate(DFileHandler *qq);

    void setErrorString(const QString &es);

    DFileHandler *q_ptr;
    QString errorString;
};

DFM_END_NAMESPACE

#endif // DFILEHANDLER_P_H
