// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrindexcontroller.h"

#include "ocrindex_interface.h"

#include <dfm-search/dsearch_global.h>

DAEMONPCORE_BEGIN_NAMESPACE

namespace {

static constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
static constexpr char kEnableOcrTextSearch[] { "enableOcrTextSearch" };

IndexControllerDescriptor buildDescriptor()
{
    return IndexControllerDescriptor {
        QStringLiteral("OcrIndexController"),
        QStringLiteral("org.deepin.Filemanager.OcrIndex"),
        QStringLiteral("/org/deepin/Filemanager/OcrIndex"),
        QString::fromLatin1(kSearchCfgPath),
        QString::fromLatin1(kEnableOcrTextSearch),
        []() { return DFMSEARCH::Global::defaultIndexedDirectory(); },
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

OcrIndexController::OcrIndexController(QObject *parent)
    : AbstractIndexController(buildDescriptor(), parent)
{
}

OcrIndexController::~OcrIndexController() = default;

DAEMONPCORE_END_NAMESPACE
