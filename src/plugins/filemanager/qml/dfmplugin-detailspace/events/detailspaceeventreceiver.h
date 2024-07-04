// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILSPACEEVENTRECEIVER_H
#define DETAILSPACEEVENTRECEIVER_H

#include "dfmplugin_detailspace_global.h"

#include <QObject>
#include <QItemSelection>

namespace dfmplugin_detailspace {

class DetailSpaceEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DetailSpaceEventReceiver)

public:
    static DetailSpaceEventReceiver &instance();

    void connectService();

public slots:
    void handleTileBarShowDetailView(quint64 windowId, bool checked);
    void handleSetSelect(quint64 windowId, const QUrl &url);
    bool handleViewExtensionRegister(CustomViewExtensionView view, int index);
    bool handleBasicViewExtensionRegister(BasicViewFieldFunc func, const QString &scheme);
    bool handleBasicViewExtensionRootRegister(BasicViewFieldFunc func, const QString &scheme);
    bool handleBasicFiledFilterAdd(const QString &scheme, const QStringList &enums);
    bool handleBasicFiledFilterRootAdd(const QString &scheme, const QStringList &enums);

    // worksapce
    void handleViewSelectionChanged(const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected);

private:
    explicit DetailSpaceEventReceiver(QObject *parent = nullptr);
};

}

#endif   // DETAILSPACEEVENTRECEIVER_H
