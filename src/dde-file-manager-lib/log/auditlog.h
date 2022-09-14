// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDITLOG_H
#define AUDITLOG_H

#include "durl.h"

class AuditLog
{
public:
    static void doCopyFromDiscAuditLog(const DUrlList &srcList, const DUrlList &destList);

};

#endif // AUDITLOG_H
