// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexclient.h"
#include "textindex_interface.h"

DPSEARCH_USE_NAMESPACE

namespace {

void registerMetaTypes()
{
    static bool registered = false;
    if (!registered) {
        int id = qRegisterMetaType<DPSEARCH_NAMESPACE::TextIndexClient::TaskType>("DPSEARCH_NAMESPACE::TextIndexClient::TaskType");
        fmDebug() << "TextIndex meta type registered with id:" << id;
        registered = true;
    }
}

IndexClientDescriptor buildDescriptor()
{
    return IndexClientDescriptor {
        QStringLiteral("TextIndexClient"),
        QStringLiteral("org.deepin.Filemanager.TextIndex"),
        QStringLiteral("/org/deepin/Filemanager/TextIndex"),
        [](QObject *parent) -> QDBusAbstractInterface * {
            return new OrgDeepinFilemanagerTextIndexInterface(
                    QStringLiteral("org.deepin.Filemanager.TextIndex"),
                    QStringLiteral("/org/deepin/Filemanager/TextIndex"),
                    QDBusConnection::sessionBus(),
                    parent);
        }
    };
}
}   // namespace

TextIndexClient *TextIndexClient::instance()
{
    static TextIndexClient instance;
    return &instance;
}

TextIndexClient::TextIndexClient(QObject *parent)
    : AbstractIndexClient(buildDescriptor(), parent)
{
    registerMetaTypes();
}

TextIndexClient::~TextIndexClient() = default;
