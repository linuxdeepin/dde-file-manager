// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMCRUMBPLUGIN_H
#define DFMCRUMBPLUGIN_H

#include <QObject>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE
#define DFMCrumbFactoryInterface_iid "com.deepin.filemanager.DFMCrumbFactoryInterface_iid"

class DFMCrumbInterface;
class DFMCrumbPlugin : public QObject
{
    Q_OBJECT
public:
    explicit DFMCrumbPlugin(QObject *parent = 0);
    ~DFMCrumbPlugin();

    virtual DFMCrumbInterface *create(const QString &key) = 0;
};
DFM_END_NAMESPACE

#endif // DFMCRUMBPLUGIN_H
