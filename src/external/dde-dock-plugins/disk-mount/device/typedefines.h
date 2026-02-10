// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPEDEFINES_H
#define TYPEDEFINES_H

#include <QObject>
#include <QString>
#include <QUrl>

inline int kDockPluginWidth = 300;
inline int kDeviceItemHeight = 70;

struct DockItemData
{
    QString id;
    QString backingID;
    QString displayName;
    QUrl targetUrl;
    QUrl targetFileUrl;
    QString iconName;
    quint64 totalSize;
    quint64 usedSize;
    QString sortKey;
    bool isProtocolDevice;
};
Q_DECLARE_METATYPE(DockItemData)

#endif   // TYPEDEFINES_H
