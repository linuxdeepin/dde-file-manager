// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileindexcontroller.h"

#include "filenameindex_interface.h"

#include <dfm-search/dsearch_global.h>

DAEMONPCORE_BEGIN_NAMESPACE

namespace {

static constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
static constexpr char kEnableFileIndexSearch[] { "enableFileIndexSearch" };

IndexControllerDescriptor buildDescriptor()
{
    return IndexControllerDescriptor {
        QStringLiteral("FileIndexController"),
        QStringLiteral("org.deepin.Filemanager.TextIndex"),
        QStringLiteral("/org/deepin/Filemanager/FileNameIndex"),
        QString::fromLatin1(kSearchCfgPath),
        QString::fromLatin1(kEnableFileIndexSearch),
        []() { return DFMSEARCH::Global::defaultIndexedDirectory(); },
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

FileIndexController::FileIndexController(QObject *parent)
    : AbstractIndexController(buildDescriptor(), parent)
{
}

FileIndexController::~FileIndexController() = default;

DAEMONPCORE_END_NAMESPACE
