// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINQUICKMETADATA_P_H
#define PLUGINQUICKMETADATA_P_H

#include <dfm-framework/lifecycle/pluginquickmetadata.h>

#include <QUrl>
#include <QString>

DPF_BEGIN_NAMESPACE

class PluginQuickData
{
public:
    QUrl quickUrl;
    QString quickId;
    QString plugin;
    QString quickType;
    QString quickParent;
    QString quickApplet;
};

DPF_END_NAMESPACE

#endif   // PLUGINQUICKMETADATA_P_H
