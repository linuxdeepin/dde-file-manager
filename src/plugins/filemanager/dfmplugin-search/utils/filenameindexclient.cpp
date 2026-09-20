// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenameindexclient.h"
#include "filenameindex_interface.h"

#include <QDBusAbstractInterface>
#include <QDBusConnection>

DPSEARCH_USE_NAMESPACE

namespace {

void registerMetaTypes()
{
    static bool registered = false;
    if (!registered) {
        int id = qRegisterMetaType<DPSEARCH_NAMESPACE::FileNameIndexClient::TaskType>("DPSEARCH_NAMESPACE::FileNameIndexClient::TaskType");
        fmDebug() << "FileNameIndex meta type registered with id:" << id;
        registered = true;
    }
}

IndexClientDescriptor buildDescriptor()
{
    return IndexClientDescriptor {
        QStringLiteral("FileNameIndexClient"),
        QStringLiteral("org.deepin.Filemanager.FileNameIndex"),
        QStringLiteral("/org/deepin/Filemanager/FileNameIndex"),
        [](QObject *parent) -> QDBusAbstractInterface * {
            return new OrgDeepinFilemanagerFileNameIndexInterface(
                    QStringLiteral("org.deepin.Filemanager.FileNameIndex"),
                    QStringLiteral("/org/deepin/Filemanager/FileNameIndex"),
                    QDBusConnection::sessionBus(),
                    parent);
        }
    };
}

}   // namespace

FileNameIndexClient *FileNameIndexClient::instance()
{
    static FileNameIndexClient instance;
    return &instance;
}

FileNameIndexClient::FileNameIndexClient(QObject *parent)
    : AbstractIndexClient(buildDescriptor(), parent)
{
    registerMetaTypes();
}

FileNameIndexClient::~FileNameIndexClient() = default;
