// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMGENERICPLUGIN_H
#define DFMGENERICPLUGIN_H

#include <QObject>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

#define DFMGenericFactoryInterface_iid "com.deepin.filemanager.DFMGenericFactoryInterface_iid"

class DFMGenericPlugin : public QObject
{
    Q_OBJECT
public:
    explicit DFMGenericPlugin(QObject *parent = 0);

    virtual QObject *create(const QString &key) = 0;
};

DFM_END_NAMESPACE

#endif // DFMGENERICPLUGIN_H
