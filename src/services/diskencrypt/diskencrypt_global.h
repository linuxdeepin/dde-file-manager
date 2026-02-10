// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMONPLUGIN_FILE_ENCRYPT_GLOBAL_H
#define DAEMONPLUGIN_FILE_ENCRYPT_GLOBAL_H

#include "globaltypesdefine.h"

#include <QtCore/qglobal.h>
#include <QString>
#include <QDebug>

#if defined(DAEMONPLUGIN_FILE_ENCRYPT_LIBRARY)
#    define DAEMONPLUGIN_FILE_ENCRYPT_EXPORT Q_DECL_EXPORT
#else
#    define DAEMONPLUGIN_FILE_ENCRYPT_EXPORT Q_DECL_IMPORT
#endif

#define FILE_ENCRYPT_NS daemonplugin_file_encrypt
#define FILE_ENCRYPT_BEGIN_NS namespace FILE_ENCRYPT_NS {
#define FILE_ENCRYPT_END_NS }
#define FILE_ENCRYPT_USE_NS using namespace FILE_ENCRYPT_NS;

#endif   // DAEMONPLUGIN_FILE_ENCRYPT_GLOBAL_H
