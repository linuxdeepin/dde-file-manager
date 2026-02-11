// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H
#include "dfmplugin_vault_global.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QObject>
namespace dfmplugin_vault {
class ServiceManager : public QObject
{
    Q_OBJECT
    using BasicExpand = QMultiMap<QString, QPair<QString, QString>>;
    using ExpandFieldMap = QMap<QString, BasicExpand>;

public:
    explicit ServiceManager(QObject *parent = nullptr);

public:
    static ExpandFieldMap basicViewFieldFunc(const QUrl &url);
    static ExpandFieldMap detailViewFieldFunc(const QUrl &url);
};
}
#endif   // SERVICEMANAGER_H
