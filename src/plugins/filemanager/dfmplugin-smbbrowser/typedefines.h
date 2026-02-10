// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPEDEFINES_H
#define TYPEDEFINES_H

#include "dfmplugin_smbbrowser_global.h"

#include <QObject>
#include <QDebug>

DPSMBBROWSER_BEGIN_NAMESPACE
Q_NAMESPACE

struct SmbShareNode
{
    QString url;
    QString displayName;
    QString iconType;
};
typedef QList<SmbShareNode> SmbShareNodes;
inline QDebug operator<<(QDebug dbg, const SmbShareNode &node)
{
    dbg.nospace() << "SmbShareNode: { url: " << node.url
                  << ", name: " << node.displayName
                  << ", icon: " << node.iconType << "}";
    return dbg;
}

enum SmbDisplayMode {
    kSeperate,
    kAggregation
};
Q_ENUM_NS(SmbDisplayMode)

DPSMBBROWSER_END_NAMESPACE

using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
Q_DECLARE_METATYPE(ContextMenuCallback)
using ItemClickedActionCallback = std::function<void(quint64 windowId, const QUrl &url)>;
Q_DECLARE_METATYPE(ItemClickedActionCallback);
using FindMeCallback = std::function<bool(const QUrl &itemUrl, const QUrl &targetUrl)>;
Q_DECLARE_METATYPE(FindMeCallback);
using RenameCallback = std::function<void(quint64 windowId, const QUrl &url, const QString &name)>;
Q_DECLARE_METATYPE(RenameCallback);

#endif   // TYPEDEFINES_H
