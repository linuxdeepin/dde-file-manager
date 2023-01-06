// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DADVANCEDINFOPLUGIN_H
#define DADVANCEDINFOPLUGIN_H

#include <../dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h>
#include <QObject>

class QWidget;

class DAdvancedInfoPlugin : public PropertyDialogExpandInfoInterface
{
    Q_OBJECT
    Q_INTERFACES(PropertyDialogExpandInfoInterface)
    Q_PLUGIN_METADATA(IID PropertyDialogExpandInfoInterface_iid FILE "dde-advanced-property-plugin.json")

public:
    explicit DAdvancedInfoPlugin(QObject *parent = nullptr);

    QWidget* expandWidget(const QString &file) override;

    QString expandWidgetTitle(const QString &file) override;
};

#endif // DADVANCEDINFOPLUGIN_H
