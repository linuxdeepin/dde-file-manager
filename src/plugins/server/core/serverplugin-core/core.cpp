// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <QDBusConnection>

SERVERPCORE_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

void Core::initialize()
{
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
    UrlRoute::regScheme(Global::Scheme::kEntry, "/", QIcon(), true);   // Why??
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
}

bool Core::start()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                 "Please check your system settings and try again.\n");
        return false;
    }
    return true;
}

SERVERPCORE_END_NAMESPACE
