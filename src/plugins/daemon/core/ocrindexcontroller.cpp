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
        // 使用 TextIndex 的 DBus 服务名，因为 TextIndex 和 OcrIndex 共用同一个插件进程
        // plugin.cpp 中会同时注册 org.deepin.Filemanager.TextIndex 和 OcrIndex 两个服务
        QStringLiteral("org.deepin.Filemanager.TextIndex"),
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
