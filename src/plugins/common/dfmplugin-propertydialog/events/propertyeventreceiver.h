// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROPERTYDIALOGUNICASTRECEIVER_H
#define PROPERTYDIALOGUNICASTRECEIVER_H

#include "dfmplugin_propertydialog_global.h"

#include <QObject>

namespace dfmplugin_propertydialog {
class PropertyEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertyEventReceiver)
private:
    explicit PropertyEventReceiver(QObject *parent = nullptr);

public:
    static PropertyEventReceiver *instance();
    void bindEvents();

    void handleShowPropertyDialog(const QList<QUrl> &urls, const QVariantHash &option);
    bool handleViewExtensionRegister(CustomViewExtensionView view, const QString &name, int index);
    bool handleCustomViewRegister(CustomViewExtensionView view, const QString &scheme);
    bool handleBasicViewExtensionRegister(BasicViewFieldFunc func, const QString &scheme);
    bool handleBasicFiledFilterAdd(const QString &scheme, const QStringList &enums);
};
}
#endif   // PROPERTYDIALOGEVENTRECEIVER_H
