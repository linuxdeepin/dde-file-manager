// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_TEXTINDEX_GLOBAL_H
#define SERVICE_TEXTINDEX_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>
#include <QStandardPaths>

#define SERVICETEXTINDEX_NAMESPACE service_textindex
#define SERVICETEXTINDEX_BEGIN_NAMESPACE namespace SERVICETEXTINDEX_NAMESPACE {
#define SERVICETEXTINDEX_END_NAMESPACE }
#define SERVICETEXTINDEX_USE_NAMESPACE using namespace SERVICETEXTINDEX_NAMESPACE;

SERVICETEXTINDEX_BEGIN_NAMESPACE

DFM_LOG_USE_CATEGORY(SERVICETEXTINDEX_NAMESPACE)

inline QString indexStorePath()
{
    static const QString kPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
            + "/deepin/dde-file-manager/index";
    return kPath;
}

SERVICETEXTINDEX_END_NAMESPACE

#endif   // SERVICE_TEXTINDEX_GLOBAL_H 