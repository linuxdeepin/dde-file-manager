// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexcontroller.h"

#include "textindex_interface.h"

#include <dfm-search/dsearch_global.h>

DAEMONPCORE_BEGIN_NAMESPACE

namespace {

static constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
static constexpr char kEnableFullTextSearch[] { "enableFullTextSearch" };

IndexControllerDescriptor buildDescriptor()
{
    return IndexControllerDescriptor {
        QStringLiteral("TextIndexController"),
        QStringLiteral("org.deepin.Filemanager.TextIndex"),
        QStringLiteral("/org/deepin/Filemanager/TextIndex"),
        QString::fromLatin1(kSearchCfgPath),
        QString::fromLatin1(kEnableFullTextSearch),
        []() { return DFMSEARCH::Global::defaultIndexedDirectory(); },
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

TextIndexController::TextIndexController(QObject *parent)
    : AbstractIndexController(buildDescriptor(), parent)
{
}

TextIndexController::~TextIndexController() = default;

DAEMONPCORE_END_NAMESPACE
