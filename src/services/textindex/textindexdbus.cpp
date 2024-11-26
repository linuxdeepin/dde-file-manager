// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexdbus.h"
#include "private/textindexdbus_p.h"

static constexpr char kTextIndexObjPath[] { "/org/deepin/Filemanager/TextIndex" };

namespace service_textindex {
DFM_LOG_REISGER_CATEGORY(SERVICETEXTINDEX_NAMESPACE)
}

SERVICETEXTINDEX_USE_NAMESPACE

TextIndexDBus::TextIndexDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext(), d(new TextIndexDBusPrivate(this))
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SessionBus, QString(name)).registerObject(kTextIndexObjPath, this, opts);
}

TextIndexDBus::~TextIndexDBus() { }

void TextIndexDBus::CloseTask()
{
}

bool TextIndexDBus::CreateIndexTask(const QString &path)
{
    return true;
}

bool TextIndexDBus::UpdateIndexTask(const QString &path)
{
    return true;
}

bool TextIndexDBus::IndexDatabaseExists()
{
    // TODO: dconfig + lock + db exists
    return true;
}

bool TextIndexDBus::IndexTaskInProgress()
{
    return false;
}
