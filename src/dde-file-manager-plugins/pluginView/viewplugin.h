// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWPLUGIN_H
#define VIEWPLUGIN_H

#include  "../plugininterfaces/view/viewinterface.h"


class ViewPlugin : /*public QObject, */public ViewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ViewInterface_iid FILE "pluginView.json")
    Q_INTERFACES(ViewInterface)
public:
    explicit ViewPlugin(QObject *parent = nullptr);

    QString bookMarkText() override;
    QIcon bookMarkNormalIcon() override;
    QIcon bookMarkHoverIcon() override;
    QIcon bookMarkPressedIcon() override;
    QIcon bookMarkCheckedIcon() override;
    QString crumbText() override;
    QIcon crumbNormalIcon() override;
    QIcon crumbHoverIcon() override;
    QIcon crumbPressedIcon() override;
    QIcon crumbCheckedIcon() override;
    bool isAddSeparator() override;
    QString scheme() override;
    QWidget* createView() override;
};

#endif // VIEWPLUGIN_H
