// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrindexclient.h"
#include "ocrindex_interface.h"

#include <QDBusAbstractInterface>
#include <QDBusConnection>

DPSEARCH_USE_NAMESPACE

namespace {

void registerMetaTypes()
{
    static bool registered = false;
    if (!registered) {
        int id = qRegisterMetaType<DPSEARCH_NAMESPACE::OcrIndexClient::TaskType>("DPSEARCH_NAMESPACE::OcrIndexClient::TaskType");
        fmDebug() << "OcrIndex meta type registered with id:" << id;
        registered = true;
    }
}

IndexClientDescriptor buildDescriptor()
{
    return IndexClientDescriptor {
        QStringLiteral("OcrIndexClient"),
        QStringLiteral("org.deepin.Filemanager.OcrIndex"),
        QStringLiteral("/org/deepin/Filemanager/OcrIndex"),
        [](QObject *parent) -> QDBusAbstractInterface * {
            return new OrgDeepinFilemanagerOcrIndexInterface(
                    QStringLiteral("org.deepin.Filemanager.OcrIndex"),
                    QStringLiteral("/org/deepin/Filemanager/OcrIndex"),
                    QDBusConnection::sessionBus(),
                    parent);
        }
    };
}

}   // namespace

OcrIndexClient *OcrIndexClient::instance()
{
    static OcrIndexClient instance;
    return &instance;
}

OcrIndexClient::OcrIndexClient(QObject *parent)
    : AbstractIndexClient(buildDescriptor(), parent)
{
    registerMetaTypes();
}

OcrIndexClient::~OcrIndexClient() = default;
